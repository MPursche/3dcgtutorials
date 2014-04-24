#include "DemoEventHandler.h"

#include <osg/Switch>

namespace osgExample {

bool DemoEventHandler::handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
{
	if (ea.getEventType() == osgGA::GUIEventAdapter::KEYDOWN)
	{
		switch(ea.getKey())
		{
		case osgGA::GUIEventAdapter::KEY_Down:
		{
			m_maxViewSpaceError -= 0.5f;
			m_maxViewSpaceError = std::max(0.5f, std::min(m_maxViewSpaceError, 100.0f));
			osgExample::UpdateViewSpaceError visitor(m_maxViewSpaceError);
			m_scene->accept(visitor);
			std::cout << "Changed maximum viewspace error to: " << m_maxViewSpaceError << std::endl;
			return true;
		} break;
		case osgGA::GUIEventAdapter::KEY_Up:
		{
			m_maxViewSpaceError += 0.5f;
			m_maxViewSpaceError = std::max(0.5f, std::min(m_maxViewSpaceError, 100.0f));
			osgExample::UpdateViewSpaceError visitor(m_maxViewSpaceError);
			m_scene->accept(visitor);
			std::cout << "Changed maximum viewspace error to: " << m_maxViewSpaceError << std::endl;
			return true;
		} break;
        case osgGA::GUIEventAdapter::KEY_T:
        {
            m_textured = !m_textured;
            if (m_textured)
            {
                m_scene->getOrCreateStateSet()->removeUniform(m_texturedUniform);
            } else {
                m_scene->getOrCreateStateSet()->addUniform(m_texturedUniform, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
            }
        } break;
        case osgGA::GUIEventAdapter::KEY_L:
            m_visualizeLod = !m_visualizeLod;
            m_visualizeLodUniform->set(m_visualizeLod);
            break;
        case osgGA::GUIEventAdapter::KEY_1:
            m_scene->setAllChildrenOff();
            m_scene->setValue(0, true);
            break;
        case osgGA::GUIEventAdapter::KEY_2:
            m_scene->setAllChildrenOff();
            m_scene->setValue(1, true);
            break;
		default:
			break;
		}

	}

	return false;
}

}