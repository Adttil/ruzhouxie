cmake_minimum_required (VERSION 3.8)

project (ruzhouxie)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_definitions(-w)
if(MSVC)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zc:preprocessor /permissive-")
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

include_directories("include")

file(GLOB test_srcs "test/*.cpp")
add_executable(all_tests ${test_srcs})
target_link_libraries(all_tests GTest::gtest_main)

file(GLOB_RECURSE example_srcs RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "example/*.cpp")
foreach(srcfile IN LISTS example_srcs)
    get_filename_component(elfname ${srcfile} NAME_WE)
    add_executable(${elfname} ${srcfile})
endforeach()