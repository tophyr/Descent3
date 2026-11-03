set(CMAKE_FOLDER "tests")

# Direct_TCP_IP is a MODULE library and cannot be linked. lanclient_tests.cpp
# #includes lanclient.cpp so the tests can reach the game-API pointers defined
# in con_dll.h and replace them with fakes.
add_executable(
  lanclient_tests
  lanclient_tests.cpp
  support/fake_host.h
)
target_link_libraries(
  lanclient_tests
  GTest::gtest_main
  ddio
  inetfile
  logger
  misc
  module
  ui
  $<$<PLATFORM_ID:Windows>:ws2_32>
  SDL3::SDL3
)
target_include_directories(lanclient_tests PRIVATE ${PROJECT_SOURCE_DIR}/netcon/lanclient)
gtest_discover_tests(lanclient_tests)
