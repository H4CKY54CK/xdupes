cmake_minimum_required(VERSION 3.19 FATAL_ERROR)

set(CMAKE_CXX_STANDARD 17)

project(xdupes VERSION 0.1.0 LANGUAGES CXX)

add_executable(${PROJECT_NAME} src/main.cpp)

target_sources(${PROJECT_NAME} PRIVATE src/logging.cpp src/parser.cpp src/progressbar.cpp src/threadpool.cpp src/utils.cpp)

target_include_directories(${PROJECT_NAME} PRIVATE include deps/xxhash)
