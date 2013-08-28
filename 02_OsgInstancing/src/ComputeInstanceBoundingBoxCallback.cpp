/*
	The MIT License (MIT)

	Copyright (c) 2013 Marcel Pursche

	Permission is hereby granted, free of charge, to any person obtaining a copy of
	this software and associated documentation files (the "Software"), to deal in
	the Software without restriction, including without limitation the rights to
	use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
	the Software, and to permit persons to whom the Software is furnished to do so,
	subject to the following conditions:

	The above copyright notice and this permission notice shall be included in all
	copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
	FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
	COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
	IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
	CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

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