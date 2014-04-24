# Define project
cmake_minimum_required(VERSION 2.6)
project(ShadowMapping)

# Set target name
set(target ShadowMapping)

find_package(OpenGL REQUIRED)
find_package(OpenSceneGraph REQUIRED osgViewer osgGA osgDB osgUtil)

# Set include directories
include_directories(
    ${OPENSCENEGRAPH_INCLUDE_DIRS}
    ${OPENGL_INCLUDE_DIR}
)

# Define source files
set(sources
    src/main.cpp
    src/ShadowedScene.cpp
    src/ShadowedScene.h
    src/FloatBufferShadowedScene.cpp
    src/FloatBufferShadowedScene.h
    src/DepthBufferShadowedScene.cpp
    src/DepthBufferShadowedScene.h
    src/SwitchTechniqueHandler.h
)

# Define shader files
set(shader
	shader/shadow_pass.vert
	shader/shadow_pass.frag
	shader/main_pass.vert
	shader/main_pass.frag
	shader/main_pass_untextured.vert
	shader/main_pass_untextured.frag
	shader/depthbuffer/shadow_pass.vert
	shader/depthbuffer/shadow_pass.frag
	shader/depthbuffer/main_pass.vert
	shader/depthbuffer/main_pass.frag
	shader/depthbuffer/main_pass_untextured.vert
	shader/depthbuffer/main_pass_untextured.frag	
)

# Define data files
set(data
	data/cow.osg
)

# Create executable
add_executable(${target} ${sources} ${shader})

target_link_libraries(${target}
    ${OPENSCENEGRAPH_LIBRARIES}
    ${OPENGL_LIBRARIES}    
)

# Setup Install Target
install(TARGETS ${target}
	RUNTIME DESTINATION bin CONFIGURATIONS
	LIBRARY DESTINATION lib CONFIGURATIONS
    ARCHIVE DESTINATION lib CONFIGURATIONS
	PUBLIC_HEADER DESTINATION include CONFIGURATIONS)
install(FILES ${shader} DESTINATION shader)
install(FILES ${data} DESTINATION data)

# for linux compatibility
if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_GNUCC)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
endif(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_GNUCC)