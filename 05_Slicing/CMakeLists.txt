# Define project
cmake_minimum_required(VERSION 2.6)
project(Slicing)

# Set target name
set(target Slicing)

find_package(OpenGL REQUIRED)
find_package(OpenSceneGraph REQUIRED osgViewer osgGA osgDB osgUtil)

set(CMAKE_DEBUG_POSTFIX "d")


# Set include directories
include_directories(
    ${OPENSCENEGRAPH_INCLUDE_DIRS}
    ${OPENGL_INCLUDE_DIR}
)

# Define source files
set(sources
    src/main.cpp
)

# Define shader files
set(shader
	shader/simple.vert
	shader/simple.frag
	shader/imposter.vert
	shader/imposter.frag
	shader/dilate.vert
	shader/dilate.frag
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