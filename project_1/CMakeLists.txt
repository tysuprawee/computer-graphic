# CMake entry point
cmake_minimum_required (VERSION 3.5)
project (CG_Project)
set(CMAKE_CXX_STANDARD 17)

find_package(OpenGL REQUIRED)


if( CMAKE_BINARY_DIR STREQUAL CMAKE_SOURCE_DIR )
    message( FATAL_ERROR "Please select another Build Directory ! (and give it a clever name, like bin_Visual2012_64bits/)" )
endif()
if( CMAKE_SOURCE_DIR MATCHES " " )
	message( "Your Source Directory contains spaces. If you experience problems when compiling, this can be the cause." )
endif()
if( CMAKE_BINARY_DIR MATCHES " " )
	message( "Your Build Directory contains spaces. If you experience problems when compiling, this can be the cause." )
endif()



# Compile external dependencies 
add_subdirectory (external)

# On Visual 2005 and above, this module can set the debug working directory
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/external/rpavlik-cmake-modules-fe2273")
include(CreateLaunchers)
include(MSVCMultipleProcessCompile) # /MP


include_directories(
	external/glfw-3.1.2/include/
	external/glm-0.9.7.1/
	external/glew-1.13.0/include/
	external/assimp-3.0.1270/include/
	.
)

set(ALL_LIBS
	${OPENGL_LIBRARY}
	glfw
	GLEW_1130
)

add_definitions(
	-DTW_STATIC
	-DTW_NO_LIB_PRAGMA
	-DTW_NO_DIRECT3D
	-DGLEW_STATIC
	-D_CRT_SECURE_NO_WARNINGS
)


# Sources shared by every task executable.
set(SHARED_SOURCES
	source/window.cpp
	source/window.hpp
	source/meshObject.cpp
	source/meshObject.hpp
	source/gridObject.cpp
	source/gridObject.hpp
	common/shader.cpp
	common/shader.hpp
	common/controls.cpp
	common/controls.hpp
	common/texture.cpp
	common/texture.hpp
	common/objloader.cpp
	common/objloader.hpp
	common/vboindexer.cpp
	common/vboindexer.hpp

	source/meshVertexShader.glsl
	source/meshFragmentShader.glsl
	source/gridVertexShader.glsl
	source/gridFragmentShader.glsl
	source/pickingVertexShader.glsl
	source/pickingFragmentShader.glsl
)

# One executable per task, so each can be demonstrated on its own:
#   p1 = Task 1 grid, p2 = Task 2 camera, p3 = Task 3 robot arm.
# They all run from source/, which is where the shaders and .obj files live.
function(add_task_executable target mainFile)
	add_executable(${target} ${mainFile} ${SHARED_SOURCES})
	target_link_libraries(${target} ${ALL_LIBS})

	# Xcode and Visual working directories
	set_target_properties(${target} PROPERTIES
		XCODE_ATTRIBUTE_CONFIGURATION_BUILD_DIR "${CMAKE_CURRENT_SOURCE_DIR}/source/")
	if (MSVC)
		create_target_launcher(${target} WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/source/")
	endif()

	if (NOT ${CMAKE_GENERATOR} MATCHES "Xcode")
		add_custom_command(
			TARGET ${target} POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E copy
				"${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}/${target}${CMAKE_EXECUTABLE_SUFFIX}"
				"${CMAKE_CURRENT_SOURCE_DIR}/source/"
		)
	endif()
endfunction()

add_task_executable(p1 source/main_task1.cpp)
add_task_executable(p2 source/main_task2.cpp)
add_task_executable(p3 source/main_task3.cpp)


SOURCE_GROUP(common REGULAR_EXPRESSION ".*/common/.*" )
SOURCE_GROUP(shaders REGULAR_EXPRESSION ".*/.*shader$" )


if (NOT ${CMAKE_GENERATOR} MATCHES "Xcode" )
add_custom_command(
   TARGET p1 POST_BUILD
   COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}/p1${CMAKE_EXECUTABLE_SUFFIX}" "${CMAKE_CURRENT_SOURCE_DIR}/source/"
)

elseif (${CMAKE_GENERATOR} MATCHES "Xcode" )

endif (NOT ${CMAKE_GENERATOR} MATCHES "Xcode" )

