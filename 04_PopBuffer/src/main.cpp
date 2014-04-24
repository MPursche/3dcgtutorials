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
#include <fstream>
#include <sstream>

#include "LevelOfDetailGeometry.h"
#include "ConvertToLevelOfDetailGeometryVisitor.h"
#include "AddTextureUniformVisitor.h"
#include "DemoEventHandler.h"
#include "KdTreeVisitor.h"

// osg
#include <osg/ref_ptr>
#include <osg/Switch>
#include <osgViewer/Viewer>
#include <osgGA/StateSetManipulator>
#include <osgViewer/ViewerEventHandlers>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <osg/CullFace>

osg::Shader* loadShaderAndAddPrelude(const std::string& fileName, const std::string& uniformDefintion, const std::string& functionDefinition)
{
    osg::Shader* vertexShader = new osg::Shader(osg::Shader::VERTEX);

    std::ifstream stream;
    stream.open(fileName, std::ifstream::in);

    if (!stream.is_open()) { return vertexShader; }

    std::stringstream shaderCode;

    while (!stream.eof())
    {
        std::string line;
        std::getline(stream, line);

        if (line.find("#version") != std::string::npos)
        {
            shaderCode << line << std::endl;
            shaderCode << uniformDefintion << std::endl;
            shaderCode << std::endl;
            shaderCode << functionDefinition << std::endl;
        } else {
            shaderCode << line << std::endl;
        }
    }

    vertexShader->setShaderSource(shaderCode.str());

    return vertexShader;
}

int main(int argc, char** argv)
{
	osg::ArgumentParser arguments(&argc, argv);
	osg::ref_ptr<osgViewer::Viewer> viewer = new osgViewer::Viewer;

	viewer->setUpViewInWindow(100, 100, 800, 600);

	// get window and set name
	osgViewer::ViewerBase::Windows windows;
	viewer->getWindows(windows);
	windows[0]->setWindowName("OpenSceneGraph Popbuffer Example");

	// create scene
    osg::ref_ptr<osg::Switch> scene = new osg::Switch();
    osg::ref_ptr<osg::Node> model = NULL;
    osg::ref_ptr<osg::Node> optimizedModel = NULL;

	if (arguments.argc() > 1)
	{
        model = osgDB::readNodeFile(arguments[1]);
        if (!model) { return -1; }

        scene->addChild(model, true);
                
        std::string outputFile;
		if (arguments.read("--convert", outputFile))
        {
            optimizedModel = dynamic_cast<osg::Node*>(model->clone(osg::CopyOp::DEEP_COPY_ALL)); 
            
            // first split geometry with kd tree
			int maxVertices = 0;
			if (arguments.read("--optimize", maxVertices))
			{
				osgExample::KdTreeVisitor kdVisitor(maxVertices);
				optimizedModel->accept(kdVisitor);
			}

            // convert geometry if requested
            osgUtil::ConvertToLevelOfDetailGeometryVisitor lodVisitor;
	        optimizedModel->accept(lodVisitor);
			osgDB::writeNodeFile(*optimizedModel, outputFile);

            scene->setValue(0, false);
            scene->addChild(optimizedModel, true);
        } else {
            optimizedModel = model;

            scene->setValue(0, false);
            scene->addChild(optimizedModel, true);
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
                
        osgUtil::ConvertToLevelOfDetailGeometryVisitor lodVisitor;
	    geode->accept(lodVisitor);

        scene->addChild(geode, true);
	}	

	osg::BoundingSphere bs = scene->getBound();

    // add overal uniforms
    AddTextureUniformVisitor textureVisitor;
    scene->accept(textureVisitor);   

    osg::Uniform* lightDirection = new osg::Uniform(osg::Uniform::FLOAT_VEC4, "lightDirection", 3);
    lightDirection->setElement(0, osg::Vec4(1.0f, 0.0f, 1.0f, 0.0f));
    lightDirection->setElement(1, osg::Vec4(-1.0f, 0.0f, 1.0f, 0.0f));
    lightDirection->setElement(2, osg::Vec4(0.0f, 1.0f, 1.0f, 0.0f));
    scene->getOrCreateStateSet()->addUniform(lightDirection);

	// add shader
    {
        osg::ref_ptr<osg::Program> program = new osg::Program();
	    osg::ref_ptr<osg::Shader> vertexShader = osgDB::readShaderFile("../shader/normal_geometry.vert");
	    osg::ref_ptr<osg::Shader> fragmentShader = osgDB::readShaderFile("../shader/popbuffer.frag");
	    program->addShader(vertexShader);
	    program->addShader(fragmentShader);
        model->getOrCreateStateSet()->setAttributeAndModes(program, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
    }

    {
	    osg::ref_ptr<osg::Program> program = new osg::Program();
        osg::ref_ptr<osg::Shader> vertexShader = loadShaderAndAddPrelude("../shader/popbuffer.vert",
                                                                         osg::LevelOfDetailGeometry::getVertexShaderUniformDefintion(),
                                                                         osg::LevelOfDetailGeometry::getVertexShaderFunctionDefinition());
        osg::ref_ptr<osg::Shader> fragmentShader = osgDB::readShaderFile("../shader/popbuffer.frag");
	    program->addShader(vertexShader);
	    program->addShader(fragmentShader);
    
        osg::ref_ptr<osg::StateSet> ss = optimizedModel->getOrCreateStateSet();
        ss->setDataVariance(osg::Object::DYNAMIC);
	    ss->setAttributeAndModes(program, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);

        osg::ref_ptr<osg::Uniform> visualizeLodUniform = new osg::Uniform("visualizeLod", false);
        visualizeLodUniform->setDataVariance(osg::Object::DYNAMIC);
        osg::ref_ptr<osg::Uniform> textureActiveUniform = new osg::Uniform("textureActive", false);

        ss->addUniform(visualizeLodUniform);

	    viewer->addEventHandler(new osgGA::StateSetManipulator(viewer->getCamera()->getOrCreateStateSet()));
	    viewer->addEventHandler(new osgViewer::StatsHandler());
        viewer->addEventHandler(new osgExample::DemoEventHandler(scene, visualizeLodUniform, textureActiveUniform));
    }

	viewer->setSceneData(scene);

	return viewer->run();
}