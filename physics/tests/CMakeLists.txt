set(CMAKE_FOLDER "tests")

# findintersection.cpp and physics.cpp are compiled directly into the test (rather than
# linking the physics library) so that only their own game-side references need stand-ins
# (see physics_link_stubs.cpp); collide.cpp and newstyle_fi.cpp are left out.
add_executable(
  physics_tests
  fvi_geometry_tests.cpp
  physics_sim_tests.cpp
  support/fvi_checks.cpp
  support/physics_link_stubs.cpp
  support/sim_object.cpp
  support/vector_helpers.cpp
  ${PROJECT_SOURCE_DIR}/physics/findintersection.cpp
  ${PROJECT_SOURCE_DIR}/physics/physics.cpp
)
target_include_directories(physics_tests PRIVATE
  ${PROJECT_SOURCE_DIR}/physics
  ${PROJECT_SOURCE_DIR}/model
  ${PROJECT_SOURCE_DIR}/rtperformance
)
target_link_libraries(
  physics_tests
  GTest::gtest_main
  ddebug
  fix
  logger
  mem
  misc
  vecmat
)
gtest_discover_tests(physics_tests)
