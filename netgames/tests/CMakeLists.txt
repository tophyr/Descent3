set(CMAKE_FOLDER "tests")

# Netgame modes are MODULE targets and cannot be linked against, so each mode gets its own test
# executable: the test file #includes the mode's main source (making its file-local helpers
# reachable), any other sources of the mode are compiled in, and the static dmfc framework the
# mode builds on is linked as usual.
function(add_netgame_test mode)
  set(target netgame_${mode}_tests)
  add_executable(${target} ${mode}_tests.cpp support/ranking_contract.cpp support/ranking_contract.h
    support/mode_score_hooks.h support/anarchy_score_hooks.h ${ARGN})
  target_include_directories(${target} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../${mode})
  target_link_libraries(${target} PRIVATE dmfc GTest::gtest_main)
  gtest_discover_tests(${target} TEST_PREFIX "${mode}." NO_PRETTY_VALUES)
endfunction()

add_netgame_test(anarchy)
add_netgame_test(coop)
add_netgame_test(ctf)
add_netgame_test(entropy ../entropy/EntropyPackets.cpp ../entropy/EntropyRoom.cpp support/vector_helpers.cpp
  support/vector_helpers.h)
add_netgame_test(hoard ../hoard/hoard_ui.cpp)
add_netgame_test(hyperanarchy)
add_netgame_test(monsterball)
add_netgame_test(roboanarchy)
add_netgame_test(tanarchy)
