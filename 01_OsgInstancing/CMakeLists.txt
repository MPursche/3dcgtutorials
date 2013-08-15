# Define project
cmake_minimum_required(VERSION 2.6)
project(OsgInstancing)

# Set target name
set(target OsgInstancing)

find_package(OpenSceneGraph REQUIRED osgViewer osgGA osgDB osgUtil)

# Set include directories
include_directories(
    ${OPENSCENEGRAPH_INCLUDE_DIRS}
)

# Define source files
set(sources
    src/main.cpp
    src/InstancedGeometryBuilder.h
    src/InstancedGeometryBuilder.cpp
)

# Define shader files
set(shader
	shader/instancing.vert
	shader/instancing.frag
)

# Define data files
set(data
	data/crater.asc
	data/grass.png
)

# Create executable
add_executable(${target} ${sources})

target_link_libraries(${target}
    ${OPENSCENEGRAPH_LIBRARIES}
    opengl32.lib    
)

# Setup Install Target
install(TARGETS ${target}
	RUNTIME DESTINATION bin CONFIGURATIONS
	LIBRARY DESTINATION lib CONFIGURATIONS
    ARCHIVE DESTINATION lib CONFIGURATIONS
	PUBLIC_HEADER DESTINATION include CONFIGURATIONS)
install(FILES ${shader} DESTINATION shader)
install(FILES ${data} DESTINATION data)