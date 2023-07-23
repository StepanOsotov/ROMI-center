project(Message)
cmake_minimum_required(VERSION 3.0 FATAL_ERROR)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_SOURCE_DIR}/../)
include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}/inc
	${CMAKE_CURRENT_SOURCE_DIR}/src
)
file(GLOB CPPS
	"${CMAKE_CURRENT_SOURCE_DIR}/inc/*.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)
add_definitions(-c -g -Wall -O0)
add_executable(${PROJECT_NAME} ${CPPS})
target_link_libraries(${PROJECT_NAME})
install(TARGETS ${PROJECT_NAME} DESTINATION bin)