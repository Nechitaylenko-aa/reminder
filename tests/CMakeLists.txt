add_executable(tests_datetime test_datetime.cpp)

# reminder_core must be available when BUILD_TESTS=ON
target_link_libraries(tests_datetime PRIVATE reminder_core Catch2::Catch2WithMain)

add_test(NAME datetime_tests COMMAND tests_datetime)
