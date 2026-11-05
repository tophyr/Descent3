# Script modules are MODULE libraries and cannot be linked against, so each test
# executable compiles one script source in directly (through a small
# aiscripts_<name>_module.cpp adapter that #includes it). Every script defines the
# same exported entry points (InitializeDLL, GetGOScriptID, ...), so each script
# needs its own test executable.

function(aiscripts_add_test name)
  add_executable(${name}
    ${ARGN}
    support/aiscripts_test_support.cpp
    $<$<PLATFORM_ID:Darwin,Linux,OpenBSD>:${PROJECT_SOURCE_DIR}/scripts/linux_lib.cpp>
  )
  target_include_directories(${name} PRIVATE ${PROJECT_SOURCE_DIR}/scripts)
  target_link_libraries(${name} PRIVATE
    GTest::gtest_main
    cfile
    dallas
    fix
    misc
    module
  )
  gtest_discover_tests(${name} TEST_PREFIX "${name}.")
endfunction()

aiscripts_add_test(aiscripts_aigame_tests support/aiscripts_aigame_module.cpp aiscripts_factory_tests.cpp)
aiscripts_add_test(aiscripts_aigame2_tests support/aiscripts_aigame2_module.cpp aiscripts_factory_tests.cpp)
aiscripts_add_test(aiscripts_aigame3_tests support/aiscripts_aigame3_module.cpp aiscripts_factory_tests.cpp)
aiscripts_add_test(aiscripts_aigame4_tests support/aiscripts_aigame4_module.cpp aiscripts_factory_tests.cpp)

aiscripts_add_test(aiscripts_barney_tests support/aiscripts_barney_module.cpp support/aiscripts_message_files.cpp
                   aiscripts_message_file_tests.cpp)
aiscripts_add_test(aiscripts_y2k_tests support/aiscripts_y2k_module.cpp support/aiscripts_message_files.cpp
                   aiscripts_message_file_tests.cpp)
