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

#ifndef _INSTANCED_GEOMETRY_BUILDER_H
#define _INSTANCED_GEOMETRY_BUILDER_H

// std
#include <vector>

// osg
#include <osg/Referenced>
#include <osg/ref_ptr>
#include <osg/Matrix>
#include <osg/Geometry>
#include <osg/Node>

namespace osgExample
{

class InstancedGeometryBuilder : public osg::Referenced
{
public:
	InstancedGeometryBuilder()
		:	m_maxMatrixUniforms(16),
			m_maxTextureResolution(16384u * 4096u)
	{
	}
	
	InstancedGeometryBuilder(GLint maxMatrixUniforms)
		:	m_maxMatrixUniforms(maxMatrixUniforms),
			m_maxTextureResolution(16384u * 4096u)
	{
	}
	
	inline void setGeometry(osg::ref_ptr<osg::Geometry> geometry) { m_geometry = geometry; }
	inline osg::ref_ptr<osg::Geometry> getGeometry() const { return m_geometry; }

	inline void addMatrix(const osg::Matrixd& matrix) { m_matrices.push_back(matrix); }
	inline osg::Matrixd getMatrix(size_t index) const { return m_matrices[index]; }

	osg::ref_ptr<osg::Node> getSoftwareInstancedNode() const;
	osg::ref_ptr<osg::Node> getHardwareInstancedNode() const;
	osg::ref_ptr<osg::Node> getTextureHardwareInstancedNode() const;

private:
	osg::ref_ptr<osg::Node>   createHardwareInstancedGeode(unsigned int start, unsigned int end) const;
	osg::ref_ptr<osg::Node>   createTextureHardwareInstancedGeode(unsigned int start, unsigned int end) const;
	osg::ref_ptr<osg::Shader> readShaderFile(const std::string& fileName, const std::string& preprocessorDefinitions) const;

	GLint						m_maxMatrixUniforms;
	unsigned int				m_maxTextureResolution;
	osg::ref_ptr<osg::Geometry> m_geometry;
	std::vector<osg::Matrixd>   m_matrices;
};

}

#endif