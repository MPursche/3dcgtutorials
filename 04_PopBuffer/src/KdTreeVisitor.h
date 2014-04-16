#pragma once

#include <vector>
#include <memory>

#include <osg/ref_ptr>
#include <osg/NodeVisitor>
#include <osg/Geometry>

namespace osgExample {

class KdTreeVisitor : public osg::NodeVisitor {
public:
    KdTreeVisitor(unsigned int maxVertices=65536u)
        : osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN)
        , m_maxVertices(maxVertices)
    {
    }

    virtual void apply(osg::Geode& geode);
    
    enum Axis {
        X_AXIS = 0,
        Y_AXIS = 1,
        Z_AXIS = 2
    };
private:
    std::vector<osg::ref_ptr<osg::Drawable> > splitGeometry(osg::ref_ptr<osg::Geometry> geometry, Axis splitAxis=X_AXIS);
    void sortByAxis(osg::ref_ptr<osg::Array> sortedVertices, Axis splitAxis);
    osg::ref_ptr<osg::DrawElementsUInt> collectTriangles(osg::ref_ptr<osg::Geometry> geometry);
    osg::ref_ptr<osg::Array> createArrayOfType(osg::ref_ptr<osg::Array> rhs);
	void addElementTo(osg::ref_ptr<osg::Array> dst, osg::ref_ptr<osg::Array> src, size_t element);

    unsigned int m_maxVertices;
};

}