cmake_minimum_required (VERSION 3.2)

set (CMAKE_CXX_STANDARD 11)

add_executable(testi testi.cpp)

set_target_properties(testi PROPERTIES FOLDER "Test")
include_directories(${CMAKE_SOURCE_DIR})
if (BOOST_SOURCE)
	add_executable(boost-si boost-si-test.cpp)
	message(${BOOST_SOURCE})
	include_directories(${BOOST_SOURCE})
	set_target_properties(boost-si PROPERTIES FOLDER "Test")
endif ()