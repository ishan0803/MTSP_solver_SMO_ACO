cmake_minimum_required(VERSION 3.5...3.18)
project(TSP)

find_package (Python3 COMPONENTS Interpreter Development REQUIRED) 
execute_process(
    COMMAND ${Python3_EXECUTABLE} -m pybind11 --cmakedir
    OUTPUT_VARIABLE PYBIND11_CMAKE_DIR
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
set(pybind11_DIR ${PYBIND11_CMAKE_DIR})

message(STATUS "Using pybind11 found at: ${pybind11_DIR}")
find_package(pybind11 REQUIRED)

# Create Python module

include_directories(cpp/include)

pybind11_add_module(MTSP_SOLVER SHARED cpp/src/pybinder.cpp cpp/src/graph.cpp cpp/src/aco.cpp cpp/src/smo.cpp cpp/src/hybrid.cpp)

set_target_properties(MTSP_SOLVER PROPERTIES CXX_STANDARD 11 CXX_STANDARD_REQUIRED ON)

# Needed for importing the "hybrid_module" module from the build directory
set(INIT_PY "${CMAKE_CURRENT_BINARY_DIR}/__init__.py")
if(NOT EXISTS ${INIT_PY})
    file(WRITE ${INIT_PY} "")
endif()

add_custom_command(TARGET MTSP_SOLVER POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
            $<TARGET_FILE_DIR:MTSP_SOLVER>/$<TARGET_FILE_NAME:MTSP_SOLVER>
            ${CMAKE_CURRENT_BINARY_DIR}/$<TARGET_FILE_NAME:MTSP_SOLVER>
)