cmake_minimum_required(VERSION 3.16)
project(docbot)

find_package(Curses REQUIRED)
find_package(CURL REQUIRED)
find_package(cJSON REQUIRED)

set(CMAKE_CXX_STANDARD 17)
set(LLVM_VERSION 15)
set(LLVM_LINK_COMPONENTS Support)
list(APPEND CMAKE_MODULE_PATH /usr/lib/llvm-${LLVM_VERSION}/cmake)
list(APPEND CMAKE_MODULE_PATH /usr/lib/llvm-${LLVM_VERSION}/lib/cmake/clang)

#include(AddLLVM)
#include(AddClang)

add_executable(docbot 
    main.cc
    docbot.cc
    openai.cc)

target_include_directories(docbot PRIVATE /usr/lib/llvm-${LLVM_VERSION}/include)
target_include_directories(docbot PRIVATE ${CURL_INCLUDE_DIRS})
target_include_directories(docbot PRIVATE ${CJSON_INCLUDE_DIRS})
target_link_directories(docbot PRIVATE /usr/lib/llvm-${LLVM_VERSION}/lib)
target_link_libraries(docbot PRIVATE ${CURSES_LIBRARIES})
target_link_libraries(docbot PRIVATE clangAST)
target_link_libraries(docbot PRIVATE clangBasic)
target_link_libraries(docbot PRIVATE clangTooling)
target_link_libraries(docbot PRIVATE clangDriver)
target_link_libraries(docbot PRIVATE clangFrontend)
target_link_libraries(docbot PRIVATE clangParse)
target_link_libraries(docbot PRIVATE clangSema)
target_link_libraries(docbot PRIVATE clangEdit)
target_link_libraries(docbot PRIVATE clangAnalysis)
target_link_libraries(docbot PRIVATE clangLex)
target_link_libraries(docbot PRIVATE clangSerialization)
target_link_libraries(docbot PRIVATE clangAST)
target_link_libraries(docbot PRIVATE clangBasic)
target_link_libraries(docbot PRIVATE clangSupport)
target_link_libraries(docbot PRIVATE LLVM-${LLVM_VERSION})
target_link_libraries(docbot PRIVATE ${CURL_LIBRARIES})
target_link_libraries(docbot PRIVATE ${CJSON_LIBRARIES})