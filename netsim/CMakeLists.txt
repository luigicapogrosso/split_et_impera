cmake_minimum_required(VERSION 3.10.2)
project(netsim)

SET(CMAKE_CXX_STANDARD "17")
SET(CMAKE_CXX_EXTENSIONS "OFF")
set(CMAKE_CXX_FLAGS "-g ")

INCLUDE_DIRECTORIES(/home/lcapogrosso/Libraries/scnsl/build/include)
LINK_DIRECTORIES(/home/lcapogrosso/Libraries/scnsl/build/lib)

include_directories(include)
file(GLOB SOURCES "src/*.*")
add_executable(netsim ${SOURCES})
target_link_libraries(netsim scnsl systemc dl)
