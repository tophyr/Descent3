set(CMAKE_FOLDER "tests")

# networking.cpp references a handful of symbols that live in the Descent3 executable (command-line args, the app
# database) and, on Windows, in directplay.cpp (which in turn needs the game's player tables). Those are stubbed in
# networking_test_stubs.cpp so that only networking.cpp is pulled out of the static library.
add_executable(networking_tests
  networking_tests.cpp
  support/chrono_timer_env.cpp
  support/networking_test_packets.cpp
  support/networking_test_reliable.cpp
  support/networking_test_stubs.cpp
  support/networking_test_unreliable.cpp
)
target_link_libraries(networking_tests PRIVATE
  GTest::gtest_main
  networking
  cfile
  ddio
  SDL3::SDL3
  $<$<PLATFORM_ID:Windows>:wsock32>
)
gtest_discover_tests(networking_tests)
