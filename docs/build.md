# Build and run

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

