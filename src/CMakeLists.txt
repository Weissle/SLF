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

message("CMAKE_CXX_FLAGS_DEBUG is ${CMAKE_CXX_FLAGS_DEBUG}")
message("CMAKE_CXX_FLAGS_RELEASE is ${CMAKE_CXX_FLAGS_RELEASE}")

add_subdirectory(tests)

include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/tools)
add_executable(si si.cpp include/Edge.hpp include/Graph.hpp 
				include/Node.hpp include/State.hpp include/VF2.hpp 
				include/Pair.hpp include/GraphReader.hpp include/AnswerReceiver.hpp
				include/argh.h include/common.h)

add_executable(sg sg.cpp include/SubgraphGenerator.hpp include/Graph.hpp
			include/Node.hpp include/Edge.hpp include/GraphReader.hpp include/argh.h)



