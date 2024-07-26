cmake_minimum_required(VERSION 3.10)

project(InfoTecTest)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(PROGRAM1_SRC src/program1.cpp)
set(PROGRAM2_SRC src/programm2.cpp)

add_executable(program1 ${PROGRAM1_SRC})
target_link_libraries(program1 pthread)

# Создание второго исполняемого файла
add_executable(program2 ${PROGRAM2_SRC})
target_link_libraries(program2 pthread)
