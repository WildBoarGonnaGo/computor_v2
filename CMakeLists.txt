cmake_minimum_required(VERSION 3.17)
project(computor_v2)

set(CMAKE_CXX_STANDARD 17)
add_compile_options(-g)

add_executable(computor_v2 RevPolNotation.cpp RevPolNotation.h EquationProc.cpp EquationProc.h rev_pol_notation.cpp
        computor.cpp computor.h Matrix.cpp Matrix.h MatrixCalc.cpp MatrixCalc.h gnuplot-iostream.h)
target_include_directories(${PROJECT_NAME} PUBLIC /opt/homebrew/include)
target_link_directories(${PROJECT_NAME} PUBLIC /opt/homebrew/Cellar/boost/1.81.0_1/lib)
# target_include_directories(${PROJECT_NAME} PUBLIC /Users/lchantel/.brew/include)
# target_link_directories(${PROJECT_NAME} PUBLIC /Users/lchantel/.brew/Cellar/boost/1.81.0_1/lib/)
target_link_libraries(${PROJECT_NAME} PUBLIC -lboost_iostreams -lboost_system -lboost_filesystem)
