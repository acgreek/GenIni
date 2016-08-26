cmake_minimum_required (VERSION 2.8.4)
project (GenIni)

cmake_policy(SET CMP0003 NEW)

set (VERSION_MAJOR 0)
set (VERSION_MINOR 9.2)
set (CMAKE_CXX_FLAGS "-Wall -ggdb3 -Wextra -std=c++11")


add_executable(genini genini.cc)
target_link_libraries(genini boost_system boost_filesystem)

include(CTest)

#add_test (output1 ./dotest test_input1.txt test_output1) 
#add_test (output2 ./dotest test_input2.txt test_output1)
