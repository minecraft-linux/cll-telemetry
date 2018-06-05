cmake_minimum_required (VERSION 2.6)

project(cll-telemetry LANGUAGES CXX)

find_package(CURL REQUIRED)

add_library(cll-telemetry include/cll/event_manager.h src/event_manager.cpp include/cll/event.h)
target_include_directories(cll-telemetry PUBLIC include/)
target_include_directories(cll-telemetry PRIVATE ${CURL_INCLUDE_DIRS})
target_link_libraries(cll-telemetry nlohmann_json ${CURL_LIBRARIES})