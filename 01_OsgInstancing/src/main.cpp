// c-std
#include <ctime>
#include <cstdlib>
#define _USE_MATH_DEFINES 1 // fix for visual studio
#include <cmath>

// osg
#include <osg/ref_ptr>
#include <osg/Switch>
#include <osg/Group>
#include <osg/Geode>
#include <osg/Geometry>
#include <osg/AlphaFunc>
#include <osgGA/StateSetManipulator>
#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>
#include <osgDB/ReadFile>

// osgExample
#include "InstancedGeometryBuilder.h"

osg::ref_ptr<osg::Switch> setupScene(unsigned int x, unsigned int y, GLint maxInstanceMatrices);

class SwitchInstancingHandler : public osgGA::GUIEventHandler
{
public:
	SwitchInstancingHandler(osg::ref_ptr<osgViewer::Viewer> viewer, osg::ref_ptr<osg::Switch> switchNode, GLint maxInstanceMatrices)
		:	m_viewer(viewer),
			m_switch(switchNode),
			m_size(50.f),
			m_maxInstanceMatrices(maxInstanceMatrices)
	{
	}

	virtual bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
	{
		if (ea.getEventType() == osgGA::GUIEventAdapter::KEYUP)
		{
			switch(ea.getKey())
			{
			case osgGA::GUIEventAdapter::KEY_1:
				m_switch->setSingleChildOn(0);
				return true;
				break;
			case osgGA::GUIEventAdapter::KEY_2:
				m_switch->setSingleChildOn(1);
				return true;
				break;
			case osgGA::GUIEventAdapter::KEY_3:
				m_switch->setSingleChildOn(2);
				return true;
				break;
			case osgGA::GUIEventAdapter::KEY_Plus:
				m_size *= 2.0;
				m_switch = setupScene((unsigned int)m_size, (unsigned int)m_size, m_maxInstanceMatrices);
				m_viewer->setSceneData(m_switch);
				return true;
				break;
			case osgGA::GUIEventAdapter::KEY_Minus:
				m_size *= 0.5;
				m_switch = setupScene((unsigned int)m_size, (unsigned int)m_size, m_maxInstanceMatrices);
				m_viewer->setSceneData(m_switch);
				return true;
				break;
			default:
				break;
			}

		}

		return false;
	}
private:
	osg::ref_ptr<osg::Switch>		m_switch;
	osg::ref_ptr<osgViewer::Viewer> m_viewer;
	float m_size;
	GLint m_maxInstanceMatrices;
};

GLint getMaxNumberOfUniforms(osg::GraphicsContext* context)
{
	context->realize();
	context->makeCurrent();
	GLint maxNumUniforms = 0;
	glGetIntegerv(GL_MAX_VERTEX_UNIFORM_COMPONENTS, &maxNumUniforms);
	context->releaseContext();

	return maxNumUniforms;
}

osg::ref_ptr<osg::Geometry> createQuads()
{
	// create two quads as geometry
	osg::ref_ptr<osg::Vec3Array>	vertexArray = new osg::Vec3Array;
	vertexArray->push_back(osg::Vec3(-1.0f, 0.0f, 0.0f));
	vertexArray->push_back(osg::Vec3(1.0f, 0.0f, 0.0f));
	vertexArray->push_back(osg::Vec3(-1.0f, 0.0f, 2.0f));
	vertexArray->push_back(osg::Vec3(1.0f, 0.0f, 2.0f));

	vertexArray->push_back(osg::Vec3(0.0f, -1.0f, 0.0f));
	vertexArray->push_back(osg::Vec3(0.0f, 1.0f, 0.0f));
	vertexArray->push_back(osg::Vec3(0.0f, -1.0f, 2.0f));
	vertexArray->push_back(osg::Vec3(0.0f, 1.0f, 2.0f));

	osg::ref_ptr<osg::DrawElementsUByte> primitive = new osg::DrawElementsUByte(GL_TRIANGLES);
	primitive->push_back(0); primitive->push_back(1); primitive->push_back(2);
	primitive->push_back(3); primitive->push_back(2); primitive->push_back(1);
	primitive->push_back(4); primitive->push_back(5); primitive->push_back(6);
	primitive->push_back(7); primitive->push_back(6); primitive->push_back(5);

	osg::ref_ptr<osg::Vec3Array>        normalArray = new osg::Vec3Array;
	normalArray->push_back(osg::Vec3(0.0f, -1.0f, 0.0f));
	normalArray->push_back(osg::Vec3(0.0f, -1.0f, 0.0f));
	normalArray->push_back(osg::Vec3(0.0f, -1.0f, 0.0f));
	normalArray->push_back(osg::Vec3(0.0f, -1.0f, 0.0f));

	normalArray->push_back(osg::Vec3(1.0f, 0.0f, 0.0f));
	normalArray->push_back(osg::Vec3(1.0f, 0.0f, 0.0f));
	normalArray->push_back(osg::Vec3(1.0f, 0.0f, 0.0f));
	normalArray->push_back(osg::Vec3(1.0f, 0.0f, 0.0f));

	osg::ref_ptr<osg::Vec2Array>		texCoords = new osg::Vec2Array;
	texCoords->push_back(osg::Vec2(0.0f, 0.0f));
	texCoords->push_back(osg::Vec2(1.0f, 0.0f));
	texCoords->push_back(osg::Vec2(0.0f, 1.0f));
	texCoords->push_back(osg::Vec2(1.0f, 1.0f));

	texCoords->push_back(osg::Vec2(0.0f, 0.0f));
	texCoords->push_back(osg::Vec2(1.0f, 0.0f));
	texCoords->push_back(osg::Vec2(0.0f, 1.0f));
	texCoords->push_back(osg::Vec2(1.0f, 1.0f));


	osg::ref_ptr<osg::Geometry> geometry = new osg::Geometry;
	geometry->setVertexArray(vertexArray);
	geometry->setNormalArray(normalArray);
	geometry->setNormalBinding(osg::Geometry::BIND_PER_VERTEX);
	geometry->setTexCoordArray(0, texCoords);
	geometry->addPrimitiveSet(primitive);

	return geometry;
}

