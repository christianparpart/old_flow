add_definitions(-Wall -Wno-deprecated -DPACKAGE_VERSION="${PACKAGE_VERSION}")
set(CMAKE_CXX_FLAGS "-std=c++0x -pthread")

add_library(XzeroFlow SHARED
  vm/Instruction.cpp
  vm/Handler.cpp
  vm/Program.cpp
  vm/Runner.cpp
  vm/Runtime.cpp
  vm/Signature.cpp
)

target_link_libraries(XzeroFlow pthread)
set_target_properties(XzeroFlow PROPERTIES VERSION ${PACKAGE_VERSION})
install(TARGETS XzeroFlow DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
