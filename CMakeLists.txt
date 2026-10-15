cmake_minimum_required(VERSION 3.20)

project(RType LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Threads REQUIRED)
find_path(ASIO_INCLUDE_DIR asio.hpp REQUIRED)

add_library(rtype_network STATIC
    network/src/NetworkHandler.cpp
    network/src/PacketCodec.cpp
    network/src/Paquet.cpp
    network/src/WakeSignal.cpp
)

target_include_directories(rtype_network PUBLIC
    network/include
    ${ASIO_INCLUDE_DIR}
)

target_link_libraries(rtype_network PUBLIC
    Threads::Threads
)

add_library(rtype_game STATIC
    game/src/Game.cpp
    game/src/GameSystems.cpp
)

target_include_directories(rtype_game PUBLIC
    game/include
)

target_link_libraries(rtype_game PUBLIC
    rtype_network
)

add_executable(rtype_server
    server/src/main.cpp
    server/src/Server.cpp
)

target_include_directories(rtype_server PRIVATE
    server/include
)

target_link_libraries(rtype_server PRIVATE
    rtype_network
    rtype_game
)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    foreach(target rtype_network rtype_game rtype_server)
        target_compile_options(${target} PRIVATE
            -Wall -Wextra -Werror
        )
    endforeach()
endif()