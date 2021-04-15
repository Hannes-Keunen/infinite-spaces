add_subdirectory(glad)

set(JSON_BuildTests OFF CACHE INTERNAL "")
add_subdirectory(json)

add_library(stb_image STATIC ${CMAKE_CURRENT_SOURCE_DIR}/stb/stb_image.c)
target_include_directories(stb_image PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/stb)

# add_library(OpenVR SHARED IMPORTED)
# set_target_properties(OpenVR PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/openvr/bin/linux64/libopenvr_api.so)
add_library(OpenVR INTERFACE)
target_link_libraries(OpenVR INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/openvr/bin/linux64/libopenvr_api.so)
target_include_directories(OpenVR INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/openvr/headers)
