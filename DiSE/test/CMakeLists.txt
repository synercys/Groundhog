#project(fronend) 

#############################################
#               Build fronend               #
#############################################

file(GLOB_RECURSE SRC_TEST ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
include_directories(${CMAKE_SOURCE_DIR}/test/)

add_executable(test ${SRC_TEST})
#target_include_directories(test ${CMAKE_CURRENT_SOURCE_DIR})

#############################################
#            Link our libraries             #
#############################################
target_link_libraries(test dEnc_tests dEnc)
