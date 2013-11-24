set(CMAKE_CXX_FLAGS "-std=c++0x -pthread")

add_executable(vmtest vmtest.cpp)
target_link_libraries(vmtest XzeroFlow)
