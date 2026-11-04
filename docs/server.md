# Server

The server (`server/`, target `rtype_server`) is the authority of the game. It
accepts UDP clients, groups them into independent game **instances**, runs one
simulation per instance and sends the resulting state back to the players of
each instance.

## Running the server

```sh
./build-linux/rtype_server
```

The server takes no command-line arguments. Its settings are currently fixed in
`server/src/main.cpp`:

| Setting | Value | Where |
| --- | --- | --- |
| UDP port | `4242`, bound on all IPv4 interfaces | `rtype::Server server(4242, 20)` |
| Tick rate | `20` simulation ticks per second for every instance | same line |
| Worker threads | `std::thread::hardware_concurrency()` | `Server` constructor |
| Session timeout | 10 seconds without any packet | `Server::removeInactiveClients` |
| Event retry delay | 200 ms | `Server::handlePendingEvents` |

The server runs until the process is stopped (for example with `Ctrl+C`); there
is no shutdown command or signal handler yet.

## Architecture

| File | Role |
| --- | --- |
| `server/src/main.cpp` | Creates the `rtype::Server` and calls `run()` |
| `server/include/Server.hpp`, `server/src/Server.cpp` | Sessions, packet handling, instances, event delivery |
| `server/include/Session.hpp` | `Session` (one per client endpoint) and `PendingEvent` |
| `server/include/Instance.hpp`, `server/src/Instance.cpp` | One game instance wrapping an `rtype::Game` |
| `server/include/ThreadPool.hpp` | `PoolThread`, the worker pool that runs instance updates |

The server depends on the `rtype_network` library (UDP transport and protocol
codec, see [Networking guide](network.md)) and on the `rtype_game` library
(simulation, see [Game engine](engine.md)).

### Threads

| Thread | Work |
| --- | --- |
| Main thread | Runs `Server::run()`: sessions, packets, scheduling, output dispatch, event retries |
| Network thread | Runs the Asio event loop of `Net::NetworkHandler` (receive and send) |
| Worker pool | Runs `Instance::update()` (one `Game::update()` call) for each instance |

Every structure that is shared between threads is protected:

- the incoming and outgoing packet queues by the mutexes of `NetworkHandler`;
- the instance map by `Server::_instancesMutex`;
- each game's input and output queues by the mutexes of `Game`;
- `Instance::_updating` (an `std::atomic<bool>`) ensures that an instance is
  never updated by two workers at the same time.

Sessions (`_sessions`) are only accessed by the main thread.

All producers share one `WakeSignal`: the network thread notifies it when a
valid packet arrives and each game notifies it at the end of an update. The main
loop therefore sleeps until there is work or until the next tick.

### Main loop

Each iteration of `Server::run()`:

