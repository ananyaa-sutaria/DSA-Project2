cmake_minimum_required(VERSION 3.15)
project(autocomplete_hash CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(${CMAKE_SOURCE_DIR}/include)

file(GLOB SRC
        src/hash/*.cpp
        src/index/*.cpp
        src/util/*.cpp
        src/main.cpp
)

add_executable(autocomplete ${SRC}
        include/third_party/httplib.h
        src/http/server.cpp)


# server executable (web UI)
add_executable(autocomplete_server
        src/http/server.cpp
        src/hash/string_pool.cpp
        src/hash/hash_table.cpp
        src/index/prefix_index.cpp
        src/util/json_words.cpp
)

target_include_directories(autocomplete_server PRIVATE ${CMAKE_SOURCE_DIR}/include)
