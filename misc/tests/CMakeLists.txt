set(CMAKE_FOLDER "tests")

add_executable(
  misc_tests
  holder_tests.cpp
  misc_tests.cpp
  psglob_tests.cpp
  psrand_tests.cpp
  support/psglob_match.cpp
  support/psrand_draw.cpp
)
target_link_libraries(
  misc_tests
  GTest::gtest_main
  misc
)
gtest_discover_tests(misc_tests)
