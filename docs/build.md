# Build and run

## Configurations

Run configuration and build commands from the repository root.

| Preset | Host system | Compiler | Build tool | Output directory |
| --- | --- | --- | --- | --- |
| `linux` | Linux | GCC or Clang with C++20 support | Make | `build-linux/` |
| `windows` | Windows | MSVC x64 | Ninja | `build-windows/` |

Both presets select Release and build the client and server. The hidden `base` preset shares the vcpkg toolchain configuration; it cannot be invoked directly. The Windows preset must run on Windows, outside WSL. These presets do not configure cross-compilation.

## Common prerequisites

- CMake 3.20 or later.
- Git.
- Python 3, available to CMake, to generate `assets.pak`.
- vcpkg: follow the [official installation guide](https://learn.microsoft.com/en-us/vcpkg/get_started/get-started?pivots=shell-powershell), selecting the shell for your system.
- `VCPKG_ROOT` set to the absolute path of your local vcpkg installation.

The root `vcpkg.json` declares Asio, SFML, and OpenSSL. CMake requires SFML 3. The presets enable automatic dependency installation through vcpkg during configuration. The first configuration can take several minutes because dependencies are downloaded and compiled, often in both Debug and Release. Subsequent builds reuse installed dependencies and compatible cached packages.

Commit `CMakePresets.json` and `vcpkg.json` so other developers can use the same setup. Machine-specific paths belong in environment variables or a local `CMakeUserPresets.json`, which is ignored by Git.

## Linux: GCC and Make

### Prerequisites

On Ubuntu/Debian, install the compiler, build tools, and system development libraries needed by SFML:

```sh
sudo apt update
sudo apt install build-essential cmake git python3 curl zip unzip tar pkg-config \
  linux-libc-dev libx11-dev libxi-dev libxrandr-dev libxcursor-dev libudev-dev libgl1-mesa-dev
```

Ensure the installed CMake is at least 3.20 and the compiler supports C++20. Other distributions require equivalent packages. The current Linux preset uses Make, so Ninja is not required for this preset.

After installing vcpkg, set its location in Bash or zsh:

```sh
export VCPKG_ROOT="$HOME/vcpkg"
```

Replace `$HOME/vcpkg` with your actual installation directory. Add this line to `~/.zshrc` or `~/.bashrc` to preserve it across terminals, then reload the file or open a new terminal. Adding vcpkg to `PATH` is optional for these CMake commands.

### Configure and build Release

```sh
cmake --preset=linux
cmake --build build-linux --parallel
```

### Run

Start the server from the repository root:

```sh
./build-linux/rtype_server
```

In another terminal, start the client from the build directory so it can find `assets.pak`:

```sh
cd build-linux
./rtype_client 127.0.0.1
```

## Windows: MSVC x64 and Ninja

### Prerequisites

- Visual Studio 2022 Build Tools, with the **Desktop development with C++** workload, the MSVC x64/x86 tools, and a Windows SDK. The Visual Studio editor is optional.
- Ninja, available on `PATH` in the build terminal.
- Windows installations of CMake, Git, Python 3, and vcpkg.

Open **x64 Native Tools Command Prompt for VS 2022**. To use the PowerShell commands below, run `powershell` inside that prompt; it inherits the initialized MSVC x64 environment. An equivalently initialized Developer PowerShell also works.

Check that the tools are available:

```powershell
cl
cmake --version
ninja --version
python --version
```

`cl` should print the Microsoft compiler banner for x64. It may also report that no source file was provided; this check does not compile anything.

Set the local vcpkg path in that PowerShell session:

```powershell
$env:VCPKG_ROOT = "C:\vcpkg"
```

Replace `C:\vcpkg` with your installation path. You may also define `VCPKG_ROOT` permanently in your Windows user environment variables, then reopen the developer terminal.

### Configure and build Release

From the repository root:

```powershell
cmake --preset=windows -DVCPKG_TARGET_TRIPLET=x64-windows
cmake --build build-windows --parallel
```

This uses Ninja with MSVC, not the Visual Studio project generator. Release is selected by `CMAKE_BUILD_TYPE` in the preset; executables are placed directly in `build-windows/`, without a `Release/` subdirectory. Do not add `architecture: x64` to this Ninja preset; select x64 through the compiler environment instead.

### Run

Start the server:

```powershell
.\build-windows\rtype_server.exe
```

In another terminal, start the client from the build directory:

```powershell
cd build-windows
.\rtype_client.exe 127.0.0.1
```

Keep the runtime DLLs supplied by the dependency setup alongside the executables when distributing the build, together with `assets.pak` for the client. A Windows build and runtime check are required to validate Windows compatibility; a successful Linux build alone does not establish it.

## Debug builds

Use separate directories to keep Release builds intact.

Linux:

```sh
cmake --preset=linux -B build-linux-debug -DCMAKE_BUILD_TYPE=Debug
cmake --build build-linux-debug --parallel
```

Windows, in the initialized MSVC x64 environment:

```powershell
cmake --preset=windows -B build-windows-debug -DCMAKE_BUILD_TYPE=Debug -DVCPKG_TARGET_TRIPLET=x64-windows
cmake --build build-windows-debug --parallel
```

Run the binaries from the corresponding Debug directory, using the same commands as above with the directory replaced.

## Subsequent and targeted builds

After changing source code, rerun only the build command for your configuration. To build a specific executable:

```sh
cmake --build build-linux --target rtype_server --parallel
cmake --build build-linux --target rtype_client --parallel
```

On Windows, substitute `build-windows` for `build-linux`. Use the Debug directory instead when appropriate. Configuring still resolves all declared dependencies, even if you subsequently build only the server target.

To rebuild project targets from scratch:

```sh
cmake --build build-linux --clean-first --parallel
```

This does not clear vcpkg's dependency caches.

If an output directory was previously configured with another generator, compiler, or toolchain, use a fresh directory with `-B`, then use that directory in subsequent build and run commands. In particular, do not reuse a Make build directory for Ninja.
