cmake_minimum_required(VERSION 3.15)
project(matching_engine_cpp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# library
add_library(matching_engine
    src/Order.cpp
    src/OrderBook.cpp
    src/MatchingEngine.cpp
)

target_include_directories(matching_engine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_compile_features(matching_engine PUBLIC cxx_std_17)

# If vcpkg/toolchain provided, prefer imported targets for json/uuid
find_package(nlohmann_json QUIET)
find_package(Boost 1.65 QUIET COMPONENTS uuid)
if(nlohmann_json_FOUND)
  message(STATUS "Found nlohmann_json: ${nlohmann_json_VERSION}")
  target_link_libraries(matching_engine PUBLIC nlohmann_json::nlohmann_json)
endif()
if(Boost_UUID_FOUND OR Boost_FOUND)
  if(TARGET Boost::uuid)
    target_link_libraries(matching_engine PUBLIC Boost::uuid)
  elseif(Boost_UUID_FOUND)
    target_link_libraries(matching_engine PUBLIC ${Boost_UUID_LIBRARIES})
    target_include_directories(matching_engine PUBLIC ${Boost_INCLUDE_DIRS})
  endif()
endif()

# CLI
add_executable(me_cli src/cli.cpp)
target_link_libraries(me_cli PRIVATE matching_engine)

# micro-benchmark harness
add_executable(me_benchmark src/benchmark.cpp)
target_link_libraries(me_benchmark PRIVATE matching_engine)


# Tests (Catch2 single-header download to avoid requiring git)
set(CATCH_SINGLE_HEADER ${CMAKE_BINARY_DIR}/third_party/catch.hpp)
if(NOT EXISTS ${CATCH_SINGLE_HEADER})
  file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/third_party)
  message(STATUS "Downloading Catch2 single header to ${CATCH_SINGLE_HEADER}")
  file(DOWNLOAD
    "https://github.com/catchorg/Catch2/releases/download/v2.13.10/catch.hpp"
    ${CATCH_SINGLE_HEADER}
    SHOW_PROGRESS
    STATUS _dl_status
  )
  list(GET _dl_status 0 _dl_code)
  if(NOT _dl_code EQUAL 0)
    message(FATAL_ERROR "Failed to download Catch2 single header: ${_dl_status}")
  endif()
endif()

add_executable(tests tests/test_matching_engine.cpp)
target_include_directories(tests PRIVATE ${CMAKE_BINARY_DIR}/third_party)
target_link_libraries(tests PRIVATE matching_engine)

enable_testing()
add_test(NAME matching_engine_tests COMMAND tests)


# Download cpp-httplib single header for the REST server
set(HTTPLIB_HEADER ${CMAKE_BINARY_DIR}/third_party/httplib.h)
if(NOT EXISTS ${HTTPLIB_HEADER})
  file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/third_party)
  message(STATUS "Downloading cpp-httplib single header to ${HTTPLIB_HEADER}")
  file(DOWNLOAD "https://raw.githubusercontent.com/yhirose/cpp-httplib/master/httplib.h" ${HTTPLIB_HEADER} SHOW_PROGRESS STATUS _dl_status)
  list(GET _dl_status 0 _dl_code)
  if(NOT _dl_code EQUAL 0)
    message(FATAL_ERROR "Failed to download cpp-httplib single header: ${_dl_status}")
  endif()
endif()

# Simple REST/WebSocket server (httplib single header)
add_executable(me_server src/server.cpp)
# Prefer a system-installed websocketpp (via vcpkg or system include). If not found,
# do NOT compile the native `ws_broadcaster.cpp` (it requires the full websocketpp
# header tree). Instead build a lightweight stub so the server still builds and
# the project can use the Node bridge as a fallback.
find_path(WEBSOCKETPP_SYSTEM_INCLUDE websocketpp/server.hpp)
if(WEBSOCKETPP_SYSTEM_INCLUDE)
  message(STATUS "Found websocketpp headers in system includes: ${WEBSOCKETPP_SYSTEM_INCLUDE}")
  target_sources(me_server PRIVATE src/ws_broadcaster.cpp)
else()
  message(STATUS "websocketpp not found in system includes; building stub broadcaster (no-op)")
  target_sources(me_server PRIVATE src/ws_broadcaster_stub.cpp)
