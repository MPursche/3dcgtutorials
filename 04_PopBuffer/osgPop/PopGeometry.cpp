#include "PopGeometry.h"
#include "PopDrawElements.h"

#include <osgUtil/CullVisitor>

using namespace osg;

namespace osgPop
{

double log2(double n)  
{  
    return log(n) / log(2);  
}

struct PopCullCallback : osg::Drawable::CullCallback
{
    virtual bool cull(osg::NodeVisitor* nv, osg::Drawable* drawable, osg::RenderInfo* renderInfo) const
    {
        osgUtil::CullVisitor* cv = dynamic_cast<osgUtil::CullVisitor*>(nv);
        PopGeometry* popGeometry = dynamic_cast<PopGeometry*>(drawable);

        if(cv && drawable)
        {
            // calculate error metric
			osg::BoundingSphere bs(popGeometry->getBound().center(), popGeometry->getMaxBounds() - popGeometry->getMinBounds());
			float screenSize = cv->clampedPixelSize(bs);
			float lod = ceilf(log2(screenSize / (popGeometry->getMaxViewSpaceError() * cv->getLODScale()))) - 1.0f;
	        lod = std::max(std::min(lod, 31.0f), 0.0f);

			popGeometry->setLod(lod);
        }
        
        return false;
    }
};



PopGeometry::PopGeometry()
	: Geometry()
	, _lodRange(32)
	, _min(FLT_MIN)
	, _max(FLT_MAX)
	, _numFixedVertices(0)
	, _lodUniform(new osg::Uniform("lod", 32.0f))
	, _minBoundsUniform(new osg::Uniform("minBounds", osg::Vec3(_min, _min, _min)))
	, _maxBoundsUniform(new osg::Uniform("maxBounds", osg::Vec3(_max, _max, _max)))
	, _numFixedVerticesUniform(new osg::Uniform("fixedVertices", _numFixedVertices))
	, _maxViewSpaceError(1.0f) 
{
	setSupportsDisplayList(false);
	setUseDisplayList(false);
	setUseVertexBufferObjects(true);

    setCullCallback(new PopCullCallback());

	_lodUniform->setDataVariance(osg::Object::DYNAMIC);
	getOrCreateStateSet()->addUniform(_lodUniform);
	_stateset->addUniform(_minBoundsUniform);
	_stateset->addUniform(_maxBoundsUniform);
	_stateset->addUniform(_numFixedVerticesUniform);
}

PopGeometry::PopGeometry(const PopGeometry& rhs, const CopyOp& copyop)
	: Geometry(rhs, copyop)
	, _lodRange(rhs._lodRange)
	, _min(rhs._min)
	, _max(rhs._max)
	, _numFixedVertices(rhs._numFixedVertices)
	, _lodUniform(copyop(rhs._lodUniform))
	, _minBoundsUniform(copyop(rhs._minBoundsUniform))
	, _maxBoundsUniform(copyop(rhs._maxBoundsUniform))
	, _numFixedVerticesUniform(copyop(rhs._numFixedVerticesUniform))
	, _maxViewSpaceError(rhs._maxViewSpaceError)
{
	setSupportsDisplayList(false);
	setUseDisplayList(false);
	setUseVertexBufferObjects(true);

	getOrCreateStateSet()->addUniform(_lodUniform);
	_stateset->addUniform(_minBoundsUniform);
	_stateset->addUniform(_maxBoundsUniform);
	_stateset->addUniform(_numFixedVerticesUniform);
}

void PopGeometry::setLod(float lod) const
{
	ref_ptr<PopDrawElements> popDrawElements = dynamic_cast<PopDrawElements*>(_primitives[0].get());
    int _lod = int(lod);

	if ( lod >= 0 && lod < _lodRange.size() && popDrawElements)
	{
		popDrawElements->setEnd(_lodRange[_lod]);
	}

    _lodUniform->set(floor(lod)+1.0f);
	_lodUniform->dirty();
}

void PopGeometry::updateUniforms()
{
	_minBoundsUniform->set(osg::Vec3(_min, _min, _min));
	_maxBoundsUniform->set(osg::Vec3(_max, _max, _max));
	_numFixedVerticesUniform->set(_numFixedVertices);
	_minBoundsUniform->dirty();
	_maxBoundsUniform->dirty();
	_numFixedVerticesUniform->dirty();
}

void PopGeometry::reconnectUniforms()
{
    if (_stateset)
    {
        osg::Uniform* lodUniform = _stateset->getUniform("lod");
        if (lodUniform) { _lodUniform = lodUniform; }
            
        osg::Uniform* minBoundsUniform = _stateset->getUniform("minBounds");
        if (minBoundsUniform) { _minBoundsUniform = minBoundsUniform; }

        osg::Uniform* maxBoundsUniform = _stateset->getUniform("maxBounds");
        if (maxBoundsUniform) { _maxBoundsUniform = maxBoundsUniform; }

        osg::Uniform* numFixedVerticesUniform = _stateset->getUniform("fixedVertices");
        if (numFixedVerticesUniform) { _numFixedVerticesUniform = numFixedVerticesUniform; }
    }
}

} // namespace osg