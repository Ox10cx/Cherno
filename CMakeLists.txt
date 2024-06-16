cmake_minimum_required(VERSION 3.5)

project(OpenGL)


set(CMAKE_CXX_STANDARD 17)

add_definitions(-DGLEW_STATIC)


file(GLOB res_list res)
# 将资源拷贝到CMAKE工程编译目录
file(COPY ${res_list} DESTINATION ${CMAKE_BINARY_DIR})

# file(GLOB head_list *.h)
# file(GLOB src_list *.cpp)

file(GLOB common_head_list 
   GLHead.h)
file(GLOB common_src_list 
    GLHead.cpp)

file(GLOB stb_image_head_list vendor/stb_image/*.h)
file(GLOB stb_image_src_list vendor/stb_image/*.cpp)


set(GLFW_PATH dependency/GLFW)
link_directories(${GLFW_PATH}/lib-x86_64)

set(GLEW_PATH dependency/GLEW)
link_directories(${GLEW_PATH}/lib)


include_directories(
    ${GLFW_PATH}/include
    ${GLEW_PATH}/include
    ${common_head_list}
    ${stb_image_head_list}
)

find_library(OpenGL_LIBRARY OpenGL )
find_library(Cocoa_LIBRARY Cocoa )
find_library(IOKit_LIBRARY IOKit )

#cmake多编译目标
add_executable(OpenGL
    ${stb_image_src_list}
    ${common_src_list}
    main.cpp
    )
  
target_link_libraries(OpenGL 
    glfw3
    GLEW
    ${OpenGL_LIBRARY}
    ${Cocoa_LIBRARY}
    ${IOKit_LIBRARY})


add_executable(Demo
    ${stb_image_src_list}
    ${common_src_list}
    demo.cpp
    )    

target_link_libraries(Demo 
    glfw3
    GLEW
    ${OpenGL_LIBRARY}
    ${Cocoa_LIBRARY}
    ${IOKit_LIBRARY})



  #set_target_properties(OpenGL PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}")
