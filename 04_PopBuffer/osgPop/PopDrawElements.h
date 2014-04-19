#pragma once

#include <osg/PrimitiveSet>

namespace osgPop
{

class OSG_EXPORT PopDrawElements
{
public:
	PopDrawElements()
		: _start(0)
		, _end(0)
	{
	}
		
	inline void setStart(size_t start) { _start = start; }
	inline void setEnd(size_t end) { _end = end; }
protected:
	size_t _start;
	size_t _end;
};

class OSG_EXPORT PopDrawElementsUByte : public PopDrawElements, public osg::DrawElementsUByte
{
public:
	PopDrawElementsUByte(GLenum mode=0)
		: osg::DrawElementsUByte(mode)
	{
		_end = size();
	}

    PopDrawElementsUByte(const osg::DrawElementsUByte& array, const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY)
        : osg::DrawElementsUByte(array,copyop)
    {
		_end = size();
	}

    PopDrawElementsUByte(const PopDrawElementsUByte& array, const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY)
        : osg::DrawElementsUByte(array,copyop)
    {
		_end = size();
	}

	virtual void draw(osg::State& state, bool useVertexBufferObjects) const;
	virtual void accept(osg::PrimitiveFunctor& functor) const;
	virtual void accept(osg::PrimitiveIndexFunctor& functor) const;
};

class OSG_EXPORT PopDrawElementsUShort : public PopDrawElements, public osg::DrawElementsUShort
{
public:
	PopDrawElementsUShort(GLenum mode=0)
		: osg::DrawElementsUShort(mode)
	{
		_end = size();
	}

    PopDrawElementsUShort(const osg::DrawElementsUShort& array, const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY)
        : osg::DrawElementsUShort(array,copyop)
    {
		_end = size();
	}

    PopDrawElementsUShort(const PopDrawElementsUShort& array, const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY)
        : osg::DrawElementsUShort(array,copyop)
    {
		_end = size();
	}

	virtual void draw(osg::State& state, bool useVertexBufferObjects) const;
	virtual void accept(osg::PrimitiveFunctor& functor) const;
	virtual void accept(osg::PrimitiveIndexFunctor& functor) const;
};

class OSG_EXPORT PopDrawElementsUInt : public PopDrawElements, public osg::DrawElementsUInt
{
public:
	PopDrawElementsUInt(GLenum mode=0)
		: osg::DrawElementsUInt(mode)
	{
		_end = size();
	}

    PopDrawElementsUInt(const osg::DrawElementsUInt& array, const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY)
        : osg::DrawElementsUInt(array,copyop)
    {
		_end = size();
	}

    PopDrawElementsUInt(const PopDrawElementsUInt& array, const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY)
        : osg::DrawElementsUInt(array,copyop)
    {
		_end = size();
	}

	virtual void draw(osg::State& state, bool useVertexBufferObjects) const;
	virtual void accept(osg::PrimitiveFunctor& functor) const;
	virtual void accept(osg::PrimitiveIndexFunctor& functor) const;
};

}