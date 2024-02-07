cmake_minimum_required(VERSION 3.20)
project(Volume_Renderer2)

set(OpenCV_DIR "/opt/local/libexec/opencv4/cmake")

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake_modules")

function(add_shader TARGET SHADER)
    # find_program(GLSLC glslc)

    set(current-shader-path "${CMAKE_CURRENT_SOURCE_DIR}/GLSLShaders/${SHADER}")
    set(current-output-path "${CMAKE_CURRENT_SOURCE_DIR}/Shaders/${SHADER}.spv")

    # Add a custom command to compile GLSL to SPIR-V.
	get_filename_component(current-output-dir "${current-output-path}" DIRECTORY)
	file(MAKE_DIRECTORY "${current-output-dir}")

    add_custom_command(
           OUTPUT "${current-output-path}"
           COMMAND "/Users/eprager/VulkanSDK/1.3.268.1/macOS/bin/glslc" -o "${current-output-path}" "${current-shader-path}"
           DEPENDS "${current-shader-path}"
           IMPLICIT_DEPENDS CXX "${current-shader-path}"
           VERBATIM)

    # Make sure our build depends on this output.
    set_source_files_properties("${current-output-path}" PROPERTIES GENERATED TRUE)
    target_sources("${TARGET}" PRIVATE "${current-output-path}")
	target_sources("${TARGET}" PRIVATE "${current-shader-path}")
	
	add_custom_command (TARGET "${PROJECT_NAME}" POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -E copy_if_different
    "${current-output-path}" "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
	)
endfunction(add_shader)

# Write demo-config.h
#message("Generating header file: ${CMAKE_BINARY_DIR}/demo-config.h")
#set(DEMO_ENABLE_MULTISAMPLE 0 CACHE BOOL "Enable multisample anti-aliasing")
#configure_file(demo-config.h.in "${CMAKE_BINARY_DIR}/demo-config.h")

find_package(wxWidgets REQUIRED COMPONENTS net core base gl)
find_package(OpenGL REQUIRED)
find_package(DCMTK REQUIRED)
find_package(OpenCV REQUIRED)

if(APPLE)
	if(DEFINED ENV_WX_CONFIG)
		set(ENV{WX_CONFIG} ${ENV_WX_CONFIG})
	endif()
endif()

if (wxWidgets_VERSION_STRING LESS 3.2)
    message(FATAL_ERROR "Found wxWidgets Version $(wxWidgets_VERSION_STRING) found. Required >= 3.2.")
endif()

if(UNIX AND NOT APPLE)
    # for Linux, BSD, Solaris, Minix
	find_package(PkgConfig REQUIRED)
	pkg_check_modules(GTK3 REQUIRED gtk+-3.0)
	include_directories("${GTK3_INCLUDE_DIRS}")
	link_directories("${GTK3_LIBRARY_DIRS}")
	add_definitions("${GTK3_CFLAGS_OTHER}")
	
	find_package(Wayland REQUIRED)
	include_directories("${WAYLAND_INCLUDE_DIRS}")

	find_package(X11 REQUIRED)
	include_directories("${X11_INCLUDE_DIR}")
endif()

file(
GLOB_RECURSE HEADERS
RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}"
"include/*.h"
)

file(
GLOB_RECURSE SOURCES
RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}"
"src/*.cpp"
)

file(
GLOB_RECURSE VERTEX_SHADERS
RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}"
"GLSLShaders/*.vert"
)

file(
GLOB_RECURSE FRAGMENT_SHADERS
RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}"
"GLSLShaders/*.frag"
)

include("${wxWidgets_USE_FILE}")

