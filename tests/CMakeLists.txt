
function (add_test_f test_name test_cpp)
	add_executable(${test_name} ${test_cpp})
	add_test(NAME ${test_name} 
			COMMAND ${test_name})
	target_link_libraries(${test_name} ${Boost_LIBRARIES} )
endfunction()

add_test_f(graph_unittest ${CMAKE_CURRENT_SOURCE_DIR}/graph/graph_unittest.cpp)
add_test_f(config_unittest ${CMAKE_CURRENT_SOURCE_DIR}/config/config_unittest.cpp)

set (tools_unittest_cpp 
	${CMAKE_CURRENT_SOURCE_DIR}/tools/tools_unittest.cpp
	${CMAKE_SOURCE_DIR}/src/tools/timer.cpp)
add_test_f(tools_unittest "${tools_unittest_cpp}")
set (si_unittest_cpp
	${CMAKE_CURRENT_SOURCE_DIR}/si/si_unittest.cpp
	${CMAKE_SOURCE_DIR}/src/tools/timer.cpp
	${CMAKE_SOURCE_DIR}/src/si/state.cpp
	${CMAKE_SOURCE_DIR}/src/si/match_sequence_selector.cpp
	${CMAKE_SOURCE_DIR}/src/si/subgraph_isomorphism_base.cpp
	${CMAKE_SOURCE_DIR}/src/si/parallel_subgraph_isomorphism.cpp
	${CMAKE_SOURCE_DIR}/src/si/task_manager.cpp
	${CMAKE_SOURCE_DIR}/src/si/tasks.cpp
	${CMAKE_SOURCE_DIR}/src/si/sequential_subgraph_isomorphism.cpp)
add_test_f(si_unittest "${si_unittest_cpp}")
