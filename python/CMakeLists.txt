# Python bindings for zmq_simple

find_package(Python3 COMPONENTS Interpreter Development REQUIRED)

find_package(pybind11 QUIET)
if(NOT pybind11_FOUND)
    message(STATUS "pybind11 not found, trying to use pip installed version")
    execute_process(
        COMMAND ${Python3_EXECUTABLE} -c "import pybind11; print(pybind11.get_cmake_dir())"
        OUTPUT_VARIABLE pybind11_DIR
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )
    if(pybind11_DIR)
        find_package(pybind11 REQUIRED PATHS ${pybind11_DIR})
    else()
        message(WARNING "pybind11 not found. Install with: pip3 install pybind11")
        return()
    endif()
endif()

pybind11_add_module(zmq_simple_py zmq_simple_py.cpp)
target_link_libraries(zmq_simple_py PRIVATE zmq_simple)
set_target_properties(zmq_simple_py PROPERTIES OUTPUT_NAME zmq_simple)

install(TARGETS zmq_simple_py
    LIBRARY DESTINATION ${Python3_SITEARCH}
)

