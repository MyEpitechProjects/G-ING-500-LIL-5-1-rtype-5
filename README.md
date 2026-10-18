# R-Type

R-Type is a multiplayer space shooter project inspired by the arcade game of the same name. Players pilot a spaceship and face enemies together in a shared online game.

The project includes a server that manages the game, game logic for players, enemies and projectiles, and a shared networking module for communication between the server and clients. The client displays the game and sends player input.

## Build

### Prerequisites

- CMake 3.20 or later.
- A C++20-compatible compiler, such as GCC or Clang.
- Make.
- Standalone Asio headers (`asio.hpp`).

Run all commands below from the repository root.

The CMake configuration builds both the server and the client.

### Configure and build

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
```

The first command configures the project in the `build/` directory. The second builds the server and client. Generated files remain in that directory.

To build only the server target and its dependencies:

```sh
cmake --build build --target rtype_server -j
```

After changing the code, simply run the build command again.

### Full rebuild

```sh
cmake --build build --clean-first -j
```

### Run the server

```sh
./build/rtype_server
```

The server currently listens on UDP port **4242**. For the debug build, use `./build-debug/rtype_server`.

### Run the client

Run the client from the build directory so it can find `assets.pak`:

```sh
cd build
./rtype_client 192.168.1.10
```

Replace the example with the server's IPv4 address. Without an address, the client connects to `127.0.0.1` on UDP port **4242**.

## Documentation

- [Networking usage guide](docs/network.md)
- [Network protocol](docs/protocol.md)

### Documentation website

The documentation is built with MkDocs. From the repository root, create a
Python environment and install the documentation dependencies:

```sh
python3 -m venv .venv
source .venv/bin/activate
python -m pip install -r requirements-docs.txt
```

Preview the documentation locally:

```sh
python -m mkdocs serve
```

Open <http://127.0.0.1:8000>. Changes to the Markdown files are reloaded automatically.

Build the static website and check for documentation warnings:

```sh
python -m mkdocs build --strict
```

The generated website is written to `site/`. Edit the pages in `docs/` and the
navigation in `mkdocs.yml`. Keep the build instructions in this README and
`docs/build.md` in sync when the build process changes.
