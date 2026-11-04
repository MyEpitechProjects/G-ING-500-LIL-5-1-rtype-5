# Client

Everything in the client is an entity of the engine's `Registry` (same `Engine`,
`SparseArray`, `Zipper` and functor-system convention as the server).

- **Scenes** (`App`): Connect (address/port fields) -> Lobby (room list from
  `LIST_INSTANCES`, *Join* / *Create* buttons) -> Game. UI widgets are entities too
  (`UiBox`, `UiText`, `UiInput`, `UiButton`).
- **Network** (`ClientNet`): HELLO -> Lobby -> CREATE_INSTANCE/INSTANCE -> InGame, with retries and timeouts.
- **Interpolation**: remote entities keep a `SnapshotBuffer` (positions by server time) and are
  drawn `INTERP_DELAY` (2 ticks) in the past, interpolated between two snapshots. `ServerClock`
  maps local time to server time using the fastest packets. Missing snapshot: extrapolation capped at 100 ms.
- **Local ship**: predicted from the keys (same speed/bounds as the server), eased back to the
  server position once idle, snapped if the gap exceeds 120 px.
- Font: DejaVu Sans (`client/assets/fonts`, free licence).
