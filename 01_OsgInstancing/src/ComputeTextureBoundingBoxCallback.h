#ifndef _COMPUTE_TEXTURE_BOUNDING_BOX_CALLBACK_H
#define _COMPUTE_TEXTURE_BOUNDING_BOX_CALLBACK_H

// std
#include <vector>

// osg
#include <osg/Matrixd>
#include <osg/Drawable>

namespace osgExample
{

class ComputeTextureBoundingBoxCallback : public osg::Drawable::ComputeBoundingBoxCallback
{
public:
	ComputeTextureBoundingBoxCallback(std::vector<osg::Matrixd> instanceMatrices)
		: m_instanceMatrices(instanceMatrices)
	{
	}

		virtual osg::BoundingBox computeBound(const osg::Drawable& drawable) const;
private:
	std::vector<osg::Matrixd> m_instanceMatrices;
};

}

#endif