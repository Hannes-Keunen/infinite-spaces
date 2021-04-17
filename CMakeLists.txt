cmake_minimum_required(VERSION 3.15)
project(NonEuclidean)

add_subdirectory(dependencies)

file(GLOB_RECURSE SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/NonEuclidean/*.cpp)
add_executable(NonEuclidean ${SOURCE})
target_include_directories(NonEuclidean PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/NonEuclidean)

target_link_libraries(NonEuclidean glfw glad nlohmann_json::nlohmann_json stb_image OpenVR)
target_include_directories(NonEuclidean PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/openvr/headers)

get_target_property(PROP OpenVR IMPORTED_LOCATION)
message(${PROP})

if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++latest")
else()
    target_link_options(NonEuclidean PUBLIC -rdynamic)
    target_compile_features(NonEuclidean PRIVATE cxx_std_17)
endif()
