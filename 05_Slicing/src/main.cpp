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
#include <sstream>

// osg
#include <osg/ref_ptr>
#include <osgViewer/Viewer>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <osg/ComputeBoundsVisitor>
#include <osg/Texture2D>
#include <osg/BlendEquation>
#include <osg/BlendFunc>

osg::Geometry* createImposter(osg::Image* colorImage, osg::Image* normalImage, const osg::Vec3d& corner, const osg::Vec3d& left, const osg::Vec3d& up)
{
    osg::Geometry* geometry = osg::createTexturedQuadGeometry(corner, left, up); 

    osg::Texture2D* colorTexture = new osg::Texture2D(static_cast<osg::Image*>(colorImage->clone(osg::CopyOp::DEEP_COPY_ALL)));
    geometry->getOrCreateStateSet()->setTextureAttributeAndModes(0, colorTexture, osg::StateAttribute::ON);
    osg::Texture2D* normalTexture = new osg::Texture2D(static_cast<osg::Image*>(normalImage->clone(osg::CopyOp::DEEP_COPY_ALL)));
    geometry->getOrCreateStateSet()->setTextureAttributeAndModes(1, normalTexture, osg::StateAttribute::ON);

    // create tangent
    osg::Vec3Array* tangents = new osg::Vec3Array();
    osg::Vec3 tangent = left;
    tangent.normalize();
    tangents->push_back(tangent);
    tangents->setBinding(osg::Array::BIND_OVERALL);
    geometry->setVertexAttribArray(5, tangents);

    return geometry;
}

