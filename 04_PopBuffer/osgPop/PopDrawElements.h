#pragma once

#include <osg/PrimitiveSet>

namespace osgPop
{

class OSG_EXPORT PopDrawElements : public osg::DrawElementsUInt
{
public:
	PopDrawElements(GLenum mode=0)
		: osg::DrawElementsUInt(mode)
		, _start(0)
		, _end(0)
	{
	}

    PopDrawElements(const osg::DrawElementsUInt& array, const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY)
        :    osg::DrawElementsUInt(array,copyop)
        ,   _start(0)
        ,   _end(size())
    {}

    PopDrawElements(const PopDrawElements& array, const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY)
        :    osg::DrawElementsUInt(array,copyop)
        ,    _start(array._start)
        ,    _end(array._end)
    {}

	virtual void draw(osg::State& state, bool useVertexBufferObjects) const;
	virtual void accept(osg::PrimitiveFunctor& functor) const;
	virtual void accept(osg::PrimitiveIndexFunctor& functor) const;

	inline void setStart(size_t start) { _start = start; }
	inline void setEnd(size_t end) { _end = end; }

protected:
	size_t _start;
	size_t _end;
};

}