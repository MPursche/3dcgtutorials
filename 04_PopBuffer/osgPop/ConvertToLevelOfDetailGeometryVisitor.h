#pragma once

#include <memory>

#include <osg/Array>
#include <osg/Geode>
#include <osg/NodeVisitor>

#include "LevelOfDetailGeometry.h"

namespace osgUtil
{

struct HalfEdge;

class OSG_EXPORT ConvertToLevelOfDetailGeometryVisitor : public osg::NodeVisitor
{
public:
	ConvertToLevelOfDetailGeometryVisitor()
		: osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN)
	{
	}

	virtual void apply(osg::Geode& geode);
protected:
	osg::ref_ptr<osg::LevelOfDetailGeometry> convert(osg::ref_ptr<osg::Geometry> geometry) const;
    bool collectHalfEdges(osg::ref_ptr<osg::Geometry> geometry,
                          osg::ref_ptr<osg::LevelOfDetailGeometry>  lodGeometry,
                          std::vector<HalfEdge>*      halfEdges) const;
	bool collectLod(osg::ref_ptr<osg::Geometry> geometry,
                    float min,
                    float max,
                    int numProtectedVertices) const;
	void findHalfEdgeOpposite(std::vector<HalfEdge>* halfEdges) const;
    void findAndSortProtectedVertices(osg::ref_ptr<osg::Geometry> geometry, osg::ref_ptr<osg::LevelOfDetailGeometry> lodGeometry, std::vector<HalfEdge>* halfEdges) const;
	osg::ref_ptr<osg::Array> createArrayOfType(osg::ref_ptr<osg::Array> rhs) const;
	void addElementTo(osg::ref_ptr<osg::Array> dst, osg::ref_ptr<osg::Array> src, size_t element) const;
	void mergeArrays(osg::ref_ptr<osg::Array> first, osg::ref_ptr<osg::Array> second) const;
};

}