# Set target name
set(target osgPop)

# Set include directories
include_directories(
    ${OPENSCENEGRAPH_INCLUDE_DIRS}
    ${OPENGL_INCLUDE_DIR}
)

# Define source files
set(sources
	ConvertToPopGeometryVisitor.cpp
	ConvertToPopGeometryVisitor.h
	HalfEdge.h
    PopGeometry.cpp
    PopGeometry.h
    PopDrawElements.cpp
    PopDrawElements.h
	Vec3ui.h
)

# Create executable
add_library(${target} SHARED ${sources})
add_definitions(-DOSG_LIBRARY)

IF(MSVC)  
 #   SET(CMAKE_CXX_FLAGS /bigobj)
ENDIF(MSVC)

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