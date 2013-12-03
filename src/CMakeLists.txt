set(CMAKE_CXX_FLAGS "-std=c++0x -pthread")

add_executable(test test.cpp)
target_link_libraries(test XzeroFlow)
