cmake_minimum_required (VERSION 2.6)

project(cll-telemetry LANGUAGES CXX)

find_package(CURL REQUIRED)

add_library(cll-telemetry include/cll/event_manager.h src/event_manager.cpp include/cll/event.h include/cll/event_uploader.h include/cll/event_upload_request.h include/cll/event_upload_source.h include/cll/configuration.h src/configuration.cpp include/cll/configuration_cache.h include/cll/configuration_manager.h src/json_utils.h src/http/http_request.cpp src/http/http_request.h src/http/http_response.h src/http/curl_error.h)
target_include_directories(cll-telemetry PUBLIC include/)
target_include_directories(cll-telemetry PRIVATE ${CURL_INCLUDE_DIRS})
target_link_libraries(cll-telemetry nlohmann_json logger ${CURL_LIBRARIES})