osg::Camera* createPostRenderCamera(osg::Texture2D* colorTexture, osg::Image* outImage)
{
    // create post render camera
    osg::Camera* ppCamera = new osg::Camera();
    ppCamera->setComputeNearFarMode(osg::CullSettings::DO_NOT_COMPUTE_NEAR_FAR);
    ppCamera->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
    ppCamera->setClearMask(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    ppCamera->setClearColor(osg::Vec4(1.0, 1.0, 1.0, 1.0));
    //ppCamera->setRenderTargetImplementation(osg::Camera::FRAME_BUFFER);
    ppCamera->setRenderTargetImplementation(osg::Camera::FRAME_BUFFER_OBJECT);
    ppCamera->setRenderOrder(osg::Camera::POST_RENDER, 1);
    osg::Matrixd viewMatrix = osg::Matrixd::lookAt(osg::Vec3(0.0, 1.0, 0.0), osg::Vec3(0.0, 0.0, 0.0), osg::Vec3(0.0, 0.0, 1.0));
    ppCamera->setViewMatrix(viewMatrix);
    osg::Matrixd projectionMatrix = osg::Matrixd::ortho(1.0, -1.0, -1.0, 1.0, 0.0, 10.0);  
    ppCamera->setProjectionMatrix(projectionMatrix);
    ppCamera->setViewport(0, 0, colorTexture->getTextureWidth(), colorTexture->getTextureHeight());
    ppCamera->attach(osg::Camera::COLOR_BUFFER, outImage);

    // create fullscreen quad
    osg::Geode* geode = new osg::Geode();
    osg::Geometry* geometry = osg::createTexturedQuadGeometry(osg::Vec3(-1.0, 0.0, -1.0), osg::Vec3(2.0, 0.0, 0.0), osg::Vec3(0.0, 0.0, 2.0));
    geode->addDrawable(geometry);
    ppCamera->addChild(geode);

    // add texture and shader
    osg::StateSet* ss = geode->getOrCreateStateSet();
    ss->addUniform(new osg::Uniform("colorTexture", 0));
    ss->setTextureAttributeAndModes(0, colorTexture, osg::StateAttribute::ON);
    
    osg::Program* program = new osg::Program;
    osg::Shader* vertexShader = osgDB::readShaderFile(osg::Shader::VERTEX, "../shader/dilate.vert");
    osg::Shader* fragmentShader = osgDB::readShaderFile(osg::Shader::FRAGMENT, "../shader/dilate.frag");
    program->addShader(vertexShader);
    program->addShader(fragmentShader);
    ss->setAttributeAndModes(program, osg::StateAttribute::ON);

    return ppCamera;
}

int main(int argc, char** argv)
{
	int width = 1024, height = 1024, depth = 16;
    
    osg::ArgumentParser arguments(&argc, argv);
	osg::ref_ptr<osgViewer::Viewer> viewer = new osgViewer::Viewer;

	viewer->setUpViewInWindow(100, 100, width, height);

	// get window and set name
	osgViewer::ViewerBase::Windows windows;
	viewer->getWindows(windows);
	windows[0]->setWindowName("OpenSceneGraph Slicing Example");

	// create scene
    osg::ref_ptr<osg::Group> scene = new osg::Group();
    osg::ref_ptr<osg::Node> model = osgDB::readNodeFile("../data/cow.osg");
    scene->addChild(model);

    osg::ComputeBoundsVisitor boundsVisitor;
    model->accept(boundsVisitor);
    osg::BoundingBox bb = boundsVisitor.getBoundingBox();

    float halfModelWidth = (bb.xMax() - bb.xMin()) / 2;
    float halfModelHeight = (bb.zMax() - bb.zMin()) / 2;
    float sliceRange = (bb.yMax() - bb.yMin()) / depth;
    osg::Vec3 center = bb.center();
    osg::Vec3 eye = osg::Vec3(center.x(), bb.yMax(), center.z());       
	viewer->setSceneData(scene);

    // get the default camera
    osg::Camera* camera = viewer->getCamera();
    camera->setComputeNearFarMode(osg::CullSettings::DO_NOT_COMPUTE_NEAR_FAR);
    osg::Matrix viewMatrix = osg::Matrixd::lookAt(eye, center, osg::Vec3(0.0, 0.0, 1.0));
    camera->setViewMatrix(viewMatrix);
    camera->setRenderOrder(osg::Camera::POST_RENDER, 0);
    camera->setRenderTargetImplementation(osg::Camera::FRAME_BUFFER_OBJECT);
    camera->setClearColor(osg::Vec4(0.0, 0.0, 0.0, 0.0));


    // create image for current view
    osg::ref_ptr<osg::Image> colorImage = new osg::Image();
    colorImage->allocateImage(width, height, 1, GL_RGBA, GL_UNSIGNED_BYTE);
    
    osg::ref_ptr<osg::Texture2D> colorTexture = new osg::Texture2D();
    colorTexture->setTextureSize(width, height);
	colorTexture->setInternalFormat(GL_RGBA8);
	colorTexture->setSourceType(GL_UNSIGNED_BYTE);
	colorTexture->setSourceFormat(GL_RGBA);

    osg::ref_ptr<osg::Image> normalImage = new osg::Image();
    normalImage->allocateImage(width, height, 1, GL_RGBA, GL_UNSIGNED_BYTE);
    osg::ref_ptr<osg::Image> depthImage = new osg::Image();
    depthImage->allocateImage(width, height, 1, GL_RGBA, GL_UNSIGNED_BYTE);
    
    camera->attach(osg::Camera::COLOR_BUFFER0, colorTexture);
    camera->attach(osg::Camera::COLOR_BUFFER1, normalImage);
    camera->attach(osg::Camera::COLOR_BUFFER2, depthImage);

    // create post render camera
    osg::ref_ptr<osg::Camera> ppCamera = createPostRenderCamera(colorTexture, colorImage);
    scene->addChild(ppCamera);

    // create program
    osg::Program* program = new osg::Program;
    osg::Shader* vertexShader = osgDB::readShaderFile(osg::Shader::VERTEX, "../shader/simple.vert");
    osg::Shader* fragmentShader = osgDB::readShaderFile(osg::Shader::FRAGMENT, "../shader/simple.frag");
    program->addShader(vertexShader);
    program->addShader(fragmentShader);
    program->addBindFragDataLocation("outColor", 0);
    program->addBindFragDataLocation("outNormal", 1);
    program->addBindFragDataLocation("outDepth", 2);
    
    model->getOrCreateStateSet()->setAttributeAndModes(program, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
    osg::ref_ptr<osg::Uniform> nearUniform = new osg::Uniform("nearPlane", 0.0f);
    osg::ref_ptr<osg::Uniform> farUniform = new osg::Uniform("farPlane", sliceRange * depth);
    model->getOrCreateStateSet()->addUniform(nearUniform);
    model->getOrCreateStateSet()->addUniform(farUniform);

    // create geode for the imposter geometries
    osg::ref_ptr<osg::Geode> geode = new osg::Geode();
    osg::StateSet* ss = geode->getOrCreateStateSet();
    ss->setAttributeAndModes(new osg::BlendEquation(osg::BlendEquation::FUNC_ADD), osg::StateAttribute::ON);
    ss->setAttributeAndModes(new osg::BlendFunc(osg::BlendFunc::SRC_ALPHA, osg::BlendFunc::ONE_MINUS_SRC_ALPHA), osg::StateAttribute::ON);
    ss->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
    ss->setMode(GL_DEPTH_TEST, osg::StateAttribute::OFF);
    {
        osg::Program* program = new osg::Program;
        osg::Shader* vertexShader = osgDB::readShaderFile(osg::Shader::VERTEX, "../shader/imposter.vert");
        osg::Shader* fragmentShader = osgDB::readShaderFile(osg::Shader::FRAGMENT, "../shader/imposter.frag");
        program->addShader(vertexShader);
        program->addShader(fragmentShader);
        program->addBindAttribLocation("vTangent", 5);
        ss->setAttributeAndModes(program,  osg::StateAttribute::ON);
        ss->addUniform(new osg::Uniform("colorTexture", 0));
        ss->addUniform(new osg::Uniform("normalTexture", 1));
    }

    viewer->realize();

    // create slices in y direction
    for (int i = 0; i < depth; ++i)
    {
        // setup ortho projection matrix
        osg::Matrixd projectionMatrix = osg::Matrixd::ortho(halfModelWidth, -halfModelWidth, -halfModelHeight, halfModelHeight, i  * sliceRange, (i+1) * sliceRange);       
        camera->setProjectionMatrix(projectionMatrix);
        
        viewer->frame();
        viewer->frame();

        std::stringstream fileName, normalFileName, depthFileName;
        fileName <<  "../data/color_" << i << ".png";
        normalFileName <<  "../data/normals_" << i << ".png";
        depthFileName <<  "../data/depth_" << i << ".png";

        osgDB::writeImageFile(*colorImage,fileName.str());
        osgDB::writeImageFile(*normalImage,normalFileName.str());
        osgDB::writeImageFile(*depthImage,depthFileName.str());

       geode->addDrawable(createImposter(colorImage, normalImage, 
                                         osg::Vec3(center.x() - halfModelWidth, (i+0.5f) * sliceRange + bb.yMin(), center.z() - halfModelHeight),
                                         osg::Vec3(2.0f * halfModelWidth, 0.0f, 0.0f),
                                         osg::Vec3(0.0f, 0.0f, 2.0f * halfModelHeight)));
    }

    // create slices in z direction
    halfModelWidth = (bb.xMax() - bb.xMin()) / 2;
    halfModelHeight = (bb.yMax() - bb.yMin()) / 2;
    sliceRange = (bb.zMax() - bb.zMin()) / depth;
    center = bb.center();
    eye = osg::Vec3(center.x(), center.y(), bb.zMin());    
    viewMatrix = osg::Matrixd::lookAt(eye, center, osg::Vec3(0.0, -1.0, 0.0));
    viewer->getCamera()->setViewMatrix(viewMatrix);


    for (int i = 0; i < depth; ++i)
    {
        // setup ortho projection matrix
        osg::Matrixd projectionMatrix = osg::Matrixd::ortho(halfModelWidth, -halfModelWidth, -halfModelHeight, halfModelHeight, i  * sliceRange, (i+1) * sliceRange);       
        camera->setProjectionMatrix(projectionMatrix);
        
        viewer->frame();
        viewer->frame();

        std::stringstream fileName, normalFileName, depthFileName;
        fileName <<  "../data/top_color_" << i << ".png";
        normalFileName <<  "../data/top_normals_" << i << ".png";
        depthFileName <<  "../data/top_depth_" << i << ".png";

        osgDB::writeImageFile(*colorImage,fileName.str());
        osgDB::writeImageFile(*normalImage,normalFileName.str());
        osgDB::writeImageFile(*depthImage,depthFileName.str());

        geode->addDrawable(createImposter(colorImage, normalImage, 
                                          osg::Vec3(center.x() + halfModelWidth, center.y() - halfModelHeight, (i + 0.5f) * sliceRange - bb.zMax()),
                                          osg::Vec3(-2.0f * halfModelWidth, 0.0f, 0.0f), 
                                          osg::Vec3(0.0f, 2.0f * halfModelHeight, 0.0f)));
    }

    osgDB::writeNodeFile(*geode, "../data/out.osgb");

	return 0;
}