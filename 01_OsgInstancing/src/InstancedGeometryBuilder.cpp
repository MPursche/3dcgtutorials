#include "InstancedGeometryBuilder.h"

// osg
#include <osg/Uniform>
#include <osg/Group>
#include <osg/Geode>
#include <osg/MatrixTransform>
#include <osgDB/ReadFile>
#include <osg/Image>
#include <osg/TextureRectangle>

// osgExample
#include "ComputeInstanceBoundingBoxCallback.h"
#include "ComputeTextureBoundingBoxCallback.h"

namespace osgExample
{

osg::ref_ptr<osg::Node> InstancedGeometryBuilder::getSoftwareInstancedNode() const
{
	// create Group to contain all instances
	osg::ref_ptr<osg::Group>	group = new osg::Group;

	// create Geode to wrap Geometry
	osg::ref_ptr<osg::Geode>	geode = new osg::Geode;
	geode->addDrawable(m_geometry);

	// now create a MatrixTransform for each matrix in the list
	for (auto it = m_matrices.begin(); it != m_matrices.end(); ++it)
	{
		osg::ref_ptr<osg::MatrixTransform> matrixTransform = new osg::MatrixTransform(*it);

		matrixTransform->addChild(geode);
		group->addChild(matrixTransform);
	}

	osg::ref_ptr<osg::Program> program = new osg::Program;
	osg::ref_ptr<osg::Shader> vsShader = osgDB::readShaderFile("shader/no_instancing.vert");
	osg::ref_ptr<osg::Shader> fsShader = osgDB::readShaderFile("shader/no_instancing.frag");
	program->addShader(vsShader);
	program->addShader(fsShader);

	group->getOrCreateStateSet()->setAttributeAndModes(program, osg::StateAttribute::ON);

	return group;
}

osg::ref_ptr<osg::Node> InstancedGeometryBuilder::getHardwareInstancedNode() const
{
	osg::ref_ptr<osg::Node> instancedNode;

	// first check if we need to split up the geometry in groups
	if (m_matrices.size() <= m_maxMatrixUniforms)
	{
		// we don't have more matrices than uniform space so we only need one geode
		instancedNode = createHardwareInstancedGeode(0, m_matrices.size());
	} else {
		// we need to split up instances into several geodes
		osg::ref_ptr<osg::Group> group = new osg::Group;

		unsigned int numGeodes = m_matrices.size() / m_maxMatrixUniforms;
		for (unsigned int i = 0; i <= numGeodes; ++i)
		{
			unsigned int start = i*m_maxMatrixUniforms;
			unsigned int end    = std::min((unsigned int)m_matrices.size(), (start + m_maxMatrixUniforms));
			group->addChild(createHardwareInstancedGeode(start, end));
		}
		instancedNode = group;
	}

	osg::ref_ptr<osg::Program> program = new osg::Program;
	osg::ref_ptr<osg::Shader> vsShader = osgDB::readShaderFile("shader/instancing.vert");
	osg::ref_ptr<osg::Shader> fsShader = osgDB::readShaderFile("shader/instancing.frag");
	program->addShader(vsShader);
	program->addShader(fsShader);

	instancedNode->getOrCreateStateSet()->setAttributeAndModes(program, osg::StateAttribute::ON);

	return instancedNode;
}

osg::ref_ptr<osg::Node> InstancedGeometryBuilder::getTextureHardwareInstancedNode() const
{
	osg::ref_ptr<osg::Node> instancedNode;

	// first check if we need to split up the geometry in groups
	if (m_matrices.size() <= m_maxTextureResolution)
	{
		// we don't have more matrices than uniform space so we only need one geode
		instancedNode = createTextureHardwareInstancedGeode(0, m_matrices.size());
	} else {
		// we need to split up instances into several geodes
		osg::ref_ptr<osg::Group> group = new osg::Group;

		unsigned int numGeodes = m_matrices.size() / m_maxTextureResolution;
		for (unsigned int i = 0; i <= numGeodes; ++i)
		{
			unsigned int start = i*m_maxTextureResolution;
			unsigned int end    = std::min((unsigned int)m_matrices.size(), (start + m_maxTextureResolution));
			group->addChild(createTextureHardwareInstancedGeode(start, end));
		}
		instancedNode = group;
	}

	
	// add shaders
	osg::ref_ptr<osg::Program> program = new osg::Program;
	osg::ref_ptr<osg::Shader> vsShader = osgDB::readShaderFile("shader/texture_instancing.vert");
	osg::ref_ptr<osg::Shader> fsShader = osgDB::readShaderFile("shader/texture_instancing.frag");
	program->addShader(vsShader);
	program->addShader(fsShader);

	instancedNode->getOrCreateStateSet()->setAttributeAndModes(program, osg::StateAttribute::ON);

	return instancedNode;
}

osg::ref_ptr<osg::Node> InstancedGeometryBuilder::createHardwareInstancedGeode(unsigned int start, unsigned int end) const
{
		// we don't have more matrices than uniform space so we only need one geode
		osg::ref_ptr<osg::Geode>	geode = new osg::Geode;
		osg::ref_ptr<osg::Geometry> geometry = new osg::Geometry(*m_geometry, osg::CopyOp::DEEP_COPY_ALL);
		geode->addDrawable(geometry);

		// first turn on hardware instancing for every primitive set
		for (unsigned int i = 0; i < geometry->getNumPrimitiveSets(); ++i)
		{
			geometry->getPrimitiveSet(i)->setNumInstances(end-start);
		}

		// we need to turn off display lists for instancing to work
		geometry->setUseDisplayList(false);
		geometry->setUseVertexBufferObjects(true);

		// create uniform array for matrices
		osg::ref_ptr<osg::Uniform> instanceMatrixUniform = new osg::Uniform(osg::Uniform::FLOAT_MAT4, "instanceModelMatrix", end-start);

		for (unsigned int i = start, j = 0; i < end; ++i, ++j)
		{
			instanceMatrixUniform->setElement(j, m_matrices[i]);
		}
		geode->getOrCreateStateSet()->addUniform(instanceMatrixUniform);
			
		// add bounding box callback so osg computes the right bounding box for our geode
		geometry->setComputeBoundingBoxCallback(new ComputeInstancedBoundingBoxCallback(instanceMatrixUniform));


		return geode;
}

osg::ref_ptr<osg::Node> InstancedGeometryBuilder::createTextureHardwareInstancedGeode(unsigned int start, unsigned int end) const
{
	osg::ref_ptr<osg::Geode>	geode = new osg::Geode;
	osg::ref_ptr<osg::Geometry> geometry = new osg::Geometry(*m_geometry, osg::CopyOp::DEEP_COPY_ALL);
	geode->addDrawable(geometry);

	// first turn on hardware instancing for every primitive set
	for (unsigned int i = 0; i < geometry->getNumPrimitiveSets(); ++i)
	{
		geometry->getPrimitiveSet(i)->setNumInstances(m_matrices.size());
	}

	// we need to turn off display lists for instancing to work
	geometry->setUseDisplayList(false);
	geometry->setUseVertexBufferObjects(true);
	
	// create texture to encode all matrices
	osg::ref_ptr<osg::Image> image = new osg::Image;
	image->allocateImage(4, end-start, 1, GL_RGBA, GL_FLOAT);
	image->setInternalTextureFormat(GL_RGBA32F_ARB);

	for (unsigned int i = start, j = 0; i < end; ++i, ++j)
	{
		osg::Matrixf matrix = m_matrices[i];
		float * data = (float*)image->data(0, j);
		memcpy(data, matrix.ptr(), 16 * sizeof(float));
	}

	osg::ref_ptr<osg::TextureRectangle> texture = new osg::TextureRectangle(image);
	texture->setInternalFormat(GL_RGBA32F_ARB);
	texture->setSourceFormat(GL_RGBA);
	texture->setSourceType(GL_FLOAT);
	texture->setTextureSize(4, end-start);
	texture->setFilter(osg::Texture::MAG_FILTER, osg::Texture::NEAREST);
	texture->setFilter(osg::Texture::MIN_FILTER, osg::Texture::NEAREST);
	texture->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_BORDER);
	texture->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_BORDER);

	geode->getOrCreateStateSet()->setTextureAttributeAndModes(1, texture, osg::StateAttribute::ON);
	geode->getOrCreateStateSet()->addUniform(new osg::Uniform("instanceMatrixTexture", 1));

	// copy part of matrix list and create bounding box callback
	std::vector<osg::Matrixd> matrices;
	matrices.insert(matrices.begin(), m_matrices.begin()+start, m_matrices.begin()+end);
	geometry->setComputeBoundingBoxCallback(new ComputeTextureBoundingBoxCallback(matrices));

	return geode;
}

}