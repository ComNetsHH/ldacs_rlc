cmake_minimum_required(VERSION 3.13)
project(tuhh_intairnet_rlc)

set(CMAKE_CXX_STANDARD 14)
add_subdirectory(glue-lib-headers)
include_directories(glue-lib-headers)

# Remove if not needed
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -I/usr/include -I/usr/local/include/ -L/usr/local/Cellar/cppunit/1.15.1/lib")

set(TARGET_SRC Rlc.hpp Rlc.cpp RlcProcess.hpp RlcProcess.cpp InetPacketPayload.cpp InetPacketPayload.hpp)
set(TARGET_TEST_SRC tests/unittests.cpp tests/RlcProcessTest.cpp)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
add_library(tuhh_intairnet_rlc SHARED ${TARGET_SRC} ${GLUE_SRC_HPP})
target_link_libraries(tuhh_intairnet_rlc LINK_PUBLIC intairnet_linklayer_glue)

add_executable(unittests ${TARGET_SRC} ${TARGET_TEST_SRC} ${GLUE_SRC_HPP})
target_link_libraries(unittests cppunit tuhh_intairnet_rlc)