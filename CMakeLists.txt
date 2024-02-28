cmake_minimum_required (VERSION 3.8)

project (ruzhouxie)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_definitions(-w)
if(MSVC)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zc:preprocessor /permissive-")
endif()

include_directories("include")

add_executable(processer "test/processer.cpp")
add_executable(adaptors "test/basic_adaptors.cpp")
add_executable(tensor "test/tensor.cpp")

file(GLOB_RECURSE native_srcs RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "test/*.cpp")
foreach(srcfile IN LISTS native_srcs)
    get_filename_component(elfname ${srcfile} NAME_WE)
    #add_executable(${elfname} ${srcfile})
endforeach()