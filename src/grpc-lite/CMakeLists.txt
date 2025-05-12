set(TARGET_NAME grpc-lite)

add_library(${TARGET_NAME} SHARED)

target_sources(${TARGET_NAME}
	PRIVATE
		message.cxx
	PUBLIC
		FILE_SET headers TYPE HEADERS
		BASE_DIRS ${GRPC_LITE_INCLUDE_DIR}
		FILES
			${GRPC_LITE_INCLUDE_DIR}/grpc-lite/message.hxx
)

set_property(TARGET ${TARGET_NAME} PROPERTY PREFIX "")