cmake_minimum_required(VERSION 3.15)
project(NonEuclidean)

add_subdirectory(dependencies)

file(GLOB_RECURSE SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/NonEuclidean/*.cpp)
add_executable(NonEuclidean ${SOURCE})
target_include_directories(NonEuclidean PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/NonEuclidean)

find_package(glfw3 REQUIRED)
target_link_libraries(NonEuclidean glfw ${GLFW_LIBRARIES} glad)
include_directories(NonEuclidean ${GLFW_INCLUDE_DIRS})

target_link_libraries(NonEuclidean nlohmann_json::nlohmann_json stb_image OpenVR)
target_link_options(NonEuclidean PUBLIC -rdynamic)