1. **Removes inactive sessions**: a session that has not sent any packet for 10
   seconds leaves its instance and is deleted (see [Timeouts](#timeouts)).
2. **Handles every received packet** (see [Packet handling](#packet-handling)).
3. **Schedules a tick** when the tick deadline is reached: for every instance
   that is not already updating, one `update()` task is queued on the worker
   pool. If the server is late, missed ticks are not replayed.
4. **Dispatches game outputs**: the outputs produced by each instance are
   converted into network packets (see [Game outputs](#game-outputs)).
5. **Sends pending events** whose retry time is reached.
6. **Waits** on the `WakeSignal` until the next tick deadline.

## Sessions

A session is created when a `HELLO` packet arrives from an endpoint (IP address
and UDP port) that has no session yet. Sending `HELLO` again from the same
endpoint keeps the existing session.

| Field | Meaning |
| --- | --- |
| `client_id` | Session identifier, assigned incrementally from `0` |
| `instance_id` | Instance joined by the client, `0` (`NO_INSTANCE`) when none |
| `pendingEvents` | Events sent to this client and not acknowledged yet, by `eventId` |
| `lastSeen` | Time of the last packet received from this endpoint |

The server answers `HELLO` with `WELCOME`, whose `header.sessionId` and
`clientId` are both the session identifier. Afterwards, the client must put this
identifier in the `header.sessionId` of `INPUT`, `INSTANCE`, `CHAT` and `ACK`
packets; the server drops these packets when the identifier does not match the
session of the sending endpoint.

## Instances

An instance is an independent game: its own `rtype::Game`, entities, score and
tick counter. Instance identifiers are assigned incrementally from `1`; `0`
means "no instance". A session belongs to at most one instance.

### Creating an instance

A client sends `CREATE_INSTANCE`. If its session is not in an instance yet, the
server creates a new instance, adds the player to it and answers `INSTANCE` with
the new identifier. If the session is already in an instance, no instance is
created and the answer carries `0`. A `CREATE_INSTANCE` from an endpoint without
session is ignored.

### Joining an instance

A client sends `INSTANCE` with the identifier of the instance to join. The
server answers `INSTANCE` with:

- the requested identifier if the instance exists and the session is not in
  another instance (joining the instance the session is already in also
  succeeds, without adding a second player);
- `0` if the instance does not exist, if the identifier is `0`, or if the
  session already belongs to another instance.

The identifier of an instance is shared out of band (for example, the creator
communicates it to the other players).

### Lifetime

Joining an instance queues an `AddPlayer` input in its game; the player entity
is created at the next update of that instance. When a session times out, a
`RemovePlayer` input is queued. Instances are never destroyed, even when they
have no player left: they keep simulating until the server stops.

## Packet handling

| Packet received | Condition | Server action |
| --- | --- | --- |
| `HELLO` | Always | Creates the session if needed, answers `WELCOME` |
| `CREATE_INSTANCE` | Known endpoint | Creates and joins an instance, answers `INSTANCE` |
| `INSTANCE` | Known endpoint, matching `sessionId` | Joins the instance, answers `INSTANCE` |
| `INPUT` | Known endpoint, matching `sessionId`, in an instance | Forwards the controls to the instance's game |
| `EVENT` / `ACK` | Known endpoint, matching `sessionId` | Removes the acknowledged pending event |
| `EVENT` / `CHAT` | Known endpoint, matching `sessionId`, in an instance, non-empty text | Broadcasts the message to the instance |
| Anything else | | Ignored |

Any packet received from a known endpoint refreshes its `lastSeen` time, even
when the packet is otherwise ignored. Packets that the codec cannot decode are
dropped by the network layer before reaching the server.

### Inputs

`INPUT.buttons` is converted into a `Controllable` component (`UP`, `DOWN`,
`LEFT`, `RIGHT`, `FIRE`) and replaces the previous controls of the player at
the next update of the instance. Controls stay active until a new `INPUT`
changes them, so clients should send an `INPUT` whenever their keys change
(including an empty one when all keys are released).

### Chat

When a valid `CHAT` event is received, the server creates a new `CHAT` event
with its own `eventId` and `senderId` set to the sender's session identifier,
then delivers it reliably (see [Events](#events)) to every session of the
sender's instance, sender included. The `senderId` and `eventId` provided by the
client are ignored, so a client cannot impersonate another player. The message
from the client to the server is not acknowledged.

## Game outputs

After each update, a game produces outputs that the server converts as follows:

| Game output | Packet | Recipients | Delivery |
| --- | --- | --- | --- |
| `Snapshot` | `SNAPSHOT` with every entity of the instance | Sessions of the instance | Best effort, sent once |
| `PlayerCreated` | `EVENT` `NEW_CLIENT` with the player entity id | Sessions of the instance | Reliable (event) |
| `EntityDestroyed` | `EVENT` `DESTROY` with the entity id | Sessions of the instance | Reliable (event) |

Snapshots carry `serverTick`, and for each entity its id, kind, position and
health. `flags` is currently always `0` and `header.sessionId` is `0`.

## Events

Events (`NEW_CLIENT`, `DESTROY`, `DISCONNECTED`, `CHAT`) are delivered with
acknowledgement and retransmission:

1. The server assigns a server-wide `eventId` and stores a copy of the packet
   in the `pendingEvents` of every session of the instance, with that session's
   identifier in `header.sessionId`.
2. At the end of the loop iteration, every pending event is sent. It is sent
   again every 200 ms while it stays pending.
3. When the client answers with an `ACK` carrying the same `eventId` and its
   session identifier, the event is removed from its session.

There is no retry limit: an event is resent until it is acknowledged or the
session is removed. Because an event can be received several times (for example
when an `ACK` is lost), clients must handle duplicates, using `eventId` to
recognize them.

## Timeouts

A session that has not sent any packet for **10 seconds** is removed:

1. its player is removed from its instance (`RemovePlayer`), which later
   produces a `DESTROY` event for its entity;
2. the session and its pending events are deleted;
3. if the session was in an instance, a `DISCONNECTED` event is broadcast to
   the remaining players of that instance. Its payload carries the **session
   identifier** of the disconnected client, not an entity identifier.

Clients that have nothing else to send should send an empty `INPUT` regularly
(the current client does so every second) to keep their session alive.

## Known limitations

- **No security.** Sessions are identified by endpoint and a predictable
  incremental identifier; there is no authentication or encryption.
- **Instances are never destroyed**, and there is no player limit per instance.
- **Unlimited retries**: pending events have no retry limit or expiry while the
  session is alive.
- **Fixed configuration**: port and tick rate are hard-coded; there is no clean
  shutdown.
- **Client support**: the current client does not create or join instances and
  does not acknowledge events yet.

See also the [network protocol](protocol.md) for the byte layout of every
packet.
