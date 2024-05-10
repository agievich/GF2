cmake_minimum_required(VERSION 3.1)
project(GF2 CXX)

set(GF2_VERSION_MAJOR "0")
set(GF2_VERSION_MINOR "9")
set(GF2_VERSION_PATCH "3")
set(GF2_VERSION "${GF2_VERSION_MAJOR}.${GF2_VERSION_MINOR}.${GF2_VERSION_PATCH}")

option(BUILD_APPS "Build apps." OFF)
option(BUILD_TESTS "Build tests." ON)
option(BUILD_DOC "Build documentation (doxygen required)." OFF)
option(INSTALL_HEADERS "Install headers." ON)

if(CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE ${CMAKE_BUILD_TYPE}
      CACHE STRING "Choose the type of build: \
        Release Debug Coverage ASan ASanDbg MemSan MemSanDbg Check"
      FORCE)
else()
  set(CMAKE_BUILD_TYPE Release
      CACHE STRING 
      "Choose the type of build: \
        Release Debug Coverage ASan ASanDbg MemSan MemSanDbg Check"
      FORCE)
endif()

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID}")

string(COMPARE EQUAL "GNU" "${CMAKE_CXX_COMPILER_ID}" CMAKE_COMPILER_IS_GNUCC)
string(REGEX MATCH "Clang" CMAKE_COMPILER_IS_CLANG "${CMAKE_CXX_COMPILER_ID}")
string(COMPARE EQUAL "MSVC" "${CMAKE_CXX_COMPILER_ID}" CMAKE_COMPILER_IS_MSVC)

# Lists of warnings and command-line flags:
# * https://gcc.gnu.org/onlinedocs/gcc/Warning-Options.html
# * https://clang.llvm.org/docs/ClangCommandLineReference.html
# * https://clang.llvm.org/docs/DiagnosticsReference.html

set(CMAKE_CXX_WARNINGS "-Wall -Wextra -Wno-parentheses \
  -Wno-bitwise-instead-of-logical -Wno-c++17-extensions \
  -Wno-deprecated-copy -Wno-deprecated-copy-with-user-provided-copy")

if(CMAKE_COMPILER_IS_GNUCC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_WARNINGS} -std=c++17")
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O2")
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -g3")
  set(CMAKE_CXX_FLAGS_COVERAGE "${CMAKE_CXX_FLAGS_COVERAGE} -O0 -g3 \
    --coverage")
  set(CMAKE_CXX_FLAGS_ASAN "${CMAKE_CXX_FLAGS_ASAN} -fno-common -O3 \
    -fsanitize=address")
  set(CMAKE_CXX_FLAGS_ASANDBG "${CMAKE_CXX_FLAGS_ASANDBG} -fno-common -O1 -g3 \
    -fsanitize=address -fno-omit-frame-pointer -fno-optimize-sibling-calls")
  set(CMAKE_CXX_FLAGS_CHECK "${CMAKE_CXX_FLAGS_CHECK} -Werror -O1 \
    -Wcast-qual -fanalyzer")
endif(CMAKE_COMPILER_IS_GNUCC)

if(CMAKE_COMPILER_IS_CLANG)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_WARNINGS} -std=c++17")
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -g3")
  set(CMAKE_CXX_FLAGS_COVERAGE "${CMAKE_CXX_FLAGS_COVERAGE} -O0 -g3 \
    --coverage")
  set(CMAKE_CXX_FLAGS_ASAN "${CMAKE_CXX_FLAGS_ASAN} -fno-common  -O3 \
    -fsanitize=address -fsanitize=undefined -fno-sanitize-recover \
    -fsanitize-undefined-trap-on-error")
  set(CMAKE_CXX_FLAGS_ASANDBG "${CMAKE_CXX_FLAGS_ASANDBG} -fno-common  -O1 -g3 \
    -fsanitize=address -fsanitize=undefined -fno-sanitize-recover \
    -fsanitize-undefined-trap-on-error -fno-omit-frame-pointer \
    -fno-optimize-sibling-calls")
  set(CMAKE_CXX_FLAGS_MEMSAN "${CMAKE_CXX_FLAGS_MEMSAN} -O3 -fsanitize=memory")
  set(CMAKE_CXX_FLAGS_MEMSANDBG "${CMAKE_CXX_FLAGS_MEMSANDBG} -O1 -g3 \ 
    -fsanitize=memory -fsanitize-memory-track-origins=2 \
    -fno-omit-frame-pointer -fno-optimize-sibling-calls")
  set(CMAKE_CXX_FLAGS_CHECK "${CMAKE_CXX_FLAGS_CHECK} -Werror -O1 -Wcast-qual")
endif(CMAKE_COMPILER_IS_CLANG)

if(CMAKE_COMPILER_IS_MSVC)
  # disable security warnings for sprintf()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /D _CRT_SECURE_NO_WARNINGS")
endif(CMAKE_COMPILER_IS_MSVC)

if(CMAKE_BUILD_TYPE STREQUAL "Coverage")
  if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_CLANG)
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} \
      -fprofile-arcs -ftest-coverage")
  endif()
endif()

if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_CLANG)
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lstdc++")
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
