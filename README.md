matching_engine_cpp

Quick local project to prototype a matching engine core.

Build (Windows, Visual Studio 2022):

1) Create a build directory and configure:

cmake -G "Visual Studio 17 2022" -S . -B "%USERPROFILE%\\BuildTools_build\\matching_engine_cpp_build"

2) Build:

cmake --build "%USERPROFILE%\\BuildTools_build\\matching_engine_cpp_build" --config Debug -- /m

Notes:
- This prototype avoids Boost and third-party JSON by using a simple atomic counter for order ids.
- If you want vcpkg integration (Boost, nlohmann::json), I can add a `vcpkg.json` manifest and CMake toolchain hint.

Using vcpkg (optional)

1. Install vcpkg (if you don't have it):

	git clone https://github.com/microsoft/vcpkg.git
	cd vcpkg
	.\bootstrap-vcpkg.bat

2. From the project root you can install required packages listed in `vcpkg.json`:

	.\vcpkg\vcpkg.exe install --manifest

3. The provided `do_configure.cmd` will automatically detect a local `vcpkg` (at the project root) or the Visual Studio bundled `vcpkg` location and pass the `vcpkg.cmake` toolchain to CMake.

If you prefer to run CMake manually, pass the toolchain file explicitly:

cmake -G "Visual Studio 17 2022" -A x64 -S . -B "%USERPROFILE%\\BuildTools_build\\matching_engine_cpp_build" -DCMAKE_TOOLCHAIN_FILE=path\\to\\vcpkg\\scripts\\buildsystems\\vcpkg.cmake

Run tests
---------

After building, run the test suite with CTest from the build folder:

```powershell
ctest --output-on-failure -C Debug --test-dir "%USERPROFILE%\\BuildTools_build\\matching_engine_cpp_build"
```

Quick API example
-----------------

Here is a minimal example showing how to create the engine with a trade callback and submit an order:

```cpp
#include "MatchingEngine.h"

int main(){
	MatchingEngine engine([](const Trade &t){
		// called for each executed trade
		std::cout << "Trade: " << t.buy_order_id << " x " << t.quantity << " @ " << t.price << std::endl;
	});

	Order o("", "BTCUSD", Side::BUY, OrderType::LIMIT, 100, 50000.0);
	engine.submit_order(o);
}
```

VS Code integration hints
------------------------

- Use the included `do_configure.cmd` to configure from a developer command prompt; it auto-detects vcpkg if present.
- If you use the CMake Tools extension, set `cmake.configureArgs` to include the vcpkg toolchain file if you want global integration.

Notes and next steps
--------------------

- The current matching engine is a prototype designed for correctness and clarity. Next steps you may want: richer matching edge cases, trade persistence, BBO update callbacks, symbol sharding, and a REST/WebSocket API layer.

WebSocket push support
---------------------

There are two supported ways to get real-time push (trades/BBO):

- Native C++ broadcaster (recommended for single-binary deployments): this uses websocketpp + Asio and requires the full header set. The CMake build will automatically use a system-provided websocketpp (for example installed via vcpkg) if available. To enable:
	- Install websocketpp and asio headers (for vcpkg: `vcpkg install websocketpp asio` and configure with the vcpkg toolchain file).
	- Re-run CMake; when websocketpp headers are found the project will compile `src/ws_broadcaster.cpp` and enable ws://localhost:8081 from the same binary.

- Node bridge (fast fallback): a small Node process polls the server's `/events/poll` endpoint and broadcasts events to WebSocket clients. This is provided in `scripts/` as `ws_bridge.js` and `ws_client.js`. Use this when you don't want to install native websocket headers or need a fast development setup.

The C++ build will compile a small "stub" broadcaster when websocketpp is not available; this keeps the server functional (REST + long-poll) while deferring WebSocket push to the Node bridge.

Quick run (Node bridge)
------------------------

For a fast demo using the Node bridge (no native websocket headers required):

1. Build the server in Release:

```powershell
cmake --build build --config Release --target me_server
```

2. Install Node deps and run the demo (this starts server+bridge+client and posts a demo trade):

```powershell
cd scripts
npm install
npm run demo
```

3. Alternatively start components in separate windows:

```powershell
.\scripts\start-all.ps1
```

Stop everything:

```powershell
.\scripts\stop-all.ps1
```

Native C++ WebSocket (production)
---------------------------------

To build the native broadcaster into the C++ binary, install `websocketpp` and `asio` via vcpkg or a system package manager and re-run CMake so the project detects the headers and compiles `src/ws_broadcaster.cpp` instead of the stub.

Benchmark harness
-----------------

A small micro-benchmark executable is included (`me_benchmark`) that submits a burst of synthetic orders and prints a simple ops/s measurement.

Build and run the benchmark:

```powershell
cmake --build "%USERPROFILE%\\BuildTools_build\\matching_engine_cpp_build" --config Release --target me_benchmark -- /m
.
```

BBO callback
------------

The engine supports an optional BBO callback that is invoked whenever the best bid or best ask changes. Provide a `BBOCallback` when constructing `MatchingEngine` to receive updates.

