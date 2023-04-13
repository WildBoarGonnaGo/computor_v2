cmake_minimum_required(VERSION 3.17)
project(computor_v2)

set(CMAKE_CXX_STANDARD 20)

add_executable(computor_v2 RevPolNotation.cpp EquationProc.cpp rev_pol_notation.cpp
        computor.cpp Matrix.cpp MatrixCalc.cpp)
