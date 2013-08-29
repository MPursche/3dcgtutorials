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

#ifndef _INSTANCED_GEOMETRY_H
#define _INSTANCED_GEOMETRY_H

// osg
#include <osg/Drawable>

namespace osgExample
{

class InstancedDrawable : public osg::Drawable
{
public:
	InstancedDrawable();
	InstancedDrawable(const InstancedDrawable& other, const osg::CopyOp& copyOp);
	~InstancedDrawable();

	META_Object(osgExample, InstancedDrawable)

	virtual osg::BoundingBox computeBound() const;
	virtual void compileGLObjects(osg::RenderInfo& renderInfo) const;
	virtual void drawImplementation(osg::RenderInfo& renderInfo) const;

	void inline setVertexArray(osg::ref_ptr<osg::Vec3Array> vertexArray) { m_vertexArray = vertexArray; }
	void inline setMatrixArray(const std::vector<osg::Matrixd>& matrixArray) { m_matrixArray = matrixArray; }
	void inline setNormalArray(osg::ref_ptr<osg::Vec3Array> normalArray) { m_normalArray = normalArray; }
	void inline setTexCoordArray(osg::ref_ptr<osg::Vec2Array> texCoordArray) { m_texCoordArray = texCoordArray; }
	void inline setDrawElements(osg::ref_ptr<osg::DrawElements> drawElements) { m_drawElements = drawElements; }

private:
	bool								m_dirty;
	mutable GLuint						m_vao;
	mutable GLuint						m_vbo;
	mutable GLuint						m_instancebo;
	mutable GLuint						m_ebo;

	osg::ref_ptr<osg::Uniform> m_modelViewProjectionMatrix;
	osg::ref_ptr<osg::Uniform> m_normalMatrix;

	osg::ref_ptr<osg::Vec3Array>		m_vertexArray;
	std::vector<osg::Matrixd>			m_matrixArray;
	osg::ref_ptr<osg::Vec3Array>		m_normalArray;
	osg::ref_ptr<osg::Vec2Array>		m_texCoordArray;
	osg::ref_ptr<osg::DrawElements>		m_drawElements;
};

} // namespace osgExample

#endif