cmake_minimum_required(VERSION 3.9)
project(statpdfanalysis)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

add_executable(analyze main.c)

target_link_libraries(analyze ${CONAN_LIBS})
