# Parallax_Online is a MODULE (runtime-loaded DLL) and cannot be linked
# against, so the sources under test are compiled straight into the test
# executable. stubs.cpp provides the entry points the game normally hands
# the DLL at load time.
add_executable(
  Parallax_Online_tests
  chat_api_tests.cpp
  gametrack_tests.cpp
  pilottrack_tests.cpp
  support/chat_helpers.cpp
  support/chat_helpers.h
  support/chat_internals.h
  support/gametrack_packets.cpp
  support/gametrack_packets.h
  support/pilottrack_packets.cpp
  support/pilottrack_packets.h
  support/stubs.cpp
  support/test_support.h
  ../chat_api.cpp
  ../mt_net.cpp
  ../mtgametrack.cpp
  ../mtpilottracker.cpp
)
target_include_directories(Parallax_Online_tests PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/..
)
target_link_libraries(Parallax_Online_tests PRIVATE
  GTest::gtest_main
  logger
  $<$<PLATFORM_ID:Windows>:ws2_32>
  SDL3::SDL3
)
gtest_discover_tests(Parallax_Online_tests)
