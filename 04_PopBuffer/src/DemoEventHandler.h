#pragma once

#include "UpdateViewSpaceErrorVisitor.h"
#include <iostream>
#include <osgGA/GUIEventHandler>

namespace osgExample {

class DemoEventHandler : public osgGA::GUIEventHandler
{
public:
    DemoEventHandler(osg::ref_ptr<osg::Node> scene,
                     osg::ref_ptr<osg::Uniform> visualizeLodUniform,
                     osg::ref_ptr<osg::Uniform> texturedUniform)
		:	m_scene(scene)
		,   m_maxViewSpaceError(1.0f)
        ,   m_visualizeLodUniform(visualizeLodUniform)
        ,   m_texturedUniform(texturedUniform)
        ,   m_visualizeLod(false)
        ,   m_textured(true)
	{
	}

	virtual bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa);
private:
	osg::ref_ptr<osg::Node>		m_scene;
    osg::ref_ptr<osg::Uniform>  m_visualizeLodUniform;
    osg::ref_ptr<osg::Uniform>  m_texturedUniform;
	float						m_maxViewSpaceError;
    bool                        m_visualizeLod;
    bool                        m_textured;
};

}