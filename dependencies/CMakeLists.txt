add_subdirectory(glad)

set(JSON_BuildTests OFF CACHE INTERNAL "")
add_subdirectory(json)

add_library(stb_image STATIC ${CMAKE_CURRENT_SOURCE_DIR}/stb/stb_image.c)
target_include_directories(stb_image PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/stb)
