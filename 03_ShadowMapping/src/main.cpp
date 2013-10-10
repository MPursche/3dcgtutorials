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

osg::ref_ptr<osg::Geometry> createQuads()
{
	// create a quads as geometry
	osg::ref_ptr<osg::Vec3Array>	vertexArray = new osg::Vec3Array;
	vertexArray->push_back(osg::Vec3(-20.0f, -20.0f, -3.0f));
	vertexArray->push_back(osg::Vec3(20.0f, -20.0f, -3.0f));
	vertexArray->push_back(osg::Vec3(-20.0f, 20.0f, -3.0f));
	vertexArray->push_back(osg::Vec3(20.0f, 20.0f, -3.0f));

	osg::ref_ptr<osg::DrawElementsUByte> primitive = new osg::DrawElementsUByte(GL_TRIANGLES);
	primitive->push_back(0); primitive->push_back(1); primitive->push_back(2);
	primitive->push_back(3); primitive->push_back(2); primitive->push_back(1);

	osg::ref_ptr<osg::Vec3Array>        normalArray = new osg::Vec3Array;
	normalArray->push_back(osg::Vec3(0.0f, 0.0f, 1.0f));
	normalArray->push_back(osg::Vec3(0.0f, 0.0f, 1.0f));
	normalArray->push_back(osg::Vec3(0.0f, 0.0f, 1.0f));
	normalArray->push_back(osg::Vec3(0.0f, 0.0f, 1.0f));

	osg::ref_ptr<osg::Vec2Array>		texCoords = new osg::Vec2Array;
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

int main(int argc, char** argv)
{
	osg::ref_ptr<osgViewer::Viewer> viewer = new osgViewer::Viewer;

	viewer->setUpViewInWindow(100, 100, 800, 600);

	// get window and set name
	osgViewer::ViewerBase::Windows windows;
	viewer->getWindows(windows);
	windows[0]->setWindowName("OpenSceneGraph Shadow Mapping Example");

	// create scene
	osg::ref_ptr<osg::Group> scene = new osg::Group;

	// load cow
	osg::ref_ptr<osg::Node> cow = osgDB::readNodeFile("../data/cow.osg");
	osg::BoundingSphere bs = cow->getBound();

	// create floating point texture
	osg::ref_ptr<osg::Texture2D> depthTexture = new osg::Texture2D;
	depthTexture->setTextureSize(1024, 1024);
	depthTexture->setInternalFormat(GL_R32F);
	depthTexture->setSourceType(GL_FLOAT);
	depthTexture->setSourceFormat(GL_RED);
	depthTexture->setFilter(osg::Texture2D::MIN_FILTER, osg::Texture2D::LINEAR);
	depthTexture->setFilter(osg::Texture2D::MAG_FILTER, osg::Texture2D::LINEAR);
	depthTexture->setWrap(osg::Texture2D::WRAP_S, osg::Texture2D::CLAMP);
	depthTexture->setWrap(osg::Texture2D::WRAP_T, osg::Texture2D::CLAMP);

	// create a directional light source
	osg::Vec3f lightDirection(0.0f, 0.8f, 1.0f);
	lightDirection.normalize();

	// create render to texture camera
	osg::ref_ptr<osg::Camera> shadowPassCamera = new osg::Camera;
	shadowPassCamera->setClearMask(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	shadowPassCamera->setClearColor(osg::Vec4f(1000.0f, 0.0f, 0.0f, 1.0f));
	shadowPassCamera->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
	shadowPassCamera->setRenderTargetImplementation(osg::Camera::FRAME_BUFFER_OBJECT);
	shadowPassCamera->setRenderOrder(osg::Camera::PRE_RENDER);
	shadowPassCamera->setViewport(0, 0, 1024, 1024);
	osg::Matrixd projectionMatrix = osg::Matrixd::ortho(-bs.radius(), bs.radius(), -bs.radius(), bs.radius(), 1.0, 1000.0);
	shadowPassCamera->setProjectionMatrix(projectionMatrix);
	osg::Vec3f eye = bs.center() + (lightDirection * bs.radius());
	osg::Matrixd viewMatrix = osg::Matrixd::lookAt(eye, bs.center(), osg::Vec3f(0.0f, 0.0f, 1.0f));
	shadowPassCamera->setViewMatrix(viewMatrix);
	shadowPassCamera->attach(osg::Camera::COLOR_BUFFER, depthTexture);
	shadowPassCamera->addChild(cow);
	shadowPassCamera->setComputeNearFarMode(osg::CullSettings::DO_NOT_COMPUTE_NEAR_FAR);

	osg::Matrixd shadowMatrix = viewMatrix * projectionMatrix * osg::Matrixd::translate(1.0, 1.0, 1.0) * osg::Matrixd::scale(0.5, 0.5, 0.5);

	// add shadow pass shader
	osg::ref_ptr<osg::Shader> vsShader = osgDB::readShaderFile("../shader/shadow_pass.vert");
	osg::ref_ptr<osg::Shader> fsShader = osgDB::readShaderFile("../shader/shadow_pass.frag");
	osg::ref_ptr<osg::Program> program = new osg::Program;
	program->addShader(vsShader);
	program->addShader(fsShader);
	shadowPassCamera->getOrCreateStateSet()->setAttributeAndModes(program, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
	
	osg::ref_ptr<osg::Geode> geode = new osg::Geode;
	geode->addDrawable(createQuads());

	// add main pass shader
	osg::ref_ptr<osg::Shader> vsShader_untextured = osgDB::readShaderFile("../shader/main_pass_untextured.vert");
	osg::ref_ptr<osg::Shader> fsShader_untextured = osgDB::readShaderFile("../shader/main_pass_untextured.frag");
	osg::ref_ptr<osg::Program> program_untextured = new osg::Program;
	program_untextured->addShader(vsShader_untextured);
	program_untextured->addShader(fsShader_untextured);

	geode->getOrCreateStateSet()->setAttributeAndModes(program_untextured, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
	geode->getOrCreateStateSet()->setTextureAttributeAndModes(0, depthTexture);
	geode->getOrCreateStateSet()->addUniform(new osg::Uniform("depthTexture", 0));
	geode->getOrCreateStateSet()->addUniform(new osg::Uniform("shadowMatrix", osg::Matrixf(shadowMatrix)));
	geode->getOrCreateStateSet()->addUniform(new osg::Uniform("shadowViewMatrix", osg::Matrixf(viewMatrix)));

	// add main pass shader
	osg::ref_ptr<osg::Shader> vsShader_main = osgDB::readShaderFile("../shader/main_pass.vert");
	osg::ref_ptr<osg::Shader> fsShader_main = osgDB::readShaderFile("../shader/main_pass.frag");
	osg::ref_ptr<osg::Program> program_main = new osg::Program;
	program_main->addShader(vsShader_main);
	program_main->addShader(fsShader_main);

	cow->getOrCreateStateSet()->setAttributeAndModes(program_main, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
	cow->getOrCreateStateSet()->setTextureAttributeAndModes(1, depthTexture);
	cow->getOrCreateStateSet()->addUniform(new osg::Uniform("colorTexture", 0));
	cow->getOrCreateStateSet()->addUniform(new osg::Uniform("depthTexture", 1));
	cow->getOrCreateStateSet()->addUniform(new osg::Uniform("shadowMatrix", osg::Matrixf(shadowMatrix)));
	cow->getOrCreateStateSet()->addUniform(new osg::Uniform("shadowViewMatrix", osg::Matrixf(viewMatrix)));
	
	// finalize scene
	scene->addChild(geode);
	scene->addChild(cow);
	scene->addChild(shadowPassCamera);
	scene->getOrCreateStateSet()->addUniform(new osg::Uniform("lightDir", lightDirection));

	viewer->setSceneData(scene);

	// add the state manipulator
    viewer->addEventHandler(new osgGA::StateSetManipulator(viewer->getCamera()->getOrCreateStateSet()));

	// add the stats handler
    viewer->addEventHandler(new osgViewer::StatsHandler);

	return viewer->run();
}