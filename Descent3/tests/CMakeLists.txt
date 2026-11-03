set(CMAKE_FOLDER "tests")

add_executable(
  porting-tests
  porting-tests.cpp
)

target_link_libraries(
  porting-tests
  module
  GTest::gtest_main
)

gtest_discover_tests(porting-tests)

add_executable(
  gamespyutils_tests
  gamespyutils_tests.cpp
  ../gamespyutils.cpp
)
target_link_libraries(
  gamespyutils_tests
  GTest::gtest_main
)
gtest_discover_tests(gamespyutils_tests)

add_executable(
  args_tests
  args_tests.cpp
  support/gather_args.cpp
  ../args.cpp
)
target_link_libraries(
  args_tests
  GTest::gtest_main
  logger
)
gtest_discover_tests(args_tests)

add_executable(
  list_tests
  list_tests.cpp
  support/ignore_assertion.cpp
  support/list_walk.cpp
  ../list.cpp
)
target_link_libraries(
  list_tests
  GTest::gtest_main
  SDL3::SDL3
  ddebug
  logger
  mem
)
gtest_discover_tests(list_tests)

add_executable(
  localization_tests
  localization_tests.cpp
  support/string_table.cpp
  ../localization.cpp
)
target_link_libraries(
  localization_tests
  GTest::gtest_main
  cfile
  ddebug
  ddio
  logger
  mem
)
gtest_discover_tests(localization_tests)

add_executable(
  briefingparse_tests
  briefingparse_tests.cpp
  support/briefing_parse.cpp
  support/briefing_recorder.cpp
  ../BriefingParse.cpp
)
target_link_libraries(
  briefingparse_tests
  GTest::gtest_main
  cfile
  ddebug
  grtext
  logger
  mem
)
gtest_discover_tests(briefingparse_tests)

# Header-only helpers; the libraries are linked only for their include directories.
add_executable(
  multi_pack_tests
  multi_pack_tests.cpp
)
target_link_libraries(
  multi_pack_tests
  GTest::gtest_main
  networking
)
gtest_discover_tests(multi_pack_tests)

add_executable(
  slider_math_tests
  slider_math_tests.cpp
  support/slider_settings.cpp
)
target_link_libraries(
  slider_math_tests
  GTest::gtest_main
  grtext
  networking
  ui
)
gtest_discover_tests(slider_math_tests)
