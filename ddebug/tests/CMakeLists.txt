set(CMAKE_FOLDER "tests")

# error.cpp and debug.cpp are compiled directly into the test so that the
# platform message-box backends (windebug.cpp / lnxdebug.cpp, which open real
# dialogs or trigger breakpoints) can be replaced by the recording fakes in
# fake_message_boxes.cpp.
add_executable(
  ddebug_tests
  ddebug_tests.cpp
  support/fake_message_boxes.cpp
  support/test_predicates.cpp
  ../debug.cpp
  ../error.cpp
)
target_include_directories(ddebug_tests PRIVATE ${PROJECT_SOURCE_DIR}/ddebug)
target_link_libraries(
  ddebug_tests
  GTest::gtest_main
  SDL3::SDL3
  logger
)
gtest_discover_tests(ddebug_tests)
