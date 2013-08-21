// osg
#include <osg/Geometry>

#include "ComputeInstanceBoundingBoxCallback.h"

namespace osgExample
{

osg::BoundingBox ComputeInstancedBoundingBoxCallback::computeBound(const osg::Drawable& drawable) const
{
	osg::BoundingBox bounds;
	const osg::Geometry* geometry = dynamic_cast<const osg::Geometry*>(&drawable);

	if (!geometry)
		return bounds;

	const osg::Vec3Array* vertices = dynamic_cast<const osg::Vec3Array*>(geometry->getVertexArray());

	for (unsigned int i = 0; i < m_instanceMatrices->getNumElements(); ++i)
	{
		osg::Matrixd matrix;
		m_instanceMatrices->getElement(i, matrix);


		for (auto it = vertices->begin(); it != vertices->end(); ++it)
		{
			bounds.expandBy(*it * matrix);
		}
	}

	return bounds;
}

}