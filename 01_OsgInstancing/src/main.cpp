// osg
#include <osg/ref_ptr>
#include <osg/Switch>
#include <osg/Group>
#include <osg/Geode>
#include <osg/Drawable>
#include <osg/ShapeDrawable>
#include <osgGA/StateSetManipulator>
#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>

GLint getMaxNumberOfUniforms(osg::GraphicsContext* context)
{
	context->realize();
	context->makeCurrent();
	GLint maxNumUniforms = 0;
	glGetIntegerv(GL_MAX_VERTEX_UNIFORM_COMPONENTS, &maxNumUniforms);
	context->releaseContext();

	return maxNumUniforms;
}

osg::ref_ptr<osg::Switch> setupScene()
{
	osg::ref_ptr<osg::Switch> switchNode = new osg::Switch;

	osg::ref_ptr<osg::Geode> softwareInstancing = new osg::Geode;
	osg::ref_ptr<osg::Geode> hardwareInstancing = new osg::Geode;
	switchNode->addChild(softwareInstancing, false);
	switchNode->addChild(hardwareInstancing, true);

	softwareInstancing->addDrawable(new osg::ShapeDrawable(new osg::Box()));
	hardwareInstancing->addDrawable(new osg::ShapeDrawable(new osg::Box()));

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

	// create scene
	viewer->setSceneData(setupScene());

	 // add the state manipulator
    viewer->addEventHandler(new osgGA::StateSetManipulator(viewer->getCamera()->getOrCreateStateSet()));

	// add the stats handler
    viewer->addEventHandler(new osgViewer::StatsHandler);

	// get context to determine max number of uniforms in vertex shader
	osgViewer::ViewerBase::Contexts contexts;
	viewer->getContexts(contexts);
	GLint maxNumUniforms = getMaxNumberOfUniforms(contexts[0]);
	contexts[0]->getState()->setUseModelViewAndProjectionUniforms(true);

	// we need to reserve some space for modelViewMatrix, projectionMatrix, modelViewProjectionMatrix and normalMatrix, we also need 16 float uniforms per matrix
	unsigned int maxInstanceMatrices = (maxNumUniforms-64) / 16;

	return viewer->run();
}