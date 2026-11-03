set(CMAKE_FOLDER "tests")

add_executable(
  hogmaker_tests
  hogmaker_tests.cpp
  support/hogmaker_test_support.cpp
  support/hogmaker_test_support.h
  ${PROJECT_SOURCE_DIR}/tools/HogMaker/HogFormat.cpp
)
target_include_directories(hogmaker_tests PRIVATE
  ${PROJECT_SOURCE_DIR}/tools/HogMaker
  ${PROJECT_SOURCE_DIR}/lib
)
# The game's own HOG reader (cfile) is used as the independent decoder.
target_link_libraries(
  hogmaker_tests
  GTest::gtest_main
  cfile
)
add_dependencies(hogmaker_tests HogMaker)
target_compile_definitions(hogmaker_tests PRIVATE HOGMAKER_EXE="$<TARGET_FILE:HogMaker>")

gtest_discover_tests(hogmaker_tests)
