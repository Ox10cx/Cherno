cmake_minimum_required(VERSION 3.5)

project(OpenGL)




set(CMAKE_CXX_STANDARD 17)

add_definitions(-DGLEW_STATIC)


file(GLOB shader_list res)
# 将资源拷贝到CMAKE工程编译目录
file(COPY ${shader_list} DESTINATION ${CMAKE_BINARY_DIR})

file(GLOB head_list *.h)
file(GLOB src_list *.cpp)


set(GLFW_PATH dependency/GLFW)
link_directories(${GLFW_PATH}/lib-x86_64)

set(GLEW_PATH dependency/GLEW)
link_directories(${GLEW_PATH}/lib)


include_directories(
    ${GLFW_PATH}/include
    ${GLEW_PATH}/include
)

find_library(OpenGL_LIBRARY OpenGL )
find_library(Cocoa_LIBRARY Cocoa )
find_library(IOKit_LIBRARY IOKit )

add_executable(OpenGL
    ${src_list}
    )

target_link_libraries(OpenGL 
    glfw3
    GLEW
    ${OpenGL_LIBRARY}
    ${Cocoa_LIBRARY}
    ${IOKit_LIBRARY})

  #set_target_properties(OpenGL PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}")
