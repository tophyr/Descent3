set(CMAKE_FOLDER "tests")

add_executable(ddio_tests
  ddio_tests.cpp
  file_tests.cpp
  support/lock_file.cpp
  support/temp_dir.cpp
)
target_link_libraries(ddio_tests PRIVATE
  GTest::gtest_main
  ddio
)
gtest_discover_tests(ddio_tests)

# The keyboard queue in key.cpp is compiled on its own, with stubs for the platform key handlers, because linking it
# from ddio drags in the SDL key handler and, through it, the renderer and game globals
add_executable(ddio_key_tests
  key_tests.cpp
  support/key_queue.cpp
  support/key_stubs.cpp
  ../key.cpp
)
target_link_libraries(ddio_key_tests PRIVATE
  GTest::gtest_main
  ddebug
)
target_include_directories(ddio_key_tests PRIVATE ${PROJECT_SOURCE_DIR}/ddio)
gtest_discover_tests(ddio_key_tests)

# The timer tests compile their clock directly rather than linking ddio, so both clocks are tested in every build,
# whichever one ENABLE_MOCK_CLOCK puts in the game
add_executable(chrono_timer_tests
  chrono_timer_tests.cpp
  ../chrono_timer.cpp
)
target_link_libraries(chrono_timer_tests PRIVATE
  GTest::gtest_main
)
target_include_directories(chrono_timer_tests PRIVATE ${PROJECT_SOURCE_DIR}/ddio)
gtest_discover_tests(chrono_timer_tests)

add_executable(mock_timer_tests
  mock_timer_tests.cpp
  ../mock_timer.cpp
)
target_compile_definitions(mock_timer_tests PRIVATE MOCK_CLOCK)
target_link_libraries(mock_timer_tests PRIVATE
  GTest::gtest_main
)
target_include_directories(mock_timer_tests PRIVATE ${PROJECT_SOURCE_DIR}/ddio)
gtest_discover_tests(mock_timer_tests)
