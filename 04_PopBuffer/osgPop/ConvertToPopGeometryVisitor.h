#pragma once

#include <memory>

#include <osg/Array>
#include <osg/Geode>
#include <osg/NodeVisitor>

#include "PopGeometry.h"

namespace osgPop
{

struct HalfEdge;

class OSG_EXPORT ConvertToPopGeometryVisitor : public osg::NodeVisitor
{
public:
	ConvertToPopGeometryVisitor()
		: osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN)
	{
	}

	virtual void apply(osg::Geode& geode);
protected:
	osg::ref_ptr<PopGeometry> convert(osg::ref_ptr<osg::Geometry> geometry);
    bool collectHalfEdges(osg::ref_ptr<osg::Geometry> geometry,
                          osg::ref_ptr<osg::DrawElementsUInt>&     drawElements,
                          std::shared_ptr<std::vector<HalfEdge> >& halfEdges);
	bool collectLod(osg::ref_ptr<osg::Geometry> geometry,
                    std::shared_ptr<std::vector<osg::ref_ptr<osg::DrawElementsUInt> > >& lodDrawElements,
                    float min,
                    float max,
                    int numProtectedVertices);
	void findHalfEdgeOpposite(std::shared_ptr<std::vector<HalfEdge> > halfEdges);
	void createAndAddDrawPrimitive(std::shared_ptr<std::vector<osg::ref_ptr<osg::DrawElementsUInt> > > lodDrawElements,
                                   osg::ref_ptr<PopGeometry> popGeometry);
    void findAndSortProtectedVertices(osg::ref_ptr<osg::Geometry> geometry, osg::ref_ptr<PopGeometry> popGeometry, std::shared_ptr<std::vector<HalfEdge> > halfEdges);
	osg::ref_ptr<osg::Array> createArrayOfType(osg::ref_ptr<osg::Array> rhs);
	void addElementTo(osg::ref_ptr<osg::Array> dst, osg::ref_ptr<osg::Array> src, size_t element);
	void mergeArrays(osg::ref_ptr<osg::Array> first, osg::ref_ptr<osg::Array> second);
};

}