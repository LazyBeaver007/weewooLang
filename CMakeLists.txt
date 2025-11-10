cmake_minimum_required(VERSION 3.20)
project(weewoo_compiler)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(LLVM_DIR "path")
find_package(LLVM REQUIRED CONFIG)
message(STATUS "LLVM_DIR: ${LLVM_DIR}")
message(STATUS "LLVM found: ${LLVM_FOUND}")
message(STATUS "LLVM includes: ${LLVM_INCLUDE_DIRS}")
include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})
add_executable(weewoo_compiler main.cpp lexer.cpp parser.cpp codegen.cpp)
include(LLVM-Config)
link_directories(${LLVM_LIBRARY_DIRS})

llvm_map_components_to_libnames(WEEWOO_LIBS
    core
    executionengine
    mcjit       
    native
    )

message(STATUS "Linking libraries: ${WEEWOO_LIBS}")
target_link_libraries(weewoo_compiler PRIVATE ${WEEWOO_LIBS})

#C++ flags
set_target_properties(weewoo_compiler PROPERTIES
    CXX_STANDARD ${CMAKE_CXX_STANDARD}
    CXX_STANDARD_REQUIRED ${CMAKE_CXX_STANDARD_REQUIRED}
    )

#needed on Windows
add_definitions(-D_SCL_SECURE_NO_WARNINGS)
add_definitions(-D_CRT_SECURE_NO_WARNINGS)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /GR")