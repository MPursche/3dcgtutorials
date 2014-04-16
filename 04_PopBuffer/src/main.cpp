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

// std
#define _USE_MATH_DEFINES
#include <cmath>

#include "PopGeometry.h"
#include "ConvertToPopGeometryVisitor.h"
#include "AddTextureUniformVisitor.h"
#include "DemoEventHandler.h"
#include "KdTreeVisitor.h"

// osg
#include <osg/ref_ptr>
#include <osgViewer/Viewer>
#include <osgGA/StateSetManipulator>
#include <osgViewer/ViewerEventHandlers>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <osg/CullFace>

int main(int argc, char** argv)
{
	osg::ref_ptr<osgViewer::Viewer> viewer = new osgViewer::Viewer;

	viewer->setUpViewInWindow(100, 100, 800, 600);

	// get window and set name
	osgViewer::ViewerBase::Windows windows;
	viewer->getWindows(windows);
	windows[0]->setWindowName("OpenSceneGraph Popbuffer Example");

	// create scene
	osg::ref_ptr<osg::Node> scene = NULL;
	if (argc > 1)
	{
		scene = osgDB::readNodeFile(std::string(argv[1]));
        if (!scene) { return -1; }
                
        if (argc > 3 && (strcmp(argv[2], "--convert") == 0))
        {
            // first split geometry with kd tree
            osgExample::KdTreeVisitor kdVisitor(USHRT_MAX);
            scene->accept(kdVisitor);

            // convert geometry if requested
            osgPop::ConvertToPopGeometryVisitor popVisitor;
	        scene->accept(popVisitor);
            osgDB::writeNodeFile(*scene, std::string(argv[3]));
        }
	}
	else
	{
		//scene = osgDB::readNodeFile("../data/trex.osgb");                
        //if (!scene) { return -1; }

        // top left quad
        osg::Vec3Array* vertices = new osg::Vec3Array();
        vertices->push_back(osg::Vec3(-10.0f, 0.0f, 0.0f));
        vertices->push_back(osg::Vec3(0.0f, 0.0f, 0.0f));
        vertices->push_back(osg::Vec3(-10.0f, -10.0f, 0.0f));
        vertices->push_back(osg::Vec3(-10.0f, -10.0f, 0.0f));
        vertices->push_back(osg::Vec3(0.0f, 0.0f, 0.0f));
        vertices->push_back(osg::Vec3(0.0f, -10.0f, 0.0f));

        // top right quad
        vertices->push_back(osg::Vec3(0.0f, 0.0f, 0.0f));
        vertices->push_back(osg::Vec3(10.0f, 0.0f, 0.0f));
        vertices->push_back(osg::Vec3(0.0f, -10.0f, 0.0f));
        vertices->push_back(osg::Vec3(0.0f, -10.0f, 0.0f));
        vertices->push_back(osg::Vec3(10.0f, 0.0f, 0.0f));
        vertices->push_back(osg::Vec3(10.0f, -10.0f, 0.0f));

        // bottom left quad
        vertices->push_back(osg::Vec3(-10.0f, 10.0f, 0.0f));
        vertices->push_back(osg::Vec3(0.0f, 10.0f, 0.0f));
        vertices->push_back(osg::Vec3(-10.0f, 0.0f, 0.0f));
        vertices->push_back(osg::Vec3(-10.0f, 0.0f, 0.0f));
        vertices->push_back(osg::Vec3(0.0f, 10.0f, 0.0f));
        vertices->push_back(osg::Vec3(0.0f, 0.0f, 0.0f));

        // bottom right quad
        vertices->push_back(osg::Vec3(0.0f, 10.0f, 0.0f));
        vertices->push_back(osg::Vec3(10.0f, 10.0f, 0.0f));
        vertices->push_back(osg::Vec3(0.0f, 0.0f, 0.0f));
        vertices->push_back(osg::Vec3(0.0f, 0.0f, 0.0f));
        vertices->push_back(osg::Vec3(10.0f, 10.0f, 0.0f));
        vertices->push_back(osg::Vec3(10.0f, 0.0f, 0.0f));

        osg::Vec3Array* normals = new osg::Vec3Array();
        for (int i = 0; i < 24; ++i)
        {
            normals->push_back(osg::Vec3(0.0f, 0.0f, 1.0f));
        }

        osg::Geometry* geometry = new osg::Geometry();
        geometry->setVertexArray(vertices);
        geometry->setNormalArray(normals);
        geometry->setNormalBinding(osg::Geometry::BIND_PER_VERTEX);
        geometry->addPrimitiveSet(new osg::DrawArrays(GL_TRIANGLES, 0, 24));
        
        osg::Geode* geode = new osg::Geode();
        geode->addDrawable(geometry);
        scene = geode;

        osgPop::ConvertToPopGeometryVisitor popVisitor;
	    scene->accept(popVisitor);
	}

	

	osg::BoundingSphere bs = scene->getBound();

	// add shader
	osg::ref_ptr<osg::Program> program = new osg::Program();
	osg::ref_ptr<osg::Shader> vertexShader = osgDB::readShaderFile("../shader/popbuffer.vert");
	osg::ref_ptr<osg::Shader> fragmentShader = osgDB::readShaderFile("../shader/popbuffer.frag");
	program->addShader(vertexShader);
	program->addShader(fragmentShader);
    
	osg::ref_ptr<osg::StateSet> ss = scene->getOrCreateStateSet();
    ss->setDataVariance(osg::Object::DYNAMIC);
	ss->setAttributeAndModes(program, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
	ss->addUniform(new osg::Uniform("lightPosition", osg::Vec4(1.0f, 0.0f, 1.0f, 0.0f)));
    osg::ref_ptr<osg::Uniform> visualizeLodUniform = new osg::Uniform("visualizeLod", false);
    visualizeLodUniform->setDataVariance(osg::Object::DYNAMIC);
    osg::ref_ptr<osg::Uniform> textureActiveUniform = new osg::Uniform("textureActive", false);

    ss->addUniform(visualizeLodUniform);
	AddTextureUniformVisitor textureVisitor;
    scene->accept(textureVisitor);

	viewer->addEventHandler(new osgGA::StateSetManipulator(viewer->getCamera()->getOrCreateStateSet()));
	viewer->addEventHandler(new osgViewer::StatsHandler());
    viewer->addEventHandler(new osgExample::DemoEventHandler(scene, visualizeLodUniform, textureActiveUniform));

	viewer->setSceneData(scene);

	return viewer->run();
}