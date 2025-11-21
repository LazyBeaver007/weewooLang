cmake_minimum_required(VERSION 3.20)
project(weewoo_compiler)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)


set(LLVM_DIR "D:/Projects/LLVM/clang+llvm-18.1.5-x86_64-pc-windows-msvc/lib/cmake/llvm")  
find_package(LLVM REQUIRED CONFIG)

message(STATUS "LLVM_DIR: ${LLVM_DIR}")
message(STATUS "LLVM found: ${LLVM_FOUND}")
message(STATUS "LLVM includes: ${LLVM_INCLUDE_DIRS}")

include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})


add_executable(weewoo_compiler main.cpp lexer.cpp parser.cpp codegen.cpp)

include(LLVM-Config)
link_directories(${LLVM_LIBRARY_DIRS})


find_package(ZLIB)
if (ZLIB_FOUND)
    message(STATUS "Found ZLIB: ${ZLIB_LIBRARY}")
else()
   
    find_library(ZLIB_LIBRARY 
        NAMES zlib zlib1 z
        PATHS "D:/Projects/LLVM/clang+llvm-18.1.5-x86_64-pc-windows-msvc/lib"
    )
    if (ZLIB_LIBRARY)
        message(STATUS "Found ZLIB manually: ${ZLIB_LIBRARY}")
        set(ZLIB_FOUND TRUE)
    else()
        message(WARNING "ZLIB not found, build may fail")
    endif()
endif()


llvm_map_components_to_libnames(WEEWOO_LIBS
    core
    executionengine
    mcjit       
    native
    support     
    )

message(STATUS "Linking libraries: ${WEEWOO_LIBS}")


if (ZLIB_FOUND)
    target_link_libraries(weewoo_compiler PRIVATE ${WEEWOO_LIBS} ${ZLIB_LIBRARY})
else()
    target_link_libraries(weewoo_compiler PRIVATE ${WEEWOO_LIBS})
endif()

# C++ flags
set_target_properties(weewoo_compiler PROPERTIES
    CXX_STANDARD ${CMAKE_CXX_STANDARD}
    CXX_STANDARD_REQUIRED ${CMAKE_CXX_STANDARD_REQUIRED}
    )

# needed on Windows
add_definitions(-D_SCL_SECURE_NO_WARNINGS)
add_definitions(-D_CRT_SECURE_NO_WARNINGS)

# LLVM requires RTTI
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /GR")