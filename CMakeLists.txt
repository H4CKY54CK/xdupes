cmake_minimum_required(VERSION 3.25 FATAL_ERROR)

project(xdupes VERSION 0.1.1 LANGUAGES CXX)

add_executable(${PROJECT_NAME} src/main.cpp)

target_sources(${PROJECT_NAME} PRIVATE src/logging.cpp src/progressbar.cpp src/threadpool.cpp src/utils.cpp)

target_include_directories(${PROJECT_NAME} PRIVATE include deps/xxhash)
target_include_directories(${PROJECT_NAME} PRIVATE include deps/parsing/include)

add_subdirectory(deps/parsing)

target_link_libraries(${PROJECT_NAME} PRIVATE parsing)
