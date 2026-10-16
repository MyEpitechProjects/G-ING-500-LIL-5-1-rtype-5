# Game engine

The engine in `game/` runs the server-side simulation. It uses an entity-component-system (ECS) design: entities are identifiers, components store data, and systems apply rules to entities that have the required components. The current implementation is specific to this game; it is not a separate general-purpose engine.

## Entities and components

`Entity` wraps a `std::size_t` identifier. `Game` assigns increasing IDs with `createEntity()` and keeps active entities in `_entities`. IDs are not reused. Destroying an entity removes it from that list, clears its registered components, and queues an `EntityDestroyed` output.

Each component type has a `SparseArray<Component>`: a `std::vector<std::optional<Component>>` indexed by entity ID. An empty slot means that the entity does not have that component. `insert_at(id, value)` grows the array if necessary; `erase(id)` clears the slot without shifting other IDs. Code that reads a slot must check the array size and whether the optional contains a value.

`Registry` stores these arrays by C++ type in an `std::unordered_map<std::type_index, std::any>`. Types must be registered before `get_components<T>()` is called. `Game::init()` registers `Position`, `Velocity`, `Health`, `Controllable`, `Player`, `Enemy`, `Projectile`, and `CoolDown`. `Drawable` is declared but is not registered or used by this simulation. When an entity is destroyed, `Registry::erase_entity()` calls the eraser installed for each registered type. Template definitions for `SparseArray` and `Registry` are in their headers so they are visible at instantiation sites.

| Component | Data used by the simulation |
| --- | --- |
| `Position`, `Velocity` | Integer coordinates and movement per tick. |
| `Health` | Remaining hit points. |
| `Controllable` | Up, down, left, right, and fire states. |
| `Player` | Player ID supplied by the server. |
| `Enemy` | Marks an enemy. |
| `Projectile` | Records whether the shot came from a player. |
| `CoolDown` | Time remaining before another player shot. |

## Systems and update order

The systems in `GameSystems.cpp` are callable classes. Most use `containers::zipper` to visit matching component slots, skipping entities that lack a required component; its iterator can also provide the entity ID through `index()`. Systems that create or destroy entities during such a traversal defer those changes until the iteration ends. `Game::update()` invokes the systems explicitly in this order; there is no automatic scheduler:

1. Increment the tick and drain the input queue. `InputQueueSystem` adds or removes players and assigns their latest controls.
2. Measure elapsed wall-clock time, then reduce shooting cooldowns with `CooldownSystem`.
3. Convert controls into player velocity with `ControlSystem`; `ShotSystem` creates player projectiles when fire is held and the cooldown is zero.
4. `EnemySystem` spawns enemies and their projectiles according to the tick count.
5. `PositionSystem` adds velocity to position. `BoundsSystem` clamps players to the playfield and removes other entities after they leave its margins.
6. `ProjectileCollisionSystem` applies projectile damage and updates the score. `EnemyCollisionSystem` handles contact between enemies and players.
7. `SnapshotSystem` queues the state of active entities, then the game notifies the shared `WakeSignal`.

Collision checks use positions from the current tick. The overlap rule is `abs(dx) <= 16 && abs(dy) <= 12`; there is no separate physics engine or sprite-based collision shape.

## Timing and server integration

`Game::run()` calls `update()` at a target rate of 60 ticks per second using `std::chrono::steady_clock` and `sleep_until`. When an update overruns, the next target is reset to the current time; missed ticks are not replayed. Movement uses integer displacement per tick, while the player shooting cooldown uses elapsed real time.

The server owns a `Game`, a `NetworkHandler`, and a shared `WakeSignal`. It runs the game and Asio networking in separate threads. Its main loop wakes on queued work or after 500 ms, forwards accepted network inputs to `Game::addInputQueue()`, and drains `Game::takeOutputQueue()`. Both queues are protected by mutexes. The game state and registry themselves are updated in the game thread; external code should use the queues rather than access them concurrently. The server currently converts and broadcasts `Snapshot` outputs. Other game outputs, including `PlayerCreated` and `EntityDestroyed`, are queued internally but are not handled separately by the server loop.

For game rules and the current player experience, see [Game](game.md). For UDP packet details, see [Network protocol](protocol.md).