osg::ref_ptr<osg::Switch> setupScene(unsigned int x, unsigned int y, GLint maxInstanceMatrices)
{
	osg::ref_ptr<osg::Switch>	switchNode = new osg::Switch;

	// create the instanced geometry builder
	osg::ref_ptr<osgExample::InstancedGeometryBuilder> builder = new osgExample::InstancedGeometryBuilder(maxInstanceMatrices);
	builder->setGeometry(createQuads());

	// create some matrices
	srand(time(NULL));
	for (unsigned int i = 0; i < x; ++i)
	{
		for (unsigned int j = 0; j < y; ++j)
		{
			// get random angle and random scale
			double angle = (rand() % 360) / 180.0 * M_PI;
			double scale = (rand() % 10)  + 1.0;
			
			osg::Matrixd modelMatrix =  osg::Matrixd::scale(scale, scale, scale) * osg::Matrixd::rotate(angle, osg::Vec3d(0.0, 0.0, 1.0)) * osg::Matrixd::translate(i*2.0f, j*2.0f, 0.0f);
			builder->addMatrix(modelMatrix);
		}
	}
	
	osg::ref_ptr<osg::Geode> hardwareInstancing = new osg::Geode;
	switchNode->addChild(builder->getSoftwareInstancedNode(), false);
	switchNode->addChild(builder->getHardwareInstancedNode(), false);
	switchNode->addChild(builder->getTextureHardwareInstancedNode(), true);

	// load texture and add it to the quad
	osg::ref_ptr<osg::Image> image = osgDB::readImageFile("data/grass.png");
	osg::ref_ptr<osg::Texture2D> texture = new osg::Texture2D(image);
	texture->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE);
	texture->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE);
	texture->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR_MIPMAP_LINEAR);
	texture->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR_MIPMAP_LINEAR);
	texture->setUseHardwareMipMapGeneration(true);

	switchNode->getOrCreateStateSet()->setTextureAttributeAndModes(0, texture, osg::StateAttribute::ON);
	switchNode->getOrCreateStateSet()->addUniform(new osg::Uniform("colorTexture", 0));
	switchNode->getOrCreateStateSet()->setAttributeAndModes(new osg::AlphaFunc(osg::AlphaFunc::GEQUAL, 0.8f), osg::StateAttribute::ON);

	return switchNode;
}

int main(int argc, char** argv)
{
	osg::ref_ptr<osgViewer::Viewer> viewer = new osgViewer::Viewer;

	viewer->setUpViewInWindow(100, 100, 800, 600);

	// get window and set name
	osgViewer::ViewerBase::Windows windows;
	viewer->getWindows(windows);
	windows[0]->setWindowName("OpenSceneGraph Instancing Example");

	// get context to determine max number of uniforms in vertex shader
	osgViewer::ViewerBase::Contexts contexts;
	viewer->getContexts(contexts);
	GLint maxNumUniforms = getMaxNumberOfUniforms(contexts[0]);

	// we need to reserve some space for modelViewMatrix, projectionMatrix, modelViewProjectionMatrix and normalMatrix, we also need 16 float uniforms per matrix
	unsigned int maxInstanceMatrices = (maxNumUniforms-64) / 16;

	// create scene
	osg::ref_ptr<osg::Switch> scene = setupScene(50, 50, maxInstanceMatrices);
	viewer->setSceneData(scene);

	 // add the state manipulator
    viewer->addEventHandler(new osgGA::StateSetManipulator(viewer->getCamera()->getOrCreateStateSet()));

	// add the stats handler
    viewer->addEventHandler(new osgViewer::StatsHandler);
	viewer->addEventHandler(new SwitchInstancingHandler(viewer, scene, maxInstanceMatrices));

	return viewer->run();
}