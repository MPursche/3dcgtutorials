#pragma once

#include <osg/NodeVisitor>
#include "PopGeometry.h"

namespace osgExample
{

class UpdateViewSpaceError : public osg::NodeVisitor
{
public:
	UpdateViewSpaceError(float maxViewSpaceError=1.0f)
		: osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN) 
		, _maxViewSpaceError(maxViewSpaceError)
	{
	}

	virtual void apply(osg::Geode& geode)
	{
		for (auto drawable: geode.getDrawableList())
		{
			osg::ref_ptr<osgPop::PopGeometry> popGeometry = dynamic_cast<osgPop::PopGeometry*>(drawable.get());

			if (popGeometry)
			{
				popGeometry->setMaxViewSpaceError(_maxViewSpaceError);
			}
		}
	}
protected:
	float _maxViewSpaceError;
};

}