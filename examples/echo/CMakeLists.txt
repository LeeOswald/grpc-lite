cmake_path(SET echo_proto  ${CMAKE_CURRENT_SOURCE_DIR}/echo.proto)
cmake_path(SET echo_header ${CMAKE_CURRENT_BINARY_DIR}/echo.pb.h)
cmake_path(SET echo_source ${CMAKE_CURRENT_BINARY_DIR}/echo.pb.cc)

set(protos	${echo_proto})
set(headers	${echo_header})
set(sources	${echo_source})

add_custom_command(
	OUTPUT  ${headers} ${sources}
	DEPENDS ${protos}
	COMMAND ${Protobuf_PROTOC_EXECUTABLE}
	ARGS
		--proto_path=${CMAKE_CURRENT_SOURCE_DIR}
		--proto_path=${Protobuf_INCLUDE_DIR}
		--cpp_out=${CMAKE_CURRENT_BINARY_DIR}
		${protos}
)

add_library(echo-proto STATIC ${sources})
target_link_libraries(echo-proto PUBLIC protobuf::libprotobuf)

set(TARGET_NAME echo)

add_executable(${TARGET_NAME}
    main.cpp
)

target_link_libraries(${TARGET_NAME} PRIVATE echo-proto libgrpc_lite::grpc_lite)
