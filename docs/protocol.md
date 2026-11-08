# R-Type network protocol — version 1

This document describes the binary format currently implemented in
[Paquet.hpp](https://github.com/EpitechPGE3-2026/G-ING-500-LIL-5-1-rtype-5/blob/main/network/include/Paquet.hpp),
[Paquet.cpp](https://github.com/EpitechPGE3-2026/G-ING-500-LIL-5-1-rtype-5/blob/main/network/src/Paquet.cpp) and
[PacketCodec.cpp](https://github.com/EpitechPGE3-2026/G-ING-500-LIL-5-1-rtype-5/blob/main/network/src/PacketCodec.cpp).

## Transport and encoding

- Transport: UDP, one application packet per datagram.
- Version: `1` (`PROTOCOL_VERSION`).
- Maximum accepted size: **1,200 bytes**, including the application header but excluding UDP/IP headers.
- Unsigned 8-, 16- or 32-bit integers; multibyte integers use **big-endian** encoding (most significant byte first).
- No padding: fields are written individually, without copying the in-memory C++ structures.
- Coordinates are encoded by copying the bits of a 32-bit `float` into a `uint32_t`, then writing it in big-endian order.
- The offsets below are expressed in bytes, starting at zero.

## Common header — 8 bytes

| Offset | Size | Field | Description |
| --- | --- | --- | --- |
| 0 | 1 | `type` | Packet type, see the next table |
| 1 | 1 | `version` | Protocol version, must be `1` |
| 2 | 2 | `sequence` | Sequence number, default: `0` |
| 4 | 4 | `sessionId` | Session identifier, default: `0` |

The codec carries `sequence` and `sessionId` without checking their progression,
uniqueness or application-level validity. It does not increment the sequence automatically.

## Packet types

| Value | Name | Intended purpose | Total size |
| --- | --- | --- | --- |
| `1` | `INPUT` | Player input, client → server | 10 bytes |
| `2` | `SNAPSHOT` | Entity states, server → client | `18 + 16 × N` bytes |
| `3` | `HELLO` | Connection request, client → server | 8 bytes |
| `4` | `WELCOME` | Session assignment, server → client | 12 bytes |
| `5` | `CREATE_INSTANCE` | Instance creation request, client → server | 8 bytes |
| `6` | `INSTANCE` | Instance join request (client → server) and answer (server → client) | 12 bytes |
| `7` | `EVENT` | Reliable events, both directions; acknowledgements | 13 bytes + payload |

These directions describe the purpose of the messages; the codec does not enforce
their direction or handle connections or sessions itself. The server behavior
for each packet is described in [Server](server.md).

### HELLO — type 3

Contains only the common header, with no payload.

### WELCOME — type 4

| Offset | Size | Field | Description |
| --- | --- | --- | --- |
| 8 | 4 | `clientId` | Session identifier assigned by the server |

The server also puts the session identifier in `header.sessionId`. The client
must reuse it in the `sessionId` of the packets it sends afterwards.

### CREATE_INSTANCE — type 5

Contains only the common header, with no payload. Asks the server to create a
new instance and to add the sender to it. The server answers with `INSTANCE`.

### INSTANCE — type 6

| Offset | Size | Field | Description |
| --- | --- | --- | --- |
| 8 | 4 | `instanceId` | Instance identifier, `0` (`NO_INSTANCE`) means none |

- Client → server: request to join the instance `instanceId`.
- Server → client: answer to `CREATE_INSTANCE` or to a join request.
  `instanceId` is the instance the client is now in, or `0` if the request
  failed.

### INPUT — type 1

| Offset | Size | Field | Description |
| --- | --- | --- | --- |
| 8 | 2 | `buttons` | Bitmask of pressed buttons |

| Bit | Value | Button |
| --- | --- | --- |
| 0 | `0x0001` | `UP` |
| 1 | `0x0002` | `DOWN` |
| 2 | `0x0004` | `LEFT` |
| 3 | `0x0008` | `RIGHT` |
| 4 | `0x0010` | `FIRE` |

Buttons are combined with bitwise OR: `UP | RIGHT = 0x0009`.
The value `0` means no button is pressed. The codec accepts opposite directions
simultaneously; other bits are rejected.

### SNAPSHOT — type 2

| Offset | Size | Field | Description |
| --- | --- | --- | --- |
| 8 | 4 | `serverTick` | Server tick associated with this state |
| 12 | 4 | `score` | Current game score |
| 16 | 2 | `count` | Number of entities `N` |
| 18 | `16 × N` | `entities` | Consecutive entities, in vector order |

An empty snapshot is valid and occupies 18 bytes. The maximum number of entities is:

```text
MAX_SNAPSHOT_ENTITIES = (1200 - 18) / 16 = 73 (integer division)
Size of a snapshot with 73 entities = 1186 bytes
```

Each entity occupies exactly **16 bytes**. The following offsets are relative
to the start of the entity; the entity at index `i` starts at `18 + 16 × i`.

| Relative offset | Size | Field | Encoding |
| --- | --- | --- | --- |
| 0 | 4 | `id` | `uint32_t` |
| 4 | 1 | `kind` | `uint8_t`: `PLAYER = 1`, `ENEMY = 2`, `PROJECTILE = 3` |
| 5 | 4 | `x` | 32-bit floating-point bits, in big-endian order |
| 9 | 4 | `y` | 32-bit floating-point bits, in big-endian order |
| 13 | 2 | `health` | `uint16_t` |
| 15 | 1 | `flags` | State bitmask |

The defined flags are `ALIVE = 0x01` and `INVULNERABLE = 0x02`.
They can be combined. Other bits in `flags` are currently not rejected by the codec.
Coordinates must be finite: NaN and infinities are rejected.
The codec applies no position limits or application-level constraints to `health` or `id`.

### EVENT — type 7 (`EventPacket`)

Every event starts with the common 8-byte header, followed by a 5-byte event
base, then a payload whose layout depends on `eventType`.

| Offset | Size | Field | Description |
| --- | --- | --- | --- |
| 8 | 1 | `eventType` | Event subtype from the table below |
| 9 | 4 | `eventId` | Identifier used to acknowledge the event |
| 13 | n | `eventData` | Payload, its layout and size `n` depend on `eventType` |

| Value | Name | Payload (`eventData`) | Purpose |
| --- | --- | --- | --- |
| `1` | `NEW_CLIENT` | `EntityEvent` (4 bytes: `entityId`) | Server → client: the game created a player entity |
| `2` | `DESTROY` | `EntityEvent` (4 bytes: `entityId`) | Server → client: the game destroyed an entity |
| `3` | `ACK` | `EmptyEvent` (0 bytes) | Client → server: acknowledgement of the event identified by `eventId` |
| `4` | `DISCONNECTED` | `EntityEvent` (4 bytes) | Server → client: a session timed out; the field carries its **session identifier** |
| `5` | `CHAT` | `ChatEvent` (6 + `length` bytes) | Both directions: text message for every player of the sender's instance |

A packet whose size does not match `13 + n` for its `eventType`, or whose
`eventType` is unknown, is rejected. Serializing an `EventPacket` whose payload
does not match its `eventType` throws `std::invalid_argument`.

#### CHAT payload (`ChatEvent`)

| Offset in payload | Size | Field | Description |
| --- | --- | --- | --- |
| 0 | 4 | `senderId` | Session identifier of the player who wrote the message |
| 4 | 2 | `length` | Text length in bytes, at most `MAX_CHAT_LENGTH` (256) |
| 6 | `length` | `text` | Raw text bytes (UTF-8), no terminating null byte |

The client sends a `CHAT` event to the server with its own `header.sessionId`;
the server ignores the client's `senderId` and `eventId`. If the sender belongs
to an instance and the text is not empty, the server broadcasts a new `CHAT`
event, with its own `eventId` and `senderId` set to the sender's session
identifier, to every session of that instance, sender included. This broadcast
is acknowledged and retried like any other event, so clients must deduplicate
chat messages by `eventId`. The client-to-server message itself is not
acknowledged. A `length` above 256 or a size other than `13 + 6 + length` is
rejected.

The default subtype is `ACK` with an `EmptyEvent` payload, and `eventId`
defaults to `0`. `eventId` is independent of the common header's `sequence` field.

To add an event: add the `EventType` value, a payload struct implementing
`Net::IEvent`, and its case in `PacketCodec::validEventData` and
`PacketCodec::deserializeEvent` (which checks the payload size).

#### Broadcast, acknowledgements and retries

Events are sent to the sessions of one instance. The server assigns an event
identifier and records the event in the pending-event map of every session of
that instance. Each recipient receives its own packet with the same
`eventId` and `eventData`, but with `header.sessionId` set to that recipient's
session identifier. This field does not identify the player announced by
`NEW_CLIENT`.

Pending events are first sent at the end of the server loop iteration. After each
send, the next retry is scheduled for 200 ms later; the actual send takes place
when the server loop next checks pending events. Retries retain the event's
identifier and payload. The attempt counter is incremented, but currently imposes
no retry limit or expiry while the session remains active.

To acknowledge an event, a client sends an `EVENT` with subtype `ACK`, the same
`eventId`, and its own `header.sessionId`. The server removes the pending event
only from the sender endpoint's session, provided the session identifier matches.
An unknown or already acknowledged
event identifier has no effect on the pending-event map. ACKs are not themselves
acknowledged. Removing a session also removes its pending events.

## Validation and errors

`PacketCodec::deserialize()` returns a `std::shared_ptr<IPacket>` holding
the decoded concrete type, or `nullptr` if the packet is invalid.

A packet is rejected if:

- the supplied pointer is null, or the size is below 8 or above 1,200 bytes;
- the version differs from `1` or the type is unknown;
- the size does not exactly match the packet type: truncated data and extra bytes are rejected;
- an `INPUT` contains undefined button bits;
- an `EVENT` has an unknown subtype or a payload size that does not match it;
- a `SNAPSHOT` declares more than 73 entities or a size other than `18 + 16 × count`;
- an entity contains an unknown type or a non-finite coordinate.

The `serialize() const` methods return the packet bytes. They throw
`std::invalid_argument` if the header version or type does not match the packet,
if the buttons are invalid, or if the snapshot contains too many entities or an
invalid entity. An invalid entity is not skipped: snapshot serialization fails.

Serializing an `EVENT` throws if its payload does not match its subtype or if a
chat message is longer than 256 bytes.

## Hexadecimal examples

HELLO, version 1, sequence 0 and session 0:

```text
03 01 00 00 00 00 00 00
```

INPUT, version 1, sequence 1, session 42, UP and RIGHT buttons:

```text
01 01 00 01 00 00 00 2A 00 09
```

WELCOME, version 1, sequence 0, session 42:

```text
04 01 00 00 00 00 00 2A 00 00 00 2A
```

CREATE_INSTANCE from session 42:

```text
05 01 00 00 00 00 00 2A
```

INSTANCE, join request or answer for instance 3, session 42:

```text
06 01 00 00 00 00 00 2A 00 00 00 03
```

Empty SNAPSHOT, version 1, sequence 2, session 42, server tick 100 and score 0:

```text
02 01 00 02 00 00 00 2A 00 00 00 64 00 00 00 00 00 00
```

DESTROY event 100, recipient session 42, entity 7:

```text
07 01 00 00 00 00 00 2A 02 00 00 00 64 00 00 00 07
```

ACK of event 100 from session 42:

```text
07 01 00 00 00 00 00 2A 03 00 00 00 64
```

CHAT event 101, recipient session 42, sent by session 5, text `hi`:

```text
07 01 00 00 00 00 00 2A 05 00 00 00 65 00 00 00 05 00 02 68 69
```

## Scope of the current protocol

The format defines acknowledgements for game events, with server-side pending-event
tracking and retransmission as described above. Snapshots are not acknowledged or
retransmitted, and there is no reassembly of snapshots split across multiple
datagrams. The codec does not handle UDP packet loss,
duplication or reordering. It provides no authentication, encryption or validation
that a sender belongs to a session. These behaviors must be implemented in the
networking and application layers.
