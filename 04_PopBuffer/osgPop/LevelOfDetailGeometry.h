#pragma once

#include <vector>
#include <string>
#include <osg/Geode>
#include <osg/Geometry>


namespace osg
{

struct PopCullCallback;

class OSG_EXPORT LevelOfDetailGeometry : public osg::Geometry
{
public:
    friend struct PopCullCallback;

	LevelOfDetailGeometry();
	LevelOfDetailGeometry(const LevelOfDetailGeometry& rhs, const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY);

    virtual osg::Object* cloneType() const { return new LevelOfDetailGeometry(); }
    virtual osg::Object* clone(const osg::CopyOp& copyop) const { return new LevelOfDetailGeometry(*this,copyop); }
    virtual bool isSameKindAs(const osg::Object* obj) const { return dynamic_cast<const LevelOfDetailGeometry*>(obj)!=NULL; }
    virtual const char* libraryName() const { return "osg"; }
    virtual const char* className() const { return "LevelOfDetailGeometry"; }

    inline void setMinBounds(float min) { _min = min; updateUniforms(); }
	inline float getMinBounds() const { return _min; }

	inline void setMaxBounds(float max) { _max = max; updateUniforms(); }
	inline float getMaxBounds() const { return _max; }

	inline void setNumberOfProtectedVertices(int numProtectedVertices) { _numProtectedVertices = numProtectedVertices; updateUniforms(); }
	inline int getNumberOfProtectedVertices() const { return _numProtectedVertices; }

	inline void setMaxViewSpaceError(float maxViewSpaceError) { _maxViewSpaceError = std::abs(maxViewSpaceError); }
	inline float getMaxViewSpaceError() const { return _maxViewSpaceError; }

    void reconnectUniforms();

    static std::string getVertexShaderUniformDefintion();
    static std::string getVertexShaderFunctionDefinition();
protected:
	void setLod(float lod);
	void updateUniforms();

	virtual ~LevelOfDetailGeometry() {}

    GLint _lastLod;
	float _min;
	float _max;
    int _numProtectedVertices;
	osg::ref_ptr<osg::Uniform> _lodUniform;
	osg::ref_ptr<osg::Uniform> _minBoundsUniform;
	osg::ref_ptr<osg::Uniform> _maxBoundsUniform;
	osg::ref_ptr<osg::Uniform> _numProtectedVerticesUniform;

	float _maxViewSpaceError;
};

} // namespace osg