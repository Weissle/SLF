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



add_subdirectory(tests)

include_directories(${CMAKE_SOURCE_DIR}/include)
add_executable(si si.cpp include/Edge.hpp include/Graph.hpp 
				include/Node.hpp include/State.hpp include/VF2.hpp 
				include/Pair.hpp include/GraphReader.hpp include/AnswerReceiver.hpp
				include/argh.h include/common.h include/NodeSet.hpp include/IndexTurner.hpp include/MatchOrderSelector.hpp)

add_executable(sg sg.cpp include/SubgraphGenerator.hpp include/Graph.hpp
			include/Node.hpp include/Edge.hpp include/GraphReader.hpp include/argh.h include/RandomGenerator.hpp)

add_executable(veridator veridator.cpp include/AnswerChecker.hpp include/SolutionReader.hpp)

