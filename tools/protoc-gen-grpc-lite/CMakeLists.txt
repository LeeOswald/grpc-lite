set(TARGET_NAME protoc-gen-grpc-lite)

add_executable(${TARGET_NAME})

target_sources(${TARGET_NAME}
	PRIVATE
		grpc-lite.cpp
		main.cpp
	PRIVATE
		FILE_SET private_headers TYPE HEADERS
		FILES
			grpc-lite.hpp
)

target_link_libraries(${TARGET_NAME}
	PRIVATE
		fmt::fmt
		protobuf::libprotobuf
		protobuf::libprotoc
)

target_compile_definitions(${TARGET_NAME}
	PRIVATE
		PLUGIN_NAME="$<TARGET_NAME:${TARGET_NAME}>"
		PLUGIN_VERSION="${PROJECT_VERSION}"
)
