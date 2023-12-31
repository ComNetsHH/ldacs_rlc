cmake_minimum_required(VERSION 3.1)
project(tuhh_intairnet_rlc)

set(CMAKE_CXX_STANDARD 14)

add_subdirectory(glue-lib-headers)
include_directories(glue-lib-headers)

set(TARGET_SRC Rlc.hpp Rlc.cpp RlcProcess.hpp RlcProcess.cpp)
set(TARGET_TEST_SRC tests/unittests.cpp tests/RlcProcessTest.cpp)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
#set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/cmake-build-debug)
add_library(tuhh_intairnet_rlc SHARED ${TARGET_SRC} ${GLUE_SRC_HPP})
target_link_libraries(tuhh_intairnet_rlc LINK_PUBLIC intairnet_linklayer_glue)

add_executable(unittests ${TARGET_SRC} ${TARGET_TEST_SRC} ${GLUE_SRC_HPP})

find_library(CPPUNITLIB cppunit)
target_include_directories(unittests PUBLIC /opt/homebrew/opt/cppunit/include)
target_link_libraries(unittests PRIVATE ${CPPUNITLIB} tuhh_intairnet_rlc)
target_include_directories(tuhh_intairnet_rlc PUBLIC glue-lib-headers)

