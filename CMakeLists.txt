cmake_minimum_required(VERSION 3.16)

project(AegisServer VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_compile_options(
    -Wall
    -Wextra
    -Wpedantic
    -O2
)

include_directories(
    ${PROJECT_SOURCE_DIR}
)

add_executable(aegis_server
    app/main.cpp
    core/server/server.cpp
    core/net/epoller.cpp
    core/net/socket_util.cpp
    core/http/http_request.cpp
    core/http/http_response.cpp
    core/http/http_conn.cpp
    core/buffer/buffer.cpp
    core/metrics/metrics.cpp
    modules/guard/connection_guard.cpp
    modules/guard/rate_limiter.cpp
    core/threadpool/thread_pool.cpp
    core/timer/timer_manager.cpp
    core/config/config.cpp
    core/log/logger.cpp
    
)