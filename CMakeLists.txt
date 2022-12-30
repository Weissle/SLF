cmake_minimum_required (VERSION 3.2)

project (subgraph-isomorphism)
set(CMAKE_EXPORT_COMPILE_COMMANDS true)
set(CMAKE_CXX_STANDARD 17)
set_property(GLOBAL PROPERTY USE_FOLDERS On)
add_compile_options(-Wall -Wextra -pedantic -Werror)

IF(UNIX)
	link_libraries(pthread)
ENDIF()

# BOOST 
SET(Boost_USE_STATIC_LIBS ON) 
find_package(Boost REQUIRED COMPONENTS program_options log log_setup)
include_directories(${Boost_INCLUDE_DIRS})

include_directories(${CMAKE_SOURCE_DIR}/include)
enable_testing()
add_subdirectory(tests)
add_subdirectory(perf)

add_executable(slf 
	./src/slf.cpp 
	./src/app.cpp
	./src/tools/timer.cpp
	./src/si/match_sequence_selector.cpp
	./src/si/subgraph_isomorphism_base.cpp
	./src/si/sequential_subgraph_isomorphism.cpp
	./src/si/parallel_subgraph_isomorphism.cpp
	./src/si/task_manager.cpp
	./src/si/state.cpp
	./src/si/tasks.cpp
)
target_link_libraries(slf ${Boost_LIBRARIES} )
