cmake_minimum_required(VERSION 3.15)
project(autocomplete_hash CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_definitions(_WIN32_WINNT=0x0A00)

include_directories(${CMAKE_SOURCE_DIR}/include)
file(GLOB CORE_SRC
        src/hash/*.cpp
        src/index/*.cpp
        src/util/*.cpp
)

add_executable(autocomplete
        src/main.cpp
        ${CORE_SRC}
)
target_include_directories(autocomplete PRIVATE ${CMAKE_SOURCE_DIR}/include)

add_executable(autocomplete_server
        src/http/server.cpp
        ${CORE_SRC}
)

target_include_directories(autocomplete_server PRIVATE ${CMAKE_SOURCE_DIR}/include)

target_link_libraries(autocomplete_server PRIVATE ws2_32)
