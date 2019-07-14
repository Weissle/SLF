cmake_minimum_required (VERSION 3.2)

project (subgraph-isomorphism)

set_property(GLOBAL PROPERTY USE_FOLDERS On)

set (CMAKE_CXX_STANDARD 14)
if(CXX_COMPILER)
SET(CMAKE_CXX_COMPILER ${CXX_COMPILER})
endif()
if(CC_COMPILER)
SET(CMAKE_CXX_COMPILER ${CC_COMPILER})
endif()


if( ${CMAKE_BUILD_TYPE} EQUAL Debug)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -O0 -g")
#add_compile_options("  ")
elseif(${CMAKE_BUILD_TYPE} EQUAL Release)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -O3")
endif()

message(${CMAKE_BUILD_TYPE})


add_subdirectory(tests)

include_directories(${CMAKE_SOURCE_DIR}/include)
add_executable(si si.cpp include/Edge.hpp include/Graph.hpp 
				include/Node.hpp include/State.hpp include/VF2.hpp 
				include/Pair.hpp include/GraphReader.hpp include/AnswerReceiver.hpp
				include/argh.h include/common.h include/NodeSet.hpp include/IndexTurner.hpp include/MatchOrderSelector.hpp)

add_executable(sg sg.cpp include/SubgraphGenerator.hpp include/Graph.hpp
			include/Node.hpp include/Edge.hpp include/GraphReader.hpp include/argh.h)



