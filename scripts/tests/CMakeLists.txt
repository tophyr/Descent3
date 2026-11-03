set(CMAKE_FOLDER "tests")

add_executable(
  dallas_tests
  dallas_uservar_tests.cpp
  dallas_vector_tests.cpp
  support/dallas_fake_engine.cpp
  support/dallas_vector_helpers.cpp
)
target_include_directories(dallas_tests PRIVATE ${PROJECT_SOURCE_DIR}/scripts)
target_link_libraries(
  dallas_tests
  GTest::gtest_main
  dallas
)
gtest_discover_tests(dallas_tests)
