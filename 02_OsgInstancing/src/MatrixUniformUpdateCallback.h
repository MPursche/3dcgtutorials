#ifndef _MATRIX_UNIFORM_UPDATE_CALLBACK_H
#define _MATRIX_UNIFORM_UPDATE_CALLBACK_H

// osg
#include <osg/ref_ptr>
#include <osg/Node>
#include <osgUtil/CullVisitor>
#include <osg/Uniform>
#include <osg/Matrix>

namespace osgExample
{

class MatrixUniformUpdateCallback : public osg::NodeCallback
{
public:
	MatrixUniformUpdateCallback()
	{
		m_modelViewProjectMatrix = new osg::Uniform("osg_ModelViewProjectionMatrix", osg::Matrixf());
		m_normalMatrix			 = new osg::Uniform("osg_NormalMatrix", osg::Matrix3());
	}

	virtual void operator()(osg::Node* node, osg::NodeVisitor* nv)
    {
		osg::ref_ptr<osgUtil::CullVisitor> cv = dynamic_cast<osgUtil::CullVisitor*>(nv);
		
		if(cv)
		{
			osg::Matrixd projectionMatrix = *cv->getProjectionMatrix();
			osg::Matrixd modelViewMatrix  = *cv->getModelViewMatrix();
			osg::Matrixf modelViewProjectionMatrix = modelViewMatrix * projectionMatrix;
			osg::Matrix3 normalMatrix(modelViewMatrix(0, 0), modelViewMatrix(0, 1), modelViewMatrix(0, 2),
									  modelViewMatrix(1, 0), modelViewMatrix(1, 1), modelViewMatrix(1, 2),
									  modelViewMatrix(2, 0), modelViewMatrix(2, 1), modelViewMatrix(2, 2));

			m_modelViewProjectMatrix->set(modelViewProjectionMatrix);
			m_normalMatrix->set(normalMatrix);
		}
    }

	inline osg::ref_ptr<osg::Uniform> getModelViewProjectionMatrixUniform() const { return m_modelViewProjectMatrix; }
	inline osg::ref_ptr<osg::Uniform> getNormalMatrixUniform() const { return m_normalMatrix; }

private:
	osg::ref_ptr<osg::Uniform> m_modelViewProjectMatrix;
	osg::ref_ptr<osg::Uniform> m_normalMatrix;
	osg::Vec4				   m_worldLightDirection;
};

} // namespae osgExample

#endif