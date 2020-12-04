cmake_minimum_required(VERSION 3.9.6)

set(CMAKE_CXX_STANDARD 11)

find_package(MPI REQUIRED)
message(STATUS ${MPI_CXX_FOUND})

set(CMAKE_CXX_COMPILER ${MPI_CXX_COMPILER})
set(CMAKE_C_COMPILER ${MPI_CXX_COMPILER})

#[[ add executable ]]
add_executable(partitionSpace src/partitionSpace.cpp)
target_include_directories(partitionSpace PRIVATE ${MPI_CXX_INCLUDE_DIRS})
target_link_libraries(partitionSpace PRIVATE ${MPI_CXX_LIBRARIES})
#[[ added executable ]]

#[[ link utils with executable ]]
add_library(utils SHARED src/utils.c)
#[[ link with utils ]]

#[[ link cll with executable ]]
add_library(cll SHARED src/circular_linked_list.cpp)
#[[ link with cll ]]

target_link_libraries(partitionSpace PRIVATE utils)
target_link_libraries(partitionSpace PRIVATE cll)
