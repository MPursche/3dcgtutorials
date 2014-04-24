#include "LevelOfDetailGeometry.h"
#include "LevelOfDetailDrawElements.h"

#include <osgUtil/CullVisitor>

using namespace osg;

namespace osg
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
        LevelOfDetailGeometry* lodGeometry = dynamic_cast<LevelOfDetailGeometry*>(drawable);

		if(cv && lodGeometry)
        {
            // calculate error metric
			osg::BoundingSphere bs(lodGeometry->getBound().center(), lodGeometry->getMaxBounds() - lodGeometry->getMinBounds());
			float screenSize = cv->clampedPixelSize(bs);
			float lod = ceilf(log2(screenSize / (lodGeometry->getMaxViewSpaceError() * cv->getLODScale()))) - 1.0f;
	        lod = std::max(std::min(lod, 31.0f), 0.0f);

			lodGeometry->setLod(lod);
        }
        
        return false;
    }
};



LevelOfDetailGeometry::LevelOfDetailGeometry()
	: Geometry()
	, _lastLod(31)
	, _min(FLT_MIN)
	, _max(FLT_MAX)
	, _numProtectedVertices(0)
	, _lodUniform(new osg::Uniform("osg_VertexLod", 32.0f))
	, _minBoundsUniform(new osg::Uniform("osg_MinBounds", osg::Vec3(_min, _min, _min)))
	, _maxBoundsUniform(new osg::Uniform("osg_MaxBounds", osg::Vec3(_max, _max, _max)))
	, _numProtectedVerticesUniform(new osg::Uniform("osg_ProtectedVertices", _numProtectedVertices))
	, _maxViewSpaceError(1.0f) 
{
	setSupportsDisplayList(true);
	setUseDisplayList(true);
	setUseVertexBufferObjects(true);

    setCullCallback(new PopCullCallback());

	_lodUniform->setDataVariance(osg::Object::DYNAMIC);
	getOrCreateStateSet()->addUniform(_lodUniform);
	_stateset->addUniform(_minBoundsUniform);
	_stateset->addUniform(_maxBoundsUniform);
	_stateset->addUniform(_numProtectedVerticesUniform);
}

LevelOfDetailGeometry::LevelOfDetailGeometry(const LevelOfDetailGeometry& rhs, const CopyOp& copyop)
	: Geometry(rhs, copyop)
	, _lastLod(31)
	, _min(rhs._min)
	, _max(rhs._max)
	, _numProtectedVertices(rhs._numProtectedVertices)
	, _lodUniform(copyop(rhs._lodUniform))
	, _minBoundsUniform(copyop(rhs._minBoundsUniform))
	, _maxBoundsUniform(copyop(rhs._maxBoundsUniform))
	, _numProtectedVerticesUniform(copyop(rhs._numProtectedVerticesUniform))
	, _maxViewSpaceError(rhs._maxViewSpaceError)
{
	setSupportsDisplayList(true);
	setUseDisplayList(true);
	setUseVertexBufferObjects(true);

	getOrCreateStateSet()->addUniform(_lodUniform);
	_stateset->addUniform(_minBoundsUniform);
	_stateset->addUniform(_maxBoundsUniform);
	_stateset->addUniform(_numProtectedVerticesUniform);
}

void LevelOfDetailGeometry::setLod(float lod)
{
    for (auto primitive: _primitives)
    {
        LevelOfDetailDrawElements* lodDrawElements = dynamic_cast<LevelOfDetailDrawElements*>(primitive.get());
    
        if (lodDrawElements)
        {
            lodDrawElements->setLod((int)lod);
        }
    }

    _lodUniform->set(floor(lod)+1.0f);
	_lodUniform->dirty();

	if((int)lod != _lastLod)
	{
		dirtyDisplayList();
		_lastLod = (int)lod;
	}
}

void LevelOfDetailGeometry::updateUniforms()
{
	_minBoundsUniform->set(osg::Vec3(_min, _min, _min));
	_maxBoundsUniform->set(osg::Vec3(_max, _max, _max));
	_numProtectedVerticesUniform->set(_numProtectedVertices);
	_minBoundsUniform->dirty();
	_maxBoundsUniform->dirty();
	_numProtectedVerticesUniform->dirty();
}

void LevelOfDetailGeometry::reconnectUniforms()
{
    if (_stateset)
    {
        osg::Uniform* lodUniform = _stateset->getUniform("osg_VertexLod");
        if (lodUniform) { _lodUniform = lodUniform; }
            
        osg::Uniform* minBoundsUniform = _stateset->getUniform("osg_MinBounds");
        if (minBoundsUniform) { _minBoundsUniform = minBoundsUniform; }

        osg::Uniform* maxBoundsUniform = _stateset->getUniform("osg_MaxBounds");
        if (maxBoundsUniform) { _maxBoundsUniform = maxBoundsUniform; }

        osg::Uniform* numFixedVerticesUniform = _stateset->getUniform("osg_ProtectedVertices");
        if (numFixedVerticesUniform) { _numProtectedVerticesUniform = numFixedVerticesUniform; }
    }
}

std::string LevelOfDetailGeometry::getVertexShaderUniformDefintion()
{
    return "uniform vec3 osg_MinBounds;\n"
           "uniform vec3 osg_MaxBounds;\n"
           "uniform float osg_VertexLod;\n"
           "uniform int osg_ProtectedVertices;\n";
}

std::string LevelOfDetailGeometry::getVertexShaderFunctionDefinition()
{
    return "vec4 quantizeVertex(vec4 vertex)\n"
           "{\n"
	       "    if (gl_VertexID < osg_ProtectedVertices)\n"
	       "    {\n"
		   "        return vertex;\n"
	       "    }\n"
	       "    else\n"
	       "    {\n"
		   "        float factor = (pow(2.0, osg_VertexLod) - 1.0f) / (osg_MaxBounds.x-osg_MinBounds.x);\n"
		   "        float invFactor = (osg_MaxBounds.x-osg_MinBounds.x) / pow(2.0, osg_VertexLod);\n"
		   "        uvec3 q_vertex = uvec3(factor * (vertex.xyz-osg_MinBounds) + 0.5);\n"
		   "        return vec4(invFactor * vec3(q_vertex) + osg_MinBounds, 1.0);\n"
	       "    }\n"
           "};\n";
}

} // namespace osg