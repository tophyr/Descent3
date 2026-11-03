# Descent3_Online_TCP_IP is a MODULE (runtime-loaded DLL) and can't be linked against,
# so the sources under test are compiled straight into the test executable.
add_executable(
  Descent3_Online_TCP_IP_tests
  chat_api_tests.cpp
  dip_gametrack_tests.cpp
  support/chat_test_helpers.cpp
  support/dip_test_helpers.cpp
  support/test_stubs.cpp
  ../chat_api.cpp
  ../dip_gametrack.cpp
)
target_include_directories(Descent3_Online_TCP_IP_tests PRIVATE
  ${PROJECT_SOURCE_DIR}/netcon/descent3onlineclient
)
target_link_libraries(
  Descent3_Online_TCP_IP_tests
  GTest::gtest_main
  GTest::gmock
  inetfile
  logger
  $<$<PLATFORM_ID:Windows>:ws2_32>
)
gtest_discover_tests(Descent3_Online_TCP_IP_tests)
