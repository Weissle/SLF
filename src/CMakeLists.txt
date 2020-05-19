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

IF(UNIX)
	link_libraries(pthread)
	message(UNIX)
	SET(CMAKE_CXX_FLAGS "-std=c++11 -lpthread ${CMAKE_CXX_FLAGS}")
	message(${CMAKE_CXX_FLAGS})
ENDIF()
IF(WIN32)
	message(WIN32)
ENDIF()

#add_compile_options("-lpthread -std=c++11 -pthread")
add_compile_options( -std=c++11 -lpthread )
add_subdirectory(tests)

include_directories(${CMAKE_SOURCE_DIR}/include)
add_executable(si si.cpp include/si/si_marcos.h)
add_executable(sg sg.cpp include/tools/SubgraphGenerator.hpp )

add_executable(veridator veridator.cpp include/tools/AnswerChecker.hpp include/tools/SolutionReader.hpp)

message("Hello" + ${CMAKE_CXX_FLAGS})

