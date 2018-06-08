find_package(GTest REQUIRED)

add_executable(cll-test main.cpp file_event_batch.cpp event_serializer.cpp memory_event_batch.cpp)
target_include_directories(cll-test PRIVATE ${GTEST_INCLUDE_DIRS})
target_link_libraries(cll-test cll-telemetry ${GTEST_LIBRARIES})

add_test(cll-telemetry cll-test)