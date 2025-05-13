set(TARGET_NAME grpc-lite)

add_library(${TARGET_NAME} SHARED)

target_sources(${TARGET_NAME}
	PRIVATE
        asio/detail/connection.cxx
        asio/detail/server.cxx
        context.cxx
		detail/message.cxx
        detail/request.cxx
        http2/detail/session.cxx
        server_base.cxx
	PUBLIC
		FILE_SET headers TYPE HEADERS
		BASE_DIRS ${GRPC_LITE_INCLUDE_DIR}
		FILES
            ${GRPC_LITE_INCLUDE_DIR}/grpc-lite/asio/detail/connection.hxx
            ${GRPC_LITE_INCLUDE_DIR}/grpc-lite/asio/detail/server.hxx
            ${GRPC_LITE_INCLUDE_DIR}/grpc-lite/context.hxx
            ${GRPC_LITE_INCLUDE_DIR}/grpc-lite/detail/fixed_string.hxx
			${GRPC_LITE_INCLUDE_DIR}/grpc-lite/detail/message.hxx
            ${GRPC_LITE_INCLUDE_DIR}/grpc-lite/detail/request.hxx
            ${GRPC_LITE_INCLUDE_DIR}/grpc-lite/detail/response.hxx
            ${GRPC_LITE_INCLUDE_DIR}/grpc-lite/grpc-lite.hxx
            ${GRPC_LITE_INCLUDE_DIR}/grpc-lite/http2/detail/event.hxx
            ${GRPC_LITE_INCLUDE_DIR}/grpc-lite/http2/detail/headers.hxx
            ${GRPC_LITE_INCLUDE_DIR}/grpc-lite/http2/detail/session.hxx
            ${GRPC_LITE_INCLUDE_DIR}/grpc-lite/server.hxx
            ${GRPC_LITE_INCLUDE_DIR}/grpc-lite/server_base.hxx
            ${GRPC_LITE_INCLUDE_DIR}/grpc-lite/service.hxx
            ${GRPC_LITE_INCLUDE_DIR}/grpc-lite/status.hxx

)

set_property(TARGET ${TARGET_NAME} PROPERTY PREFIX "")

target_compile_definitions(${TARGET_NAME} PRIVATE GRPC_LITE_EXPORTS)

target_link_libraries(${TARGET_NAME} libnghttp2::libnghttp2)