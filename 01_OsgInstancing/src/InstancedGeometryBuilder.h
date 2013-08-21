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
			m_maxTextureResolution(16384)
	{
	}
	
	InstancedGeometryBuilder(GLint maxMatrixUniforms)
		:	m_maxMatrixUniforms(maxMatrixUniforms),
			m_maxTextureResolution(16384)
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
	osg::ref_ptr<osg::Node> createHardwareInstancedGeode(unsigned int start, unsigned int end) const;
	osg::ref_ptr<osg::Node> createTextureHardwareInstancedGeode(unsigned int start, unsigned int end) const;

	GLint						m_maxMatrixUniforms;
	unsigned int				m_maxTextureResolution;
	osg::ref_ptr<osg::Geometry> m_geometry;
	std::vector<osg::Matrixd>   m_matrices;
};

}

#endif