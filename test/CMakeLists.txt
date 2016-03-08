cmake_minimum_required(VERSION 3.4)
project(nucleo-cc1120-test)

set(CMAKE_CXX_FLAGS "-pthread")
set(CMAKE_EXE_LINKER_FLAGS "-pthread")

enable_testing()
find_package(GTest REQUIRED)

# add more include directories
include_directories("../")
include_directories("./mock")

set(src "../CC1120.cpp")
file(GLOB mock_src "mock/*.cpp" "mock/*.c")
file(GLOB tests "tests/*.cpp")

add_executable(all-tests ${src} ${mock_src} ${tests})
target_link_libraries(all-tests gtest gtest_main gmock)
