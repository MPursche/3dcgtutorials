#ifndef _COMPUTE_INSTANCE_BOUNDING_BOX_CALLBACK_H
#define _COMPUTE_INSTANCE_BOUNDING_BOX_CALLBACK_H

// osg
#include <osg/ref_ptr>
#include <osg/Uniform>
#include <osg/Drawable>

namespace osgExample
{

class ComputeInstancedBoundingBoxCallback : public osg::Drawable::ComputeBoundingBoxCallback
{
public:
	ComputeInstancedBoundingBoxCallback(osg::ref_ptr<osg::Uniform> instanceMatrices)
		: m_instanceMatrices(instanceMatrices)
	{
	}

		virtual osg::BoundingBox computeBound(const osg::Drawable& drawable) const;
private:
	osg::ref_ptr<osg::Uniform> m_instanceMatrices;
};

}

#endif