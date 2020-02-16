cmake_minimum_required (VERSION 3.2)

set (CMAKE_CXX_STANDARD 11)

add_executable(testi testi.cpp)

# 设置boost使用静态链接
set(Boost_USE_STATIC_LIBS ON)
# 设置需要的boost 组件
set(BOOST_COMPONENTS regex graph system )
# 使用cmake find_package 查找boost库位置
find_package(Boost COMPONENTS regex graph system)

set_target_properties(testi PROPERTIES FOLDER "Test")
#target_link_libraries(testi Boost::graph Boost::regex Boost::system)

include_directories(${CMAKE_SOURCE_DIR}/include)

if (BOOST_SOURCE)
	add_executable(boost-si boost-si-test.cpp)
	message(${BOOST_SOURCE})
	include_directories(${BOOST_SOURCE})
	set_target_properties(boost-si PROPERTIES FOLDER "Test")
endif ()