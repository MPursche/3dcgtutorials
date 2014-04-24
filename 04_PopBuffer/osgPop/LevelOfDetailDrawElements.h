#pragma once

#include <osg/PrimitiveSet>

namespace osg
{

class OSG_EXPORT LevelOfDetailDrawElements
{
public:
	LevelOfDetailDrawElements()
	    : _lodRange(32) 
        , _end(0)
	{
	}

    LevelOfDetailDrawElements(const LevelOfDetailDrawElements& rhs)
        : _lodRange(rhs._lodRange)
        , _end(rhs._end)
    {
    }

    inline void setLod(int lod) { if (lod >= 0 && lod < 32) { _end = _lodRange[lod]; } }
    inline void setLodRanges(const std::vector<GLint>& lodRange) { _lodRange = lodRange; }
    inline std::vector<GLint> getLodRanges() const { return _lodRange; }
protected:
    std::vector<GLint> _lodRange;
	GLint _end;
};

class OSG_EXPORT LevelOfDetailDrawElementsUByte : public LevelOfDetailDrawElements, public osg::DrawElementsUByte
{
public:
	LevelOfDetailDrawElementsUByte(GLenum mode=0)
		: osg::DrawElementsUByte(mode)
	{
		_end = size();
	}

    LevelOfDetailDrawElementsUByte(const osg::DrawElementsUByte& array, const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY)
        : osg::DrawElementsUByte(array,copyop)
    {
		_end = size();
	}

    LevelOfDetailDrawElementsUByte(const LevelOfDetailDrawElementsUByte& array, const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY)
        : osg::DrawElementsUByte(array,copyop)
        , LevelOfDetailDrawElements(array)
    {
		_end = size();
	}

	virtual void draw(osg::State& state, bool useVertexBufferObjects) const;
	virtual void accept(osg::PrimitiveFunctor& functor) const;
	virtual void accept(osg::PrimitiveIndexFunctor& functor) const;
};

class OSG_EXPORT LevelOfDetailDrawElementsUShort : public LevelOfDetailDrawElements, public osg::DrawElementsUShort
{
public:
	LevelOfDetailDrawElementsUShort(GLenum mode=0)
		: osg::DrawElementsUShort(mode)
	{
		_end = size();
	}

    LevelOfDetailDrawElementsUShort(const osg::DrawElementsUShort& array, const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY)
        : osg::DrawElementsUShort(array,copyop)
    {
		_end = size();
	}

    LevelOfDetailDrawElementsUShort(const LevelOfDetailDrawElementsUShort& array, const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY)
        : osg::DrawElementsUShort(array,copyop)
        , LevelOfDetailDrawElements(array)
    {
		_end = size();
	}

	virtual void draw(osg::State& state, bool useVertexBufferObjects) const;
	virtual void accept(osg::PrimitiveFunctor& functor) const;
	virtual void accept(osg::PrimitiveIndexFunctor& functor) const;
};

class OSG_EXPORT LevelOfDetailDrawElementsUInt : public LevelOfDetailDrawElements, public osg::DrawElementsUInt
{
public:
	LevelOfDetailDrawElementsUInt(GLenum mode=0)
		: osg::DrawElementsUInt(mode)
	{
		_end = size();
	}

    LevelOfDetailDrawElementsUInt(const osg::DrawElementsUInt& array, const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY)
        : osg::DrawElementsUInt(array,copyop)
    {
		_end = size();
	}

    LevelOfDetailDrawElementsUInt(const LevelOfDetailDrawElementsUInt& array, const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY)
        : osg::DrawElementsUInt(array,copyop)
        , LevelOfDetailDrawElements(array)
    {
		_end = size();
	}

	virtual void draw(osg::State& state, bool useVertexBufferObjects) const;
	virtual void accept(osg::PrimitiveFunctor& functor) const;
	virtual void accept(osg::PrimitiveIndexFunctor& functor) const;
};

}