cmake_minimum_required(VERSION 2.6)
project(GF2 CXX)

set(GF2_VERSION_MAJOR "0")
set(GF2_VERSION_MINOR "9")
set(GF2_VERSION_PATCH "2")
set(GF2_VERSION "${GF2_VERSION_MAJOR}.${GF2_VERSION_MINOR}.${GF2_VERSION_PATCH}")

option(BUILD_APPS "Build apps." OFF)
option(BUILD_TESTS "Build tests." ON)
option(BUILD_DOC "Build documentation (doxygen required)." OFF)
option(INSTALL_HEADERS "Install headers." ON)

if(CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE ${CMAKE_BUILD_TYPE}
      CACHE STRING "Choose the type of build: Release Debug Coverage ASan ASanDbg MemSan MemSanDbg Check"
      FORCE)
else()
  set(CMAKE_BUILD_TYPE Release
      CACHE STRING "Choose the type of build: Release Debug Coverage ASan ASanDbg MemSan MemSanDbg Check"
      FORCE)
endif()

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -Wwrite-strings -Wlogical-op -Wno-parentheses -Wno-unused-parameter -Wno-strict-aliasing")
  set(CMAKE_CXX_FLAGS_RELEASE     "-O2")
  set(CMAKE_CXX_FLAGS_DEBUG       "-O0 -g3")
  set(CMAKE_CXX_FLAGS_COVERAGE    "-O0 -g3 --coverage -fprofile-arcs -ftest-coverage")
  set(CMAKE_CXX_FLAGS_ASAN        "-fsanitize=address -fno-common -O3")
  set(CMAKE_CXX_FLAGS_ASANDBG     "-fsanitize=address -fno-common -O1 -g3 -fno-omit-frame-pointer -fno-optimize-sibling-calls ")
  set(CMAKE_CXX_FLAGS_CHECK       "-Werror -O1 -Wcast-qual")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -Wwrite-strings -Wpointer-arith -Wno-parentheses -Wno-unused-parameter -Wno-strict-aliasing")
  set(CMAKE_CXX_FLAGS_RELEASE     "-O2")
  set(CMAKE_CXX_FLAGS_DEBUG       "-O0 -g3")
  set(CMAKE_CXX_FLAGS_COVERAGE    "-O0 -g3 --coverage -fprofile-arcs -ftest-coverage")
  set(CMAKE_CXX_FLAGS_ASAN        "-fsanitize=address -fno-common -fsanitize=undefined -fno-sanitize-recover -O3")
  set(CMAKE_CXX_FLAGS_ASANDBG     "-fsanitize=address -fno-common -fsanitize=undefined -fno-sanitize-recover -O1 -g3 -fno-omit-frame-pointer -fno-optimize-sibling-calls")
  set(CMAKE_CXX_FLAGS_MEMSAN      "-fsanitize=memory -O3")
  set(CMAKE_CXX_FLAGS_MEMSANDBG   "-fsanitize=memory -O1 -g3 -fno-omit-frame-pointer -fno-optimize-sibling-calls -fsanitize-memory-track-origins=2")
  set(CMAKE_CXX_FLAGS_CHECK       "-Werror -O1")
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Coverage")
  if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set(CMAKE_SHARED_LINKER_FLAGS "--coverage")
  endif()
endif(CMAKE_BUILD_TYPE STREQUAL "Coverage")

# We have to use an additional linker option to support clock_gettime()
IF(CMAKE_SYSTEM_NAME STREQUAL Linux)
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lrt")
endif()

include_directories(include/)
add_subdirectory(include)

if(BUILD_APPS)
  add_subdirectory(apps)
endif()

if(BUILD_TESTS)
  enable_testing()
  add_subdirectory(test)
endif()

if(BUILD_DOC)
  add_subdirectory(doc)
endif()
