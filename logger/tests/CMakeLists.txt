set(CMAKE_FOLDER "tests")

add_executable(logger_tests
  logger_tests.cpp
  support/logger_test_files.cpp
)
target_link_libraries(logger_tests PRIVATE
  GTest::gtest_main
  logger
)
# Each test creates (and removes) its own subdirectory here. A fixed location is
# needed because death-test children may re-run the test body in a new process.
target_compile_definitions(logger_tests PRIVATE
  LOGGER_TESTS_SCRATCH_DIR="${CMAKE_CURRENT_BINARY_DIR}/scratch"
)

gtest_discover_tests(logger_tests)
