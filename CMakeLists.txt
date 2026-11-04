cmake_minimum_required(VERSION 3.20)

project(RType LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Threads REQUIRED)
find_path(ASIO_INCLUDE_DIR asio.hpp REQUIRED)
find_package(Python3 REQUIRED COMPONENTS Interpreter)
find_package(SFML 3 REQUIRED COMPONENTS Graphics Window System)
find_package(OpenSSL REQUIRED COMPONENTS Crypto)

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

file(GLOB_RECURSE ENGINE_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/engine/src/*.cpp"
)

add_library(rtype_engine STATIC ${ENGINE_SOURCES})

target_include_directories(rtype_engine PUBLIC
    engine/include
    engine/include/components
    engine/include/modules
)

file(GLOB_RECURSE GAME_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/game/src/*.cpp"
)

add_library(rtype_game STATIC ${GAME_SOURCES})

target_include_directories(rtype_game PUBLIC
    game/include
    game/include/components
    game/include/messages
)

target_link_libraries(rtype_game PUBLIC
    rtype_engine
    rtype_network
)

add_executable(rtype_server
    server/src/main.cpp
    server/src/Server.cpp
    server/src/Instance.cpp
)

target_include_directories(rtype_server PRIVATE
    server/include
)

target_link_libraries(rtype_server PRIVATE
    rtype_network
    rtype_game
)

file(GLOB_RECURSE CLIENT_ASSETS CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/client/assets/*"
)

add_custom_command(
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/assets.pak"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/client/assets"
        "${CMAKE_CURRENT_BINARY_DIR}/client-assets"
    COMMAND Python3::Interpreter
        "${CMAKE_CURRENT_SOURCE_DIR}/client/tools/pack_assets.py"
        "${CMAKE_CURRENT_BINARY_DIR}/client-assets"
        "${CMAKE_CURRENT_BINARY_DIR}/assets.pak"
    DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/client/tools/pack_assets.py"
        ${CLIENT_ASSETS}
    VERBATIM
)

add_custom_target(rtype_client_assets
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/assets.pak"
)

add_executable(rtype_client
    client/src/main.cpp
    client/src/App.cpp
    client/src/AssetsPak.cpp
    client/src/Animation.cpp
    client/src/Resources.cpp
    client/src/ClientNet.cpp
    client/src/ecs/Systems.cpp
)

target_include_directories(rtype_client PRIVATE
    client/include
)

target_link_libraries(rtype_client PRIVATE
    rtype_network
    rtype_engine
    SFML::Graphics
    SFML::Window
    SFML::System
    OpenSSL::Crypto
)

add_dependencies(rtype_client rtype_client_assets)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    foreach(target rtype_network rtype_engine rtype_game rtype_server)
        target_compile_options(${target} PRIVATE
            -Wall -Wextra -Werror
        )
    endforeach()
endif()
