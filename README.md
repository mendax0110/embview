# embview

Desktop tool for analyzing and debugging embedded devices over Serial, TCP, and UDP.

## Features

- Multi-device connections (Serial / TCP / UDP) with per-device reader threads
- Binary and ASCII protocol parsing (line-based, CSV)
- Real-time plotting with ImPlot (per-channel config, expression transforms, markers)
- Statistics, FFT analysis, hex view, session recording, triggers
- CSV export, log file management, number converter
- CLI mode for headless serial capture (`--cli`)

## Build

Requires CMake 3.22+, a C++20 compiler, and Ninja. All dependencies are fetched automatically via CMake FetchContent.

### Windows (MSVC 2022)

```
configure.bat
build.bat
test.bat
```

### Manual

```
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build
ctest --test-dir build --output-on-failure
```

## Run

```
embview                                          # GUI
embview --cli -p COM3 -b 115200                  # CLI, serial
embview --cli -p COM3 -b 115200 -j              # CLI, JSON output
```

## Project Structure

```
src/core/    Transport, protocol, data store, device manager, triggers, expressions
src/ui/      ImGui panels (plot, connection, log, stats, hex, FFT, recorder, ...)
src/cli/     CLI mode
tests/       GoogleTest unit tests
docs/        Doxyfile
```

## Dependencies

Dear ImGui (docking), ImPlot, GLFW, spdlog, nlohmann/json, libserialport, GoogleTest

## License

See LICENSE file.
