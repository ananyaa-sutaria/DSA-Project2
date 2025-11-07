#cmake_minimum_required(VERSION 3.15)
#project(autocomplete_hash CXX)
#
#set(CMAKE_CXX_STANDARD 20)
#set(CMAKE_CXX_STANDARD_REQUIRED ON)
#add_compile_definitions(_WIN32_WINNT=0x0A00)    #needed to compile on catherine's computer
#
#include_directories(${CMAKE_SOURCE_DIR}/include)
#
#file(GLOB SRC
#        src/hash/*.cpp
#        src/index/*.cpp
#        src/util/*.cpp
#        #src/main.cpp
#)
#
#add_executable(autocomplete ${SRC}
#        include/third_party/httplib.h
#        src/http/server.cpp
#        include/index/candidate.hpp
#        include/index/trie_index.hpp
#        src/index/trie_index.cpp)
#
#
## server executable (web UI)
#add_executable(autocomplete_server
#        src/http/server.cpp
#        src/hash/string_pool.cpp
#        src/hash/hash_table.cpp
#        src/index/prefix_index.cpp
#        src/util/json_words.cpp
#)
#
#target_include_directories(autocomplete_server PRIVATE ${CMAKE_SOURCE_DIR}/include)
#
#
cmake_minimum_required(VERSION 3.15)
project(autocomplete_hash CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Setting this for modern Windows APIs, as you requested.
add_compile_definitions(_WIN32_WINNT=0x0A00)

include_directories(${CMAKE_SOURCE_DIR}/include)

# Define CORE_SRC to include all common, shared source files,
# but EXCLUDE the main entry points (main.cpp and server.cpp).
# This prevents the "multiple definition of main" error.
file(GLOB CORE_SRC
        src/hash/*.cpp
        src/index/*.cpp
        src/util/*.cpp
)

# 1. CLI Executable: Uses src/main.cpp as its entry point.
add_executable(autocomplete
        src/main.cpp
        ${CORE_SRC}
)
target_include_directories(autocomplete PRIVATE ${CMAKE_SOURCE_DIR}/include)


# 2. Server Executable (Web UI): Uses src/http/server.cpp as its entry point.
add_executable(autocomplete_server
        src/http/server.cpp
        ${CORE_SRC}
)

target_include_directories(autocomplete_server PRIVATE ${CMAKE_SOURCE_DIR}/include)

# FIX: Link the required Windows Sockets library (ws2_32).
# This resolves all the 'undefined reference to __imp_...' errors from httplib.
target_link_libraries(autocomplete_server PRIVATE ws2_32)
