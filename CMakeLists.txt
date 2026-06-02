cmake_minimum_required(VERSION 3.14)
project(SAT3D)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

FetchContent_Declare(glfw GIT_REPOSITORY https://github.com/glfw/glfw.git GIT_TAG 3.3.8)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(glfw)

FetchContent_Declare(glm GIT_REPOSITORY https://github.com/g-truc/glm.git GIT_TAG 1.0.1)
FetchContent_MakeAvailable(glm)

find_package(OpenGL REQUIRED)

add_executable(SAT3D
    src/main.cpp
    src/rendering/Shader.cpp
    src/rendering/Camera.cpp
    src/rendering/Mesh.cpp
    src/rendering/LineBatch.cpp
    src/geometry/ConvexShape.cpp
    src/physics/SAT.cpp
    src/glad/src/glad.c
)

target_include_directories(SAT3D PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/glad/include
    ${OPENGL_INCLUDE_DIRS}
)

target_compile_definitions(SAT3D PRIVATE USE_GLAD)

target_link_libraries(SAT3D PRIVATE ${OPENGL_LIBRARIES} glfw glm::glm)

if(CMAKE_SYSTEM_NAME MATCHES "Darwin")
    target_link_libraries(SAT3D PRIVATE
        "-framework OpenGL" "-framework Cocoa" "-framework IOKit")
endif()

file(COPY ${CMAKE_SOURCE_DIR}/shaders DESTINATION ${CMAKE_BINARY_DIR})

# Headless numerical tests for the SAT core (no OpenGL).  Run with:
#   cmake --build build --target sat_tests && ./build/sat_tests
add_executable(sat_tests
    src/test_sat.cpp
    src/geometry/ConvexShape.cpp
    src/physics/SAT.cpp
)
target_include_directories(sat_tests PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(sat_tests PRIVATE glm::glm)
