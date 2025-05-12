set(TARGET_NAME grpc-lite)

add_library(${TARGET_NAME} SHARED)

target_sources(${TARGET_NAME}
	PRIVATE
		detail/message.cxx
        detail/request.cxx
	PUBLIC
		FILE_SET headers TYPE HEADERS
		BASE_DIRS ${GRPC_LITE_INCLUDE_DIR}
		FILES
            ${GRPC_LITE_INCLUDE_DIR}/grpc-lite/grpc-lite.hxx
            ${GRPC_LITE_INCLUDE_DIR}/grpc-lite/status.hxx
            ${GRPC_LITE_INCLUDE_DIR}/grpc-lite/detail/fixed_string.hxx
			${GRPC_LITE_INCLUDE_DIR}/grpc-lite/detail/message.hxx
            ${GRPC_LITE_INCLUDE_DIR}/grpc-lite/detail/request.hxx
            ${GRPC_LITE_INCLUDE_DIR}/grpc-lite/detail/response.hxx
)

set_property(TARGET ${TARGET_NAME} PROPERTY PREFIX "")

target_compile_definitions(${TARGET_NAME} PRIVATE GRPC_LITE_EXPORTS)