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

#include <GL/glew.h>

#include <iostream>

#include "InstancedDrawable.h"

struct VertexData
{
	GLfloat vertex[3];
	GLfloat normal[3];
	GLfloat texCoord[2];
};

namespace osgExample
{

InstancedDrawable::InstancedDrawable()
	:	m_dirty(true),
		m_vao(0u),
		m_vbo(0u),
		m_instancebo(0u),
		m_ebo(0u),
		m_vertexArray(NULL),
		m_normalArray(NULL),
		m_texCoordArray(NULL),
		m_drawElements(NULL)
{
	setUseDisplayList(false);
	setUseVertexBufferObjects(true);
}

InstancedDrawable::InstancedDrawable(const InstancedDrawable& other, const osg::CopyOp& copyOp)
	:	osg::Drawable(other, copyOp),
		m_vao(0u),
		m_vbo(0u),
		m_instancebo(0u),
		m_ebo(0u),
		m_vertexArray(dynamic_cast<osg::Vec3Array*>(copyOp(other.m_vertexArray))),
		m_normalArray(dynamic_cast<osg::Vec3Array*>(copyOp(other.m_normalArray))),
		m_texCoordArray(dynamic_cast<osg::Vec2Array*>(copyOp(other.m_texCoordArray))),
		m_drawElements(dynamic_cast<osg::DrawElements*>(copyOp(other.m_drawElements)))
{
}

InstancedDrawable::~InstancedDrawable()
{
	glDeleteBuffers(1, &m_vbo);
	glDeleteBuffers(1, &m_instancebo);
	glDeleteBuffers(1, &m_ebo);
	glDeleteVertexArrays(1, &m_vao);
}

osg::BoundingBox InstancedDrawable::computeBound() const
{
	osg::BoundingBox bb;

	for (auto it = m_matrixArray.begin(); it != m_matrixArray.end(); ++it)
	{
		for (unsigned int i = 0; i < m_vertexArray->getNumElements(); ++i)
		{
			bb.expandBy(m_vertexArray->at(i) * (*it));
		}
	}

	return bb;
}

void InstancedDrawable::compileGLObjects(osg::RenderInfo& renderInfo) const
{
	if(!m_vbo || !m_instancebo || !m_ebo || !m_vao)
	{
		GLuint buffers[] = {0u, 0u, 0u};
		glGenBuffers(3, buffers);
		m_vbo = buffers[0];
		m_instancebo = buffers[1];
		m_ebo = buffers[2];

		// create one array to fit all vertex data
		VertexData* vertexData = new VertexData[m_vertexArray->size()];
		for (unsigned int i = 0; i < m_vertexArray->size(); ++i)
		{
			vertexData[i].vertex[0] = m_vertexArray->at(i).x();
			vertexData[i].vertex[1] = m_vertexArray->at(i).y();
			vertexData[i].vertex[2] = m_vertexArray->at(i).z();

			vertexData[i].normal[0] = m_normalArray->at(i).x();
			vertexData[i].normal[1] = m_normalArray->at(i).y();
			vertexData[i].normal[2] = m_normalArray->at(i).z();

			vertexData[i].texCoord[0] = m_texCoordArray->at(i).x();
			vertexData[i].texCoord[1] = m_texCoordArray->at(i).y();
		}
		
		glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(VertexData) * m_vertexArray->size(), vertexData, GL_STATIC_DRAW);
		delete[] vertexData;

		// pack matrices into float array
		osg::ref_ptr<osg::FloatArray> matrixArray = new osg::FloatArray(m_matrixArray.size()*16);
	
		for (unsigned int i = 0; i < m_matrixArray.size(); ++i)
		{
			for (unsigned int j = 0; j < 16; ++j)
			{
				(*matrixArray)[i*16+j] =  m_matrixArray[i].ptr()[j];
			}
		}

		glBindBuffer(GL_ARRAY_BUFFER, m_instancebo);
		glBufferData(GL_ARRAY_BUFFER, matrixArray->getTotalDataSize(), matrixArray->getDataPointer(), GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_drawElements->getTotalDataSize(), m_drawElements->getDataPointer(), GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		
		glGenVertexArrays(1, &m_vao);

		glBindVertexArray(m_vao);
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glEnableVertexAttribArray(2);
		glEnableVertexAttribArray(3);
		glEnableVertexAttribArray(4);
		glEnableVertexAttribArray(5);
		glEnableVertexAttribArray(6);
		glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), 0);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), (GLvoid*)(sizeof(GLfloat) * 3));
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(VertexData), (GLvoid*)(sizeof(GLfloat) * 6));
		glBindBuffer(GL_ARRAY_BUFFER, m_instancebo);
		glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 16 * sizeof(float), 0);
		glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, 16 * sizeof(float), (GLvoid*)(4  * sizeof(float)));
		glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, 16 * sizeof(float), (GLvoid*)(8  * sizeof(float)));
		glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, 16 * sizeof(float), (GLvoid*)(12 * sizeof(float)));
		glVertexAttribDivisor(3, 1);
		glVertexAttribDivisor(4, 1);
		glVertexAttribDivisor(5, 1);
		glVertexAttribDivisor(6, 1);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
		glBindVertexArray(0);

		// unbind all buffers to prevent undefined behavior of osg
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}
}

void InstancedDrawable::drawImplementation(osg::RenderInfo& renderInfo) const
{
	glBindVertexArray(m_vao);
	GLenum dataType;
	switch(m_drawElements->getType())
	{
	case osg::DrawElements::DrawElementsUBytePrimitiveType:
		dataType = GL_UNSIGNED_BYTE;
		break;
	case osg::DrawElements::DrawElementsUShortPrimitiveType:
		dataType = GL_UNSIGNED_SHORT;
		break;
	case osg::DrawElements::DrawElementsUIntPrimitiveType:
	default:
		dataType = GL_UNSIGNED_INT;
		break;
	}

	glDrawElementsInstanced(m_drawElements->getMode(), m_drawElements->getNumIndices(), dataType, NULL, m_drawElements->getNumInstances());
	glBindVertexArray(0);
}

}