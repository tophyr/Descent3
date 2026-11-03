set(CMAKE_FOLDER "tests")

add_executable(mem_tests
  mem_tests.cpp
)
target_link_libraries(mem_tests PRIVATE
  GTest::gtest_main
  mem
)
gtest_discover_tests(mem_tests)
