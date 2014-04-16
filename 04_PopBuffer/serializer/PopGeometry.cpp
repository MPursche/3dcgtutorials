#include "PopGeometry.h"
#include "PopDrawElements.h"

#include <vector>
#include <iostream>

#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

static bool checkLodRange(const osgPop::PopGeometry& popGeometry)
{
    return true;
}

static bool readLodRange(osgDB::InputStream& is, osgPop::PopGeometry& popGeomtetry)
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

static bool writeLodRange(osgDB::OutputStream& os, const osgPop::PopGeometry& popGeometry)
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

struct PopGeometryFinishedObjectReadCallback : public osgDB::FinishedObjectReadCallback
{
    virtual void objectRead(osgDB::InputStream&, osg::Object& obj)
    {
        osgPop::PopGeometry& geometry = static_cast<osgPop::PopGeometry&>(obj);
        
        // convert primitive sets to pop primitives
        for (size_t i = 0; i < geometry.getNumPrimitiveSets(); ++i)
        {
            osg::DrawElementsUInt* drawElements = dynamic_cast<osg::DrawElementsUInt*>(geometry.getPrimitiveSet(i));

            if (drawElements)
            {
                geometry.setPrimitiveSet(i, new osgPop::PopDrawElements(*drawElements));
            }
        }

        // reconnect uniforms
        geometry.reconnectUniforms();
    }
};

REGISTER_OBJECT_WRAPPER( PopGeometry,
                         new osgPop::PopGeometry,
                         osgPop::PopGeometry,
                         "osg::Object osg::Drawable osg::Geometry osgPop::PopGeometry" )
{
    ADD_FLOAT_SERIALIZER( MinBounds, FLT_MIN ); // _min
    ADD_FLOAT_SERIALIZER( MaxBounds, FLT_MAX ); // _max
    ADD_INT_SERIALIZER( NumberOfFixedVertices, 0 ); // _numFixedVertices
    ADD_USER_SERIALIZER( LodRange ); // _lodRange
    ADD_FLOAT_SERIALIZER( MaxViewSpaceError, 0.5f );

    wrapper->addFinishedObjectReadCallback( new PopGeometryFinishedObjectReadCallback() );
}