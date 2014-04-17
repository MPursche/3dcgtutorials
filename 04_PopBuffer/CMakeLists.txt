# Define project
cmake_minimum_required(VERSION 2.6)
project(PopBuffer)

# Set target name
set(target PopBuffer)

find_package(OpenGL REQUIRED)
find_package(OpenSceneGraph REQUIRED osgViewer osgGA osgDB osgUtil)

set(CMAKE_DEBUG_POSTFIX "d")

# add osg node subproject
add_subdirectory(osgPop)


# Set include directories
include_directories(
    ${OPENSCENEGRAPH_INCLUDE_DIRS}
    ${OPENGL_INCLUDE_DIR}
	osgPop
)

# Define source files
set(sources
	src/AddTextureUniformVisitor.h
	src/DemoEventHandler.cpp
	src/DemoEventHandler.h
	src/KdTreeVisitor.cpp
	src/KdTreeVisitor.h
	src/UpdateViewSpaceErrorVisitor.h
    src/main.cpp
)

# Define shader files
set(shader
	shader/popbuffer.vert
	shader/popbuffer.frag
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
    osgPop
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


# add serializer for pop geometry to the project
set(OSG_PLUGIN_DIR "" CACHE PATH "Installation dir for PopGeometry serializer")
add_subdirectory(serializer)