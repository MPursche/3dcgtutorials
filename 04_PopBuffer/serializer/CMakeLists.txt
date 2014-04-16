INCLUDE_DIRECTORIES(
    ${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/../osgPop
    ${CMAKE_CURRENT_BINARY_DIR} 
    ${OPENSCENEGRAPH_INCLUDE_DIRS}
)

SET(LIBNAME osgdb_serializers_osgpop)

SET(SERIALIZERS_OSG
    PopGeometry.cpp
)

IF(UNIX)
    IF( CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64" )
	ADD_DEFINITIONS(-fPIC)
    ENDIF( CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64" )
ENDIF(UNIX)

ADD_LIBRARY(${LIBNAME}
    SHARED
    ${SERIALIZERS_OSG}
)
add_definitions(-DOSG_LIBRARY)

TARGET_LINK_LIBRARIES(${LIBNAME}
    ${OPENSCENEGRAPH_LIBRARIES}
	osgPop
)

# set debug postfix to d to comply with osg serializer name pattern
SET_TARGET_PROPERTIES(${LIBNAME}
    PROPERTIES
    DEBUG_POSTFIX "d"
    RELEASE_POSTFIX ""
)

IF(MSVC)  
    SET_TARGET_PROPERTIES(${LIBNAME}
        PROPERTIES
        LINK_FLAGS "/NODEFAULTLIB:libcmt /SUBSYSTEM:CONSOLE")

ENDIF(MSVC)

# install plugin to osg plugin dir
# RENDERING_SERVER_INSTALL(${LIBNAME})
IF (MSVC)
  #INSTALL(TARGETS ${LIBNAME}
  #    LIBRARY DESTINATION ${OSG_PLUGIN_DIR}
  #    CONFIGURATIONS)
  INSTALL(FILES ${CMAKE_BINARY_DIR}/bin/osgdb_serializers_osgpop.dll
	DESTINATION ${OSG_PLUGIN_DIR}
	RENAME osgdb_serializers_osgpop.dll)
ELSE (MSVC)
 IF (CMAKE_BUILD_TYPE MATCHES Debug)
    MESSAGE(STATUS "Install plugin: ${CMAKE_BINARY_DIR}/bin/libosgdb_serializers_preprocessord.so")
    INSTALL(FILES ${CMAKE_BINARY_DIR}/bin/libosgdb_serializers_osgpopd.so
	DESTINATION ${OSG_PLUGIN_DIR}
	RENAME osgdb_serializers_osgpopd.so)
  ELSE (CMAKE_BUILD_TYPE MATCHES Debug)
    MESSAGE(STATUS "Install plugin: ${CMAKE_BINARY_DIR}/bin/libosgdb_serializers_preprocessor.so")
    INSTALL(FILES ${CMAKE_BINARY_DIR}/bin/libosgdb_serializers_osgpop.so
	DESTINATION ${OSG_PLUGIN_DIR}
	RENAME osgdb_serializers_osgpop.so)
  ENDIF (CMAKE_BUILD_TYPE MATCHES Debug)
ENDIF (MSVC)