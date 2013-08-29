#ifndef _LIGHT_UNIFORM_UPDATE_CALLBACK_H
#define _LIGHT_UNIFORM_UPDATE_CALLBACK_H

// osg
#include <osg/ref_ptr>
#include <osg/Node>
#include <osgUtil/CullVisitor>
#include <osg/Camera>
#include <osg/Uniform>
#include <osg/Matrix>
#include <osg/Vec3>

namespace osgExample
{

class LightUniformUpdateCallback : public osg::NodeCallback
{
public:
	LightUniformUpdateCallback(osg::ref_ptr<osg::Uniform> lightDirection)
		:	m_lightDirection(lightDirection)
	{
		osg::Vec3 worldLightDirection;
		lightDirection->get(worldLightDirection);
		m_worldLightDirection = osg::Vec4(worldLightDirection, 0.0f);
	}

	virtual void operator()(osg::Node* node, osg::NodeVisitor* nv)
    {
		osg::ref_ptr<osgUtil::CullVisitor> cv = dynamic_cast<osgUtil::CullVisitor*>(nv);
		
		if(cv)
		{
			osg::Matrixd modelViewMatrix = *cv->getModelViewMatrix();
			osg::Vec4 newLightDirection = m_worldLightDirection * modelViewMatrix;

			m_lightDirection->set(osg::Vec3(newLightDirection.x(), newLightDirection.y(), newLightDirection.z()));
		}
    }

private:
	osg::ref_ptr<osg::Uniform> m_lightDirection;
	osg::Vec4				   m_worldLightDirection;
};

} // namespae osgExample

#endif