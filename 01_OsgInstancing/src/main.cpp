// c-std
#include <ctime>
#include <cstdlib>
#define _USE_MATH_DEFINES 1 // fix for visual studio
#include <cmath>
#include <iostream>

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
#include <osg/Light>
#include <osg/LightSource>

// osgExample
#include "InstancedGeometryBuilder.h"
#include "SwitchTechniqueHandler.h"

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

	osg::ref_ptr<osg::StateSet> stateSet = switchNode->getOrCreateStateSet();
	stateSet->setTextureAttributeAndModes(0, texture, osg::StateAttribute::ON);
	stateSet->addUniform(new osg::Uniform("colorTexture", 0));
	stateSet->setAttributeAndModes(new osg::AlphaFunc(osg::AlphaFunc::GEQUAL, 0.8f), osg::StateAttribute::ON);

	// add light source
	osg::ref_ptr<osg::Light> light = new osg::Light(0);
	light->setAmbient(osg::Vec4(0.2f, 0.2f, 0.2f, 1.0f));
	light->setDiffuse(osg::Vec4(0.8f, 0.8f, 0.2f, 1.0f));
	light->setPosition(osg::Vec4(-1.0f, -1.0f, -1.0f, 0.0f));
	
	osg::ref_ptr<osg::LightSource> lightSource = new osg::LightSource;
	lightSource->setLight(light);

	switchNode->addChild(lightSource);
	
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
	osg::ref_ptr<osg::Switch> scene = setupScene(64, 64, maxInstanceMatrices);
	viewer->setSceneData(scene);

	 // add the state manipulator
    viewer->addEventHandler(new osgGA::StateSetManipulator(viewer->getCamera()->getOrCreateStateSet()));

	// add the stats handler
    viewer->addEventHandler(new osgViewer::StatsHandler);
	viewer->addEventHandler(new osgExample::SwitchInstancingHandler(viewer, scene, maxInstanceMatrices, setupScene));

	// print usage
	std::cout << "OpenSceneraph Instancing Example" << std::endl;
	std::cout << "================================" << std::endl << std::endl;
	std::cout << "Switch between instancing techniques: 1, 2, 3" << std::endl;
	std::cout << "Increase/decrease scene complexety: +/-" << std::endl;
	std::cout << "Cycle through different status informations(FPS and other stats): s" << std::endl;

	return viewer->run();
}