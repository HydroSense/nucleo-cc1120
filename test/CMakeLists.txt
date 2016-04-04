cmake_minimum_required(VERSION 3.0)
project(nucleo-cc1120-test)

set(CMAKE_CXX_FLAGS "-pthread")
set(CMAKE_EXE_LINKER_FLAGS "-pthread")

find_package(GTest REQUIRED)

# add more include directories
include_directories("../")
include_directories("./mock")

set(src "../CC1120.cpp")
file(GLOB mock_src "mock/*.cpp" "mock/*.c")
file(GLOB tests "tests/*.cpp")

foreach(test ${tests})
  get_filename_component(test_name ${test} NAME_WE)
  add_executable(${test_name} ${test} ${mock_src} ${src})
  target_link_libraries(${test_name} ${GTEST_BOTH_LIBRARIES} gmock)
endforeach(test)
