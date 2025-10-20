cmake_minimum_required(VERSION 3.11)
project(zmq_simple VERSION 1.0.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

message(STATUS "Building with embedded libzmq")   
set(LIBZMQ_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/libzmq")
set(BUILD_SHARED OFF CACHE BOOL "" FORCE)
set(BUILD_STATIC ON CACHE BOOL "" FORCE)
set(BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(WITH_PERF_TOOL OFF CACHE BOOL "" FORCE)
set(ENABLE_CPACK OFF CACHE BOOL "" FORCE)
set(WITH_DOCS OFF CACHE BOOL "" FORCE)
    
add_subdirectory(${LIBZMQ_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}/libzmq EXCLUDE_FROM_ALL)
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${LIBZMQ_SOURCE_DIR}/include
)
    
set(SOURCES src/zmq_simple.cpp)
# 静态库版本 - 用于 Docker 和独立部署
add_library(zmq_simple_static STATIC ${SOURCES})
add_dependencies(zmq_simple_static libzmq-static)
target_link_libraries(zmq_simple_static PUBLIC libzmq-static)
set_target_properties(zmq_simple_static PROPERTIES
    OUTPUT_NAME zmq_simple
    PUBLIC_HEADER include/zmq_simple.hpp
)

# 动态库版本 - 用于 Python 绑定
add_library(zmq_simple SHARED ${SOURCES})
add_dependencies(zmq_simple libzmq-static)
target_link_libraries(zmq_simple PRIVATE libzmq-static)
set_target_properties(zmq_simple PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION 1
    PUBLIC_HEADER include/zmq_simple.hpp
)

install(TARGETS zmq_simple zmq_simple_static
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    PUBLIC_HEADER DESTINATION include
)

option(BUILD_EXAMPLES "Build example programs" ON)
option(BUILD_PYTHON "Build Python bindings" ON)

if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

if(BUILD_PYTHON)
    add_subdirectory(python)
endif()

