# Networking usage guide

This guide describes the module in `network/`, shared by the client and server. It transports packets over **UDP IPv4**. It does not depend on the game engine or graphics library.

## 1. Obtaining and building the module

In this repository, use `network/include/` and `network/src/` directly. To integrate the module into another project, copy both directories together, preserving their layout.

Prerequisites: a C++20 compiler, standalone Asio headers (`<asio.hpp>`, not Boost.Asio) and thread support. The module uses `std::bit_cast`, which requires C++20.

Add the include path and these **four source files** to your target:

```makefile
CPPFLAGS += -I../network/include
CXXFLAGS += -std=c++20 -pthread
LDFLAGS += -pthread

SRC += ../network/src/NetworkHandler.cpp \
       ../network/src/PacketCodec.cpp \
       ../network/src/Paquet.cpp \
       ../network/src/WakeSignal.cpp
```

These paths assume a Makefile located in `client/` or `server/`. Add the sources before computing `OBJ`, and use `$(LDFLAGS)` when linking. Keep your application's other sources and libraries.

The server Makefile already references these sources. The client Makefile has the include path, but still needs the four source files to use the networking module.

From the repository root, a standalone `example.cpp` program using the module can be built as follows:

```sh
g++ -std=c++20 -Wall -Wextra -Werror -pthread \
    -Inetwork/include example.cpp \
    network/src/NetworkHandler.cpp network/src/PacketCodec.cpp \
    network/src/Paquet.cpp network/src/WakeSignal.cpp \
    -o example
```

## 2. Structure and API

| Component | Purpose |
| --- | --- |
| `NetworkHandler.hpp` | Socket, Asio event loop, outgoing and incoming queues |
| `Paquet.hpp` | Concrete packet types and protocol constants |
| `IPacket.hpp` | Polymorphic interface with `serialize()` |
| `PacketCodec.hpp` | Binary encoding and decoding |
| `WakeSignal.hpp` | Optional wake-up mechanism for a consumer loop |

Include `NetworkHandler.hpp` to use the module; it includes the required packet types.

```cpp
Net::NetworkHandler network(portLocal, wakeSignal);
```

The constructor immediately opens and binds the socket on all IPv4 interfaces. The port is the **local port**, not the remote server's port. Use a fixed port on the server and `0` on the client to let the system choose an available port. A socket opening error or an already occupied port may cause the constructor to throw an exception.

| Method | Usage |
| --- | --- |
| `run()` | Starts receiving, then runs the Asio event loop; a blocking call in a dedicated thread |
| `send(packet, destination)` | Transfers a packet into the outgoing queue; the Asio event loop performs the actual send |
| `popPacket()` | Removes a received packet, or immediately returns `std::nullopt` if the queue is empty |
| `stop()` | Stops the Asio event loop; does not join the thread or notify `WakeSignal` |
| `receive()` | Starts a receive operation; already called by `run()`, so do not call it separately |

Use one thread running `run()` per handler. `send()` and `popPacket()` protect their queues with mutexes and can be called from the application thread. The handler must remain alive until the networking thread has finished.

A received item provides:

```cpp
Net::IncomingPacket<Net::PacketCodec::Packet>
// .sender : asio::ip::udp::endpoint (source IP and port)
// .packet : std::shared_ptr<IPacket> (Net::PacketCodec::Packet)
```

## 3. Client integration

Place the following snippets at the appropriate stages of the client lifecycle: initialization, rendering loop, then shutdown.

### Start the networking thread and send HELLO

```cpp
#include "NetworkHandler.hpp"
#include <atomic>
#include <thread>

Net::NetworkHandler network(0, nullptr);
asio::ip::udp::endpoint server(
    asio::ip::make_address("127.0.0.1"), 4242
);

std::atomic<bool> networkFailed{false};
std::thread networkThread([&] {
    try {
        network.run();
    } catch (const std::exception& error) {
        std::cerr << "Network error: " << error.what() << '\n';
        networkFailed.store(true);
        network.stop();
    }
});

network.send(std::make_unique<Net::HelloPacket>(), server);
```

Replace the address and port with those of the server. `127.0.0.1` refers to the local machine. `make_address` expects a numeric address; for a DNS name, use an `asio::ip::udp::resolver` and select an IPv4 endpoint.

UDP does not establish a connection: HELLO/WELCOME is an application-level exchange. The handler does not retry automatically if a packet is lost.

### Process packets every frame

Declare this state before the rendering loop:

```cpp
bool welcomed = false;
std::uint32_t sessionId = 0;
```

Every frame, drain the queue without blocking rendering:

