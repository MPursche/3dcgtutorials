# Define project
cmake_minimum_required(VERSION 2.6)
project(OsgInstancing)

# Set target name
set(target OsgInstancing)

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
    src/InstancedGeometryBuilder.h
    src/InstancedGeometryBuilder.cpp
	src/SwitchTechniqueHandler.h
	src/ComputeInstanceBoundingBoxCallback.h
	src/ComputeInstanceBoundingBoxCallback.cpp
	src/ComputeTextureBoundingBoxCallback.h
	src/ComputeTextureBoundingBoxCallback.cpp
	src/ASCFileLoader.h
	src/ASCFileLoader.cpp
)

# Define shader files
set(shader
	shader/instancing.vert
	shader/instancing.frag
	shader/no_instancing.vert
	shader/no_instancing.frag
	shader/texture_instancing.vert
	shader/texture_instancing.frag
	shader/ubo_instancing.vert
	shader/ubo_instancing.frag
)

# Define data files
set(data
	data/crater.asc
	data/grass.png
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

# Setup Option to activate ATI Bugfix
option(ATI_FIX "Fixes a bug that occured with Catalyst 11.6(don't now about newer versions, so try yourself)" false)

if(ATI_FIX)
	add_definitions(-DATI_FIX)
endif(ATI_FIX)