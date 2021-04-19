add_subdirectory(glad)
add_subdirectory(glfw)

add_library(stb_image STATIC ${CMAKE_CURRENT_SOURCE_DIR}/stb/stb_image.c)
target_include_directories(stb_image PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/stb)

# add_library(OpenVR SHARED IMPORTED)
add_library(OpenVR INTERFACE)
target_include_directories(OpenVR INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/openvr/headers)
if (UNIX AND NOT APPLE)
    target_link_libraries(OpenVR INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/openvr/bin/linux64/libopenvr_api.so")
elseif(WIN32)
    target_link_libraries(OpenVR INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/openvr/lib/win64/openvr_api.lib")
endif()
