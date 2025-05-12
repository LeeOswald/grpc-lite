# Boost
find_package(Boost REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})
link_directories(${Boost_LIBRARY_DIRS})

# {fmt}
find_package(fmt REQUIRED)

# GTest
find_package(GTest REQUIRED)

# libnghttp2
find_package(libnghttp2 REQUIRED)

# protobuf
find_package(protobuf REQUIRED)
