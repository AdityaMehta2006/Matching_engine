RUNNING.md
==========

Quick commands to build, test, demo and push this project on Windows (PowerShell).

1) Configure & build (Visual Studio 2022 generator)

```powershell
cmake -G "Visual Studio 17 2022" -S . -B "%USERPROFILE%\BuildTools_build\matching_engine_cpp_build"
cmake --build "%USERPROFILE%\BuildTools_build\matching_engine_cpp_build" --config Debug -- /m
```

2) Run unit tests

```powershell
ctest --output-on-failure -C Debug --test-dir "%USERPROFILE%\BuildTools_build\matching_engine_cpp_build"
```

3) Quick server demo (Node bridge, default ports: HTTP 8080, WS bridge 8081)

Build me_server in Release first:

```powershell
cmake --build "%USERPROFILE%\BuildTools_build\matching_engine_cpp_build" --config Release --target me_server -- /m
```

Start components from project root:

```powershell
# start server (new window)
Start-Process -NoNewWindow -FilePath "%USERPROFILE%\BuildTools_build\matching_engine_cpp_build\Release\me_server.exe"

# in scripts/ run the Node bridge and demo client
cd scripts
npm install
npm run demo
```

4) Integration test (runs server + posts demo orders)

From project root (requires PowerShell):

```powershell
.
cmake --build "%USERPROFILE%\BuildTools_build\matching_engine_cpp_build" --config Release --target integration_test -- /m
```

5) Pushing changes safely

- Use SSH (recommended): add your key to GitHub and run `.
  scripts\push_to_github.ps1` from repo root.
- If you must use HTTPS, prefer Git Credential Manager or `scripts\push_with_pat.ps1` for one-time PAT pushes. Revoke any PATs posted publicly immediately.

6) Enabling native WebSocket in C++

- Option A (vcpkg): clone `vcpkg` to the repo root or install it system-wide, then run `vcpkg install websocketpp asio nlohmann-json boost-uuid` and re-run CMake with the toolchain file.
- Option B (FetchContent): the project will attempt to FetchContent websocketpp/asio during configure if system headers are absent. You can force native build with:

```powershell
cmake -S . -B build -DFORCE_NATIVE_WEBSOCKET=ON
```

If you run into header or build issues, share the `cmake` output and I’ll help debug.
