# Deletes archives from a vcpkg "files" binary cache that no build directory uses.
#
#   cmake -DCACHE_DIR=<cache dir> -DBUILD_ROOT=<dir holding the build dirs> -P prune-vcpkg-cache.cmake
#
# vcpkg names each archive after the package's ABI hash, which is the SHA-256 of the
# vcpkg_abi_info.txt it installs with the package. Archives whose hash does not match an
# installed package were built for an older compiler, port version or triplet.

# Script mode starts with old policies, under which if(IN_LIST) is an error.
cmake_minimum_required(VERSION 3.20)

foreach(var CACHE_DIR BUILD_ROOT)
  if(NOT DEFINED ${var})
    message(FATAL_ERROR "${var} is not set")
  endif()
endforeach()

file(GLOB abi_files "${BUILD_ROOT}/*/vcpkg_installed/*/share/*/vcpkg_abi_info.txt")
if(NOT abi_files)
  # Pruning against an empty set would delete the whole cache.
  message(FATAL_ERROR "No vcpkg_abi_info.txt found under ${BUILD_ROOT}/*/vcpkg_installed")
endif()

set(used "")
foreach(abi_file IN LISTS abi_files)
  file(SHA256 "${abi_file}" hash)
  list(APPEND used "${hash}")
endforeach()

file(GLOB_RECURSE archives "${CACHE_DIR}/*.zip")
set(kept 0)
set(deleted 0)
foreach(archive IN LISTS archives)
  get_filename_component(hash "${archive}" NAME_WE)
  if(hash IN_LIST used)
    math(EXPR kept "${kept} + 1")
  else()
    file(REMOVE "${archive}")
    math(EXPR deleted "${deleted} + 1")
  endif()
endforeach()

message(STATUS "vcpkg binary cache: kept ${kept} archives, deleted ${deleted}")
