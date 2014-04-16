#pragma once

#include <vector>
#include <osg/Geode>
#include <osg/Geometry>


namespace osgPop
{

class OSG_EXPORT PopGeometry : public osg::Geometry
{
public:
	PopGeometry();
	PopGeometry(const PopGeometry& rhs, const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY);

    virtual osg::Object* cloneType() const { return new PopGeometry(); }
    virtual osg::Object* clone(const osg::CopyOp& copyop) const { return new PopGeometry(*this,copyop); }
    virtual bool isSameKindAs(const osg::Object* obj) const { return dynamic_cast<const PopGeometry*>(obj)!=NULL; }
    virtual const char* libraryName() const { return "osgPop"; }
    virtual const char* className() const { return "PopGeometry"; }

	void drawImplementation(osg::RenderInfo& renderInfo) const;

	inline void setMinBounds(float min) { _min = min; updateUniforms(); }
	inline float getMinBounds() const { return _min; }

	inline void setMaxBounds(float max) { _max = max; updateUniforms(); }
	inline float getMaxBounds() const { return _max; }

	inline void setNumberOfFixedVertices(int numFixedVertices) { _numFixedVertices = numFixedVertices; updateUniforms(); }
	inline int getNumberOfFixedVertices() const { return _numFixedVertices; }

	inline void setMaxViewSpaceError(float maxViewSpaceError) { _maxViewSpaceError = std::abs(maxViewSpaceError); }
	inline float getMaxViewSpaceError() const { return _maxViewSpaceError; }

	inline std::vector<GLint>& getLodRange() { return _lodRange; }
    inline const std::vector<GLint>& getLodRange() const { return _lodRange; }

    void reconnectUniforms();
protected:
	void setLod(int lod) const;
	void updateUniforms();

	virtual ~PopGeometry() {}

	std::vector<GLint> _lodRange;
	float _min;
	float _max;
	int _numFixedVertices;
	osg::ref_ptr<osg::Uniform> _lodUniform;
	osg::ref_ptr<osg::Uniform> _minBoundsUniform;
	osg::ref_ptr<osg::Uniform> _maxBoundsUniform;
	osg::ref_ptr<osg::Uniform> _numFixedVerticesUniform;

	float _maxViewSpaceError;
};

} // namespace osg