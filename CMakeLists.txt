cmake_minimum_required(VERSION 3.15)
project(autocomplete_hash CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_compile_options(-O3 -march=native -DNDEBUG -Wall -Wextra -pedantic)

include_directories(${CMAKE_SOURCE_DIR}/include)

file(GLOB SRC
        src/hash/*.cpp
        src/index/*.cpp
        src/util/*.cpp
        src/main.cpp
)

add_executable(autocomplete ${SRC})
