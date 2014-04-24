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

#include "FloatBufferShadowedScene.h"
#include "DepthBufferShadowedScene.h"
#include "SwitchTechniqueHandler.h"

// osg
#include <osg/ref_ptr>
#include <osg/Switch>
#include <osgGA/StateSetManipulator>
#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>
#include <osg/NodeCallback>

class UpdateLight : public osg::NodeCallback
{
public:
    UpdateLight()
		: osg::NodeCallback()
	{
		osg::Vec3f lightDirection(0.0f, 0.8f, 1.0f);
		lightDirection.normalize();
		lightPosition = osg::Vec4(lightDirection, 0.0f);
		lastUpdateTime = 0.0;
	}

	void operator()(osg::Node* node, osg::NodeVisitor* nv)
    {
		osg::ref_ptr<ShadowedScene> shadowedScene = dynamic_cast<ShadowedScene*>(node);

		if(shadowedScene)
		{
			double elapsedTime = nv->getFrameStamp()->getSimulationTime() - lastUpdateTime;
			osg::Matrixd rotationMatrix = osg::Matrixd::rotate(elapsedTime * 0.05f * M_PI, osg::Vec3(0.0, 0.0, 1.0));
			lightPosition = lightPosition * rotationMatrix;
			shadowedScene->setLightPosition(lightPosition);
			lastUpdateTime = nv->getFrameStamp()->getSimulationTime();
		}

        traverse(node,nv);
    }
private:
	osg::Vec4 lightPosition;
	double lastUpdateTime;
};

int main(int argc, char** argv)
{
	osg::ref_ptr<osgViewer::Viewer> viewer = new osgViewer::Viewer;

	viewer->setUpViewInWindow(100, 100, 800, 600);

	// get window and set name
	osgViewer::ViewerBase::Windows windows;
	viewer->getWindows(windows);
	windows[0]->setWindowName("OpenSceneGraph Shadow Mapping Example");

	// create scene
	osg::ref_ptr<osg::Switch> scene = new osg::Switch;
	osg::ref_ptr<ShadowedScene> depthBufferScene = new DepthBufferShadowedScene;
	depthBufferScene->initialize();
	scene->addChild(depthBufferScene, false);
	osg::ref_ptr<ShadowedScene> floatBufferScene = new FloatBufferShadowedScene;
	floatBufferScene->initialize();
	scene->addChild(floatBufferScene, true);

	depthBufferScene->addUpdateCallback(new UpdateLight);
	floatBufferScene->addUpdateCallback(new UpdateLight);

	viewer->setSceneData(scene);

	// add the state manipulator
    viewer->addEventHandler(new osgGA::StateSetManipulator(viewer->getCamera()->getOrCreateStateSet()));
	viewer->addEventHandler(new osgExample::SwitchTechniqueHandler(scene));

	// add the stats handler
    viewer->addEventHandler(new osgViewer::StatsHandler);

	return viewer->run();
}