```cpp
while (auto incoming = network.popPacket()) {
    if (incoming->sender != server)
        continue;

    if (auto* welcome = dynamic_cast<Net::WelcomePacket*>(incoming->packet.get())) {
        sessionId = welcome->clientId;
        welcomed = true;
    } else if (auto* snapshot = dynamic_cast<Net::SnapshotPacket*>(incoming->packet.get())) {
        for (const auto& entity : snapshot->entities) {
            // Update the graphical object identified by entity.id:
            // entity.x, entity.y, entity.kind, entity.health, entity.flags.
        }
    }
}
```

The pointers obtained through `dynamic_cast` refer to objects owned by `incoming->packet`. Do not keep them after that packet is destroyed: copy the data needed for rendering. The endpoint filter compares IP address and port; it is not authentication.

### Send player input

After receiving WELCOME, create a new packet for each send:

```cpp
if (welcomed) {
    auto input = std::make_unique<Net::InputPacket>();
    input->header.sessionId = sessionId;
    input->buttons = Net::InputButton::UP | Net::InputButton::RIGHT;
    network.send(std::move(input), server);
}
```

`buttons = 0` means no direction is active. To check a direction when receiving a packet:

```cpp
bool up = (inputPacket.buttons & Net::InputButton::UP) != 0;
```

`|` combines buttons; `&` checks whether they are set. The protocol accepts `UP`, `DOWN`, `LEFT`, `RIGHT` and `FIRE`.

The server only applies inputs from clients that are in an instance. After
WELCOME, send `CreateInstancePacket` to create a game, or an `InstancePacket`
with an `instanceId` to join one; the server answers with an `InstancePacket`
(`instanceId == Net::NO_INSTANCE` on failure). See [Server](server.md).

### Shut down the client

When the window closes, or if `networkFailed` is true:

```cpp
network.stop();
if (networkThread.joinable())
    networkThread.join();
```

Also perform this cleanup if application code throws an exception after the thread starts. Destroying a `std::thread` that is still joinable terminates the program. The handler's destructor does not perform this cleanup for you.

## 4. Server integration with WakeSignal

A server can sleep while waiting for work. The example below is a minimal networking loop: it assigns a session identifier to each endpoint and responds to HELLO packets. Game integration and the shutdown trigger still need to be connected to the application.

```cpp
#include "NetworkHandler.hpp"
#include <atomic>
#include <thread>
#include <unordered_map>

void runNetworkServer(std::atomic<bool>& running,
                      const std::shared_ptr<WakeSignal>& wake) {
    Net::NetworkHandler network(4242, wake);
    std::unordered_map<asio::ip::udp::endpoint, std::uint32_t> sessions;
    std::uint32_t nextId = 1;

    std::thread networkThread([&] {
        try {
            network.run();
        } catch (const std::exception& error) {
            std::cerr << "Network error: " << error.what() << '\n';
        }
        running.store(false);
        wake->notify();
    });

    try {
        while (running.load()) {
            wake->wait();
            if (!running.load())
                break;

            while (auto incoming = network.popPacket()) {
                if (dynamic_cast<Net::HelloPacket*>(incoming->packet.get())) {
                    auto [it, inserted] = sessions.try_emplace(incoming->sender, nextId);
                    if (inserted)
                        ++nextId;
                    auto welcome = std::make_unique<Net::WelcomePacket>();
                    welcome->header.sessionId = it->second;
                    welcome->clientId = it->second;
                    network.send(std::move(welcome), incoming->sender);
                } else if (auto* input = dynamic_cast<Net::InputPacket*>(incoming->packet.get())) {
                    auto it = sessions.find(incoming->sender);
                    if (it == sessions.end() || input->header.sessionId != it->second)
                        continue;
                    const bool up = (input->buttons & Net::InputButton::UP) != 0;
                    (void)up; // Forward the controls to the game input queue.
                }
            }
            // Drain the other queues associated with the same WakeSignal here,
            // such as snapshots produced by the game thread.
        }
    } catch (...) {
        network.stop();
        networkThread.join();
        throw;
    }
    network.stop();
    networkThread.join();
}
```

The caller creates `std::atomic<bool> running{true}` and `auto wake = std::make_shared<WakeSignal>()`. Another thread can request shutdown with `running.store(false)` followed by `wake->notify()`. Do not call `notify()` directly from a POSIX signal handler: it uses a mutex.

### Broadcast a snapshot

Build the data, then allocate a packet for each recipient:

```cpp
Net::SnapshotPacket snapshot;
snapshot.serverTick = 42;
snapshot.entities.push_back(Net::EntityState{
    1, Net::EntityKind::PLAYER, 100.0f, 200.0f, 100, Net::EntityFlag::ALIVE
});

auto packet = std::make_shared<Net::SnapshotPacket>(snapshot);
for (const auto& [endpoint, sessionId] : sessions)
    network.send(packet, endpoint);
```

