#include "PopDrawElements.h"

#include <vector>
#include <iostream>

#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

static bool checkLodRange(const osg::PopGeometry& popGeometry)
{
    return true;
}

static bool readLodRange(osgDB::InputStream& is, osg::PopGeometry& popGeomtetry)
{
    std::vector<GLint> lodRange;
    
    is >> is.BEGIN_BRACKET;
    for (size_t i = 0; i < 32; ++i)
    {
        GLint lodEnd;
        is >> lodEnd;
        lodRange.push_back(lodEnd);
    }
    is >> is.END_BRACKET;

    popGeomtetry.getLodRange() = lodRange;

    return true;
}

static bool writeLodRange(osgDB::OutputStream& os, const osg::PopGeometry& popGeometry)
{
    os << os.BEGIN_BRACKET << std::endl;

    const std::vector<GLint>& lodRange = popGeometry.getLodRange();
    for (GLint lodEnd: lodRange)
    {
        os << lodEnd << std::endl;
    }
    os << os.END_BRACKET << std::endl;

    return true;
}

REGISTER_OBJECT_WRAPPER( PopDrawElements,
                         new osg::PopDrawElements,
                         osg::PopDrawElements,
                         "osg::Object osg::Drawable osg::Geometry osg::PopGeometry" )
{
    ADD_FLOAT_SERIALIZER( MinBounds, FLT_MIN ); // _min
    ADD_FLOAT_SERIALIZER( MaxBounds, FLT_MAX ); // _max
    ADD_INT_SERIALIZER( NumberOfFixedVertices, 0 ); // _numFixedVertices
    ADD_USER_SERIALIZER( LodRange ); // _lodRange
    ADD_FLOAT_SERIALIZER( MaxViewSpaceError, 0.5f );
}