if(APPLE)
set(APPLE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/include/CocoaSurface.h" "${CMAKE_CURRENT_SOURCE_DIR}/src/CocoaSurface.mm")
endif()

include_directories("${OPENGL_INCLUDE_DIR}" "${CMAKE_BINARY_DIR}" "${OpenCV_INCLUDE_DIRS}")

if(WIN32)
	add_executable(${PROJECT_NAME} WIN32
		"${CMAKE_CURRENT_SOURCE_DIR}/${SOURCES}"
		"${CMAKE_CURRENT_SOURCE_DIR}/${HEADERS}"
		"${CMAKE_CURRENT_SOURCE_DIR}/${VERTEX_SHADERS}"
		"${CMAKE_CURRENT_SOURCE_DIR}/${FRAGMENT_SHADERS}"
	)
else()
	if(APPLE)
	add_executable(${PROJECT_NAME}
		"${CMAKE_CURRENT_SOURCE_DIR}/${SOURCES}"
		"${CMAKE_CURRENT_SOURCE_DIR}/${HEADERS}"
		"${CMAKE_CURRENT_SOURCE_DIR}/${VERTEX_SHADERS}"
		"${CMAKE_CURRENT_SOURCE_DIR}/${FRAGMENT_SHADERS}"
		"${APPLE_FILES}"
	)
	else()
	add_executable(${PROJECT_NAME}
		"${CMAKE_CURRENT_SOURCE_DIR}/${SOURCES}"
		"${CMAKE_CURRENT_SOURCE_DIR}/${HEADERS}"
		"${CMAKE_CURRENT_SOURCE_DIR}/${VERTEX_SHADERS}"
		"${CMAKE_CURRENT_SOURCE_DIR}/${FRAGMENT_SHADERS}"
	)
	endif()
endif()

target_include_directories(${PROJECT_NAME} PUBLIC
	"${CMAKE_CURRENT_SOURCE_DIR}/include/"
	"/usr/local/include/"
	"/Users/eprager/local/include/"
	"/opt/local/include/"
)

target_link_directories(${PROJECT_NAME} PUBLIC
	"/usr/local/lib/"
	"/Users/eprager/local/lib/"
	"/opt/local/lib/"
)

target_link_libraries(${PROJECT_NAME}
	SDL2
	SDL2_image
	mattresses
	evk
	${OPENGL_LIBRARIES}
	${wxWidgets_LIBRARIES}
	${DCMTK_LIBRARIES}
	${OpenCV_LIBS}
)

if(UNIX AND NOT APPLE)
    # for Linux, BSD, Solaris, Minix
	target_link_libraries(${PROJECT_NAME} ${GTK3_LIBRARIES})
	
	target_link_libraries(${PROJECT_NAME} ${WAYLAND_LIBRARIES})

	target_link_libraries(${PROJECT_NAME} ${X11_LIBRARIES})
endif()

target_link_libraries(${PROJECT_NAME}
	vulkan
)

# Copy SDL2 DLLs to output folder on Windows
#if(WIN32)
#    foreach(DLL ${SDL2_DLLS})
#        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND
#            ${CMAKE_COMMAND} -E copy_if_different ${DLL} $<TARGET_FILE_DIR:${PROJECT_NAME}>)
#    endforeach()
#endif()

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 20)
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD_REQUIRED ON)

foreach(shader "${VERTEX_SHADERS}")
	cmake_path(GET shader FILENAME shader_filename)
	add_shader(${PROJECT_NAME} "${shader_filename}")
endforeach()
foreach(shader "${FRAGMENT_SHADERS}")
	cmake_path(GET shader FILENAME shader_filename)
	add_shader(${PROJECT_NAME} "${shader_filename}")
endforeach()

#add_custom_command ( TARGET ${PROJECT_NAME} POST_BUILD
#COMMAND ${CMAKE_COMMAND} -E copy_if_different
#${CMAKE_CURRENT_SOURCE_DIR}/src/ct_data.dat $<TARGET_FILE_DIR:${PROJECT_NAME}>
#)
#
#add_custom_command ( TARGET ${PROJECT_NAME} POST_BUILD
#COMMAND ${CMAKE_COMMAND} -E copy_if_different
#${CMAKE_CURRENT_SOURCE_DIR}/src/synthetic_data.dat $<TARGET_FILE_DIR:${PROJECT_NAME}>
#)
#
#set(CUBEMAP_IMAGES ${CMAKE_CURRENT_SOURCE_DIR}/src/cubemap/front.jpg ${CMAKE_CURRENT_SOURCE_DIR}/src/cubemap/back.jpg ${CMAKE_CURRENT_SOURCE_DIR}/src/cubemap/top.jpg ${CMAKE_CURRENT_SOURCE_DIR}/src/cubemap/bottom.jpg ${CMAKE_CURRENT_SOURCE_DIR}/src/cubemap/right.jpg ${CMAKE_CURRENT_SOURCE_DIR}/src/cubemap/left.jpg)
#
#foreach(cubemap_image ${CUBEMAP_IMAGES})
#add_custom_command ( TARGET ${PROJECT_NAME} POST_BUILD
#COMMAND ${CMAKE_COMMAND} -E copy_if_different
#${cubemap_image} $<TARGET_FILE_DIR:${PROJECT_NAME}>
#)
#endforeach()
