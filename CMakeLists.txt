cmake_minimum_required(VERSION 3.9.6)

set(CMAKE_CXX_STANDARD 11)

find_package(MPI REQUIRED)
message(STATUS ${MPI_CXX_FOUND})

set(CMAKE_CXX_COMPILER ${MPI_CXX_COMPILER})
set(CMAKE_C_COMPILER ${MPI_CXX_COMPILER})

set(files genPoints.c partitionSpace.cpp parallelSearch.c processPool.c forkJoin.cpp)
set(targets genPoints partitionSpace parallelSearch processPool forkJoin)

list(LENGTH files len)
math(EXPR len "${len} - 1")
add_library(cll SHARED src/circular_linked_list.cpp)

add_library(utils SHARED src/utils.c)
add_compile_options(-Wall -O3)

foreach(val RANGE ${len})
    list(GET files ${val} file)
    list(GET targets ${val} target)
    message(STATUS "${file}  ${target}")
    add_executable(${target} src/${file})
    target_include_directories(${target} PRIVATE ${MPI_CXX_INCLUDE_DIRS})
    target_link_libraries(${target} PRIVATE ${MPI_CXX_LIBRARIES})
    target_link_libraries(${target} PRIVATE utils)
endforeach()

target_link_libraries(partitionSpace PRIVATE cll)



