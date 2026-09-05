cmake_minimum_required(VERSION 3.23)

set(VCPKG_TARGET_TRIPLET x64-windows-static)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE FILEPATH "")

project(
	PlayerRotationGPSupport
	VERSION 1.2.0
	DESCRIPTION "SKSE plugin adding RaceMenu player rotation with gamepad support."
	LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)

set(headers src/pch.hpp
			src/config.hpp
			src/event.hpp
			src/hooks.hpp)
set(sources src/event.cpp
			src/plugin.cpp
			src/hooks.cpp)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${headers} ${sources})

find_package(toml11 CONFIG REQUIRED)
set(COMMONLIB_PREBUILT OFF CACHE BOOL "Build CommonLibSSE from source with this project's static runtime" FORCE)
set(BUILD_TESTS OFF CACHE BOOL "Do not build CommonLibSSE's own test suite" FORCE)
add_subdirectory(lib/commonlibsse-ng EXCLUDE_FROM_ALL)

add_library(${PROJECT_NAME} SHARED ${headers} ${sources})
add_library("${PROJECT_NAME}::${PROJECT_NAME}" ALIAS "${PROJECT_NAME}")

target_link_libraries(${PROJECT_NAME} PUBLIC CommonLibSSE::CommonLibSSE)
target_link_libraries(${PROJECT_NAME} PUBLIC toml11::toml11)

if (MSVC)
	add_compile_definitions(_UNICODE NOMINMAX)
	target_compile_options("${PROJECT_NAME}" PRIVATE /EHsc /W4 /WX)
endif ()

target_include_directories(${PROJECT_NAME} PRIVATE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/src>
        $<INSTALL_INTERFACE:src>)
target_include_directories(${PROJECT_NAME} PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)
target_precompile_headers(${PROJECT_NAME} PRIVATE src/pch.hpp)
