cmake_minimum_required (VERSION 3.8)

project (ruzhouxie)

add_library(RUZHOUXIE INTERFACE)
target_include_directories(RUZHOUXIE INTERFACE include)

option(RUZHOUXIE_INCLUDE_TESTS "Include tests" ON)
if(NOT RUZHOUXIE_INCLUDE_TESTS)
        return()
endif()

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(MAGIC_CPP_INCLUDE_TESTS OFF)

add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -w")
if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zc:preprocessor /permissive- /w-")
endif()

include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

FetchContent_Declare(
    magic_cpp
    URL https://github.com/Adttil/magic-cpp/archive/refs/heads/opt-test.zip
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(magic_cpp)

include_directories("include")

set(CMAKE_EXE_LINKER_FLAGS "-static")

file(GLOB test_srcs "test/*.cpp")
add_executable(ruzhouxie_tests ${test_srcs})
target_link_libraries(ruzhouxie_tests PRIVATE GTest::gtest_main MAGIC_CPP)