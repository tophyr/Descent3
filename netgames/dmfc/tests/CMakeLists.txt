set(CMAKE_FOLDER "tests")

add_executable(
  dmfc_tests
  support/dmfc_builders.cpp
  support/dmfc_fakes.cpp
  support/dmfc_test_support.cpp
  support/hosts_file.cpp
  support/ice_util.cpp
  support/pinfo_util.cpp
  dmfcbase_tests.cpp
  encryption_tests.cpp
  hosts_tests.cpp
  inputcommand_tests.cpp
  pinfo_tests.cpp
  precord_tests.cpp
)
target_include_directories(dmfc_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(
  dmfc_tests
  GTest::gtest_main
  cfile
  dmfc
)
gtest_discover_tests(dmfc_tests)
