#include "PopDrawElements.h"

#include <osg/State>

using namespace osg;

namespace osgPop 
{

void PopDrawElements::draw(State& state, bool useVertexBufferObjects) const
{
	GLenum mode = _mode;
    #if defined(OSG_GLES1_AVAILABLE) || defined(OSG_GLES2_AVAILABLE)
        if (mode==GL_POLYGON) mode = GL_TRIANGLE_FAN;
        if (mode==GL_QUAD_STRIP) mode = GL_TRIANGLE_STRIP;
    #endif

    if (useVertexBufferObjects)
    {
        GLBufferObject* ebo = getOrCreateGLBufferObject(state.getContextID());
        state.bindElementBufferObject(ebo);
        if (ebo)
        {
			if (_numInstances>=1) state.glDrawElementsInstanced(mode, _end, GL_UNSIGNED_INT, (const GLvoid *)(ebo->getOffset(getBufferIndex()) + _start), _numInstances);
            else glDrawElements(mode, _end, GL_UNSIGNED_INT, (const GLvoid *)(ebo->getOffset(getBufferIndex())+ _start));
        }
        else
        {
            if (_numInstances>=1) state.glDrawElementsInstanced(mode, _end, GL_UNSIGNED_INT, &front() + _start, _numInstances);
            else glDrawElements(mode, _end, GL_UNSIGNED_INT, &front() + _start);
        }
    }
    else
    {
        if (_numInstances>=1) state.glDrawElementsInstanced(mode, _end, GL_UNSIGNED_INT, &front() + _start, _numInstances);
        else glDrawElements(mode, _end, GL_UNSIGNED_INT, &front() + _start);
    }
}

void PopDrawElements::accept(PrimitiveFunctor& functor) const
{
	if (!empty()) functor.drawElements(_mode,_end,&front()+_start);
}

void PopDrawElements::accept(PrimitiveIndexFunctor& functor) const
{
    if (!empty()) functor.drawElements(_mode,_end,&front()+_start);
}

}
