cmake_minimum_required(VERSION 3.16)
project(mystl LANGUAGES CXX)

# C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(CMAKE_BUILD_TYPE STREQUAL Debug)
    add_compile_options(-g)
elseif(CMAKE_BUILD_TYPE STREQUAL Release)
    add_compile_options(-o2)
endif()
message(STATUS "CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")

# 头文件搜索路径
include_directories(
    ${PROJECT_SOURCE_DIR}/include/reactor/
    ${PROJECT_SOURCE_DIR}/include/tcp/
    ${PROJECT_SOURCE_DIR}/include/thread/
    ${PROJECT_SOURCE_DIR}/include/util/
)

file(GLOB TEST_FILES test/*.cpp)

# 生成测试可执行程序
add_executable(
    myreactor_test ${TEST_FILES}
)