`Net::PacketCodec::Packet` is a `std::shared_ptr<IPacket>`, so the same packet can be sent to several recipients. Create a separate packet per recipient when a field differs, such as `header.sessionId`.

## 5. Using WakeSignal correctly

`WakeSignal` acts as a notification mechanism between work producers and a consumer. It contains neither packets nor results: data remains in the corresponding queues.

- `notify()` records a pending wake-up (`_pending = true`) and wakes one waiting thread.
- `wait()` blocks until a wake-up is pending, then consumes it (`_pending = false`).
- A notification received before `wait()` remains pending: the next call does not block.
- Multiple notifications can merge into a single wake-up. This is not a packet counter.

The handler automatically notifies after adding a **valid** packet to the incoming queue. An invalid packet is rejected without notification. `send()` does not use this signal to trigger sending: it uses `asio::post`.

After each wake-up, drain **all** relevant queues. If networking and game logic share the same `shared_ptr<WakeSignal>`, the consumer must check both the networking queue and the game output queue. Each producer must enqueue its data with appropriate synchronization, then call `notify()`.

Use one consumer per signal: `notify_one()` does not wake every thread. The signal does not replace queue mutexes or automatically make other objects thread-safe.

In a rendering loop, pass `nullptr` to the handler and call `popPacket()` every frame. Calling `wait()` on the rendering thread would freeze the display when there is no traffic. The current API does not provide a timed wait.

To stop a blocked consumer, update its shutdown flag, then call `notify()`. Calling `network.stop()` alone does not unblock `wake->wait()`.

## 6. Packets and protocol limits

The codec handles serialization: there is no need to construct bytes manually when using `send()` and `popPacket()` normally.

| Packet | Usual direction | Data |
| --- | --- | --- |
| `HelloPacket` | Client → server | Header only |
| `WelcomePacket` | Server → client | Header and `clientId` (session identifier) |
| `CreateInstancePacket` | Client → server | Header only |
| `InstancePacket` | Both directions | Header and `instanceId` |
| `InputPacket` | Client → server | Header and `buttons` bitmask |
| `SnapshotPacket` | Server → client | Header, `serverTick`, `score`, `entities` list |
| `EventPacket` | Both directions | Header, `eventType`, `eventId`, `eventData` (`IEvent`) |

Each header contains `type`, `version`, `sequence` and `sessionId`. The default constructors of concrete packets initialize the correct type and version. If you provide a `PacketHeader` yourself, explicitly initialize its `type`.

`sequence` and `sessionId` default to zero. The handler does not assign or increment them, or validate their application-level meaning. Session management, selecting recent snapshots and handling duplicates are the responsibility of the client/server.

The format uses multibyte integers in network byte order (big-endian). Positions are encoded as 32-bit floating-point values. The limit is **1,200 bytes per packet**, or **73 entities per snapshot** (18 base bytes and 16 per entity). The codec does not split larger snapshots or reassemble them.

Decoding rejects incorrect sizes, unknown versions/types, unsupported buttons, unknown entity types, unknown event subtypes and non-finite positions. In these cases, `deserialize()` returns `nullptr`; the handler prints `invalid packet` and continues receiving.

Serialization may throw `std::invalid_argument` for invalid data. It runs in the thread executing `run()`, not necessarily the one calling `send()`: wrap `run()` in a `try/catch` block.

See also the [protocol documentation](protocol.md); the definitions used by the implementation are in [Paquet.hpp](https://github.com/EpitechPGE3-2026/G-ING-500-LIL-5-1-rtype-5/blob/main/network/include/Paquet.hpp) and [PacketCodec.cpp](https://github.com/EpitechPGE3-2026/G-ING-500-LIL-5-1-rtype-5/blob/main/network/src/PacketCodec.cpp).

## 7. Current state and integration considerations

The module is a UDP transport: it provides no delivery or ordering guarantees, duplicate prevention, reconnection, session timeout, encryption or authentication. Event acknowledgements, retries and session timeouts are implemented by the server (see [Server](server.md)). `send()` does not confirm receipt by the recipient.

The current version stores outgoing bytes in a `shared_ptr` captured by the Asio callback: their memory remains valid until the operation completes. Preserve this lifetime if you modify the transport.

## 8. Checking an integration

1. Build both executables with the four networking source files.
2. Start the server on the chosen port, then point the client to that IP address and port.
3. Check that HELLO and WELCOME are received and verify the exchanged identifiers.
4. Send INPUT and check the direction bitmask on the server.
5. Send a snapshot with one entity and check its values on the client.
6. Connect a second client to the same instance and check that both receive the snapshots.
7. Shut down with no network traffic to check that waiting threads are woken and threads are joined.
