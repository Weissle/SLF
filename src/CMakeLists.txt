cmake_minimum_required (VERSION 3.2)

project (subgraph-isomorphism)

set_property(GLOBAL PROPERTY USE_FOLDERS On)

set (CMAKE_CXX_STANDARD 11)

add_executable(si si.cpp Edge.hpp Graph.hpp 
				Node.hpp State.hpp VF2.h VF2.cpp 
				Pair.hpp GraphReader.hpp
				argh.h)

add_executable(testi testi.cpp)
set_target_properties(testi PROPERTIES FOLDER "Test")