endif()

target_include_directories(me_server PRIVATE ${CMAKE_BINARY_DIR}/third_party)
target_link_libraries(me_server PRIVATE matching_engine)
if(nlohmann_json_FOUND)
  target_link_libraries(me_server PRIVATE nlohmann_json::nlohmann_json)
endif()

# Fallback: download nlohmann single header into third_party if package not found
set(NLOHMANN_SINGLE_HEADER ${CMAKE_BINARY_DIR}/third_party/nlohmann/json.hpp)
if(NOT EXISTS ${NLOHMANN_SINGLE_HEADER})
  file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/third_party/nlohmann)
  message(STATUS "Downloading single-header nlohmann/json.hpp to ${NLOHMANN_SINGLE_HEADER}")
  file(DOWNLOAD "https://raw.githubusercontent.com/nlohmann/json/develop/single_include/nlohmann/json.hpp" ${NLOHMANN_SINGLE_HEADER} SHOW_PROGRESS STATUS _dl_status)
  list(GET _dl_status 0 _dl_code)
  if(NOT _dl_code EQUAL 0)
    message(WARNING "Failed to download nlohmann single header: ${_dl_status}")
  endif()
endif()

# Ensure matching_engine can include third_party headers (for json when vendored)
target_include_directories(matching_engine PUBLIC ${CMAKE_BINARY_DIR}/third_party)

# WebSocket++ (header-only) + standalone Asio single-header fallback
set(WEBSOCKETPP_HEADER_DIR ${CMAKE_BINARY_DIR}/third_party/websocketpp)
set(ASIO_HEADER ${CMAKE_BINARY_DIR}/third_party/asio.hpp)
if(NOT EXISTS ${WEBSOCKETPP_HEADER_DIR}/websocketpp)
  file(MAKE_DIRECTORY ${WEBSOCKETPP_HEADER_DIR})
  message(STATUS "Downloading websocketpp headers to ${WEBSOCKETPP_HEADER_DIR}")
  file(DOWNLOAD "https://raw.githubusercontent.com/zaphoyd/websocketpp/master/websocketpp/logger/levels.hpp" ${WEBSOCKETPP_HEADER_DIR}/logger_levels.hpp SHOW_PROGRESS STATUS _dl_status)
  # Instead of downloading entire websocketpp repo, get the packaged single-folder via zip is complex; try to download core headers used below
  file(DOWNLOAD "https://raw.githubusercontent.com/zaphoyd/websocketpp/master/websocketpp/config/asio_no_tls.hpp" ${WEBSOCKETPP_HEADER_DIR}/config_asio_no_tls.hpp SHOW_PROGRESS STATUS _dl_status2)
  file(DOWNLOAD "https://raw.githubusercontent.com/zaphoyd/websocketpp/master/websocketpp/server.hpp" ${WEBSOCKETPP_HEADER_DIR}/server.hpp SHOW_PROGRESS STATUS _dl_status3)
  file(DOWNLOAD "https://raw.githubusercontent.com/zaphoyd/websocketpp/master/websocketpp/connection.hpp" ${WEBSOCKETPP_HEADER_DIR}/connection.hpp SHOW_PROGRESS STATUS _dl_status4)
endif()
if(NOT EXISTS ${ASIO_HEADER})
  file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/third_party)
  message(STATUS "Downloading standalone Asio single-header to ${ASIO_HEADER}")
  file(DOWNLOAD "https://raw.githubusercontent.com/chriskohlhoff/asio/asio-1-24-1/include/asio.hpp" ${ASIO_HEADER} SHOW_PROGRESS STATUS _dl_status5)
endif()

target_include_directories(me_server PRIVATE ${WEBSOCKETPP_HEADER_DIR} ${CMAKE_BINARY_DIR}/third_party)

# Integration test target (runs a PowerShell script that starts the server, posts orders, then shuts it down)
add_custom_target(integration_test
  COMMAND ${CMAKE_COMMAND} -E echo "Running integration_test script"
  COMMAND powershell -NoProfile -ExecutionPolicy Bypass -File ${CMAKE_SOURCE_DIR}/scripts/integration_test.ps1 -ExePath $<TARGET_FILE:me_server>
  DEPENDS me_server
)

