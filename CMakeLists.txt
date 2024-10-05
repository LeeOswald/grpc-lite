cmake_minimum_required(VERSION 3.27)
project(grpc-lite VERSION 0.4.0 LANGUAGES CXX)

cmake_policy(SET CMP0135 NEW)
cmake_policy(SET CMP0144 NEW)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/a")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

# boost.asio
find_package(Boost 1.81 REQUIRED)
add_library(asio INTERFACE)
target_include_directories(asio INTERFACE ${Boost_INCLUDE_DIR})
target_compile_definitions(asio
    INTERFACE
        BOOST_ASIO_STANDALONE
        BOOST_ASIO_NO_DEPRECATED
)

# protobuf
find_package(Protobuf REQUIRED)

# {fmt}
add_subdirectory(3rd_party/fmt)


add_subdirectory(tools)