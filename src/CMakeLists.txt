cmake_minimum_required (VERSION 3.2)

project (subgraph-isomorphism)

set_property(GLOBAL PROPERTY USE_FOLDERS On)

set (CMAKE_CXX_STANDARD 11)
if(CXX_COMPILER)
SET(CMAKE_CXX_COMPILER ${CXX_COMPILER})
endif()
if(CC_COMPILER)
SET(CMAKE_CXX_COMPILER ${CC_COMPILER})
endif()
include_directories(${CMAKE_SOURCE_DIR}/include)
add_executable(si si.cpp include/Edge.hpp include/Graph.hpp 
				include/Node.hpp include/State.hpp include/VF2.hpp 
				include/Pair.hpp include/GraphReader.hpp include/AnswerReceiver.hpp
				include/argh.h)

add_subdirectory(tests)
