set(CMAKE_FOLDER "tests")

# polymodel.cpp is compiled directly into the test (rather than linking the `model` library) so
# that newstyle.cpp, which needs the renderer, stays out; test_stubs.cpp supplies the few game-side
# symbols polymodel.cpp references.
add_executable(model_tests
  polymodel_tests.cpp
  support/polymodel_harness.cpp
  support/test_stubs.cpp
  ../polymodel.cpp
)
target_include_directories(model_tests PRIVATE ${PROJECT_SOURCE_DIR}/model)
target_link_libraries(model_tests PRIVATE
  GTest::gtest_main
  cfile
  ddebug
  fix
  logger
  mem
  misc
  vecmat
)

gtest_discover_tests(model_tests)
