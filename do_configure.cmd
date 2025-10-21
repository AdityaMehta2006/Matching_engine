@echo off
call "%ProgramFiles(x86)%\Microsoft Visual Studio\2022\BuildTools\Common7\Tools\VsDevCmd.bat" -arch=amd64 -host_arch=amd64
setlocal EnableDelayedExpansion
set "SRC=%~dp0"
:: remove trailing backslash from SRC if present to avoid escaping the closing quote
if "%SRC:~-1%"=="\" set "SRC=%SRC:~0,-1%"
set "BUILD=%USERPROFILE%\BuildTools_build\matching_engine_cpp_build"
:: remove trailing backslash from BUILD if present
if "%BUILD:~-1%"=="\" set "BUILD=%BUILD:~0,-1%"
echo SRC="%SRC%"
echo BUILD="%BUILD%"

:: Look for a vcpkg installation in a couple of common places and add the toolchain file if found
set "VCPKG_TOOLCHAIN="
if exist "%SRC%vcpkg\scripts\buildsystems\vcpkg.cmake" set "VCPKG_TOOLCHAIN=%SRC%vcpkg\scripts\buildsystems\vcpkg.cmake"
set "PF86=%ProgramFiles(x86)%"
if not defined VCPKG_TOOLCHAIN if exist "!PF86!\Microsoft Visual Studio\2022\BuildTools\VC\vcpkg\scripts\buildsystems\vcpkg.cmake" set "VCPKG_TOOLCHAIN=!PF86!\Microsoft Visual Studio\2022\BuildTools\VC\vcpkg\scripts\buildsystems\vcpkg.cmake"

if defined VCPKG_TOOLCHAIN (
  echo Found vcpkg toolchain: !VCPKG_TOOLCHAIN!
  cmake -G "Visual Studio 17 2022" -A x64 -S "%SRC%" -B "%BUILD%" -DCMAKE_TOOLCHAIN_FILE="!VCPKG_TOOLCHAIN!"
) else (
  cmake -G "Visual Studio 17 2022" -A x64 -S "%SRC%" -B "%BUILD%"
)

pause
