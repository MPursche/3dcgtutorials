#include "LevelOfDetailDrawElements.h"

#include <osg/State>

using namespace osg;

namespace osg
{

void LevelOfDetailDrawElementsUByte::draw(State& state, bool useVertexBufferObjects) const
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
			if (_numInstances>=1) state.glDrawElementsInstanced(mode, _end, GL_UNSIGNED_BYTE, (const GLvoid *)(ebo->getOffset(getBufferIndex())), _numInstances);
            else glDrawElements(mode, _end, GL_UNSIGNED_BYTE, (const GLvoid *)(ebo->getOffset(getBufferIndex())));
        }
        else
        {
            if (_numInstances>=1) state.glDrawElementsInstanced(mode, _end, GL_UNSIGNED_BYTE, &front(), _numInstances);
            else glDrawElements(mode, _end, GL_UNSIGNED_BYTE, &front());
        }
    }
    else
    {
        if (_numInstances>=1) state.glDrawElementsInstanced(mode, _end, GL_UNSIGNED_BYTE, &front(), _numInstances);
        else glDrawElements(mode, _end, GL_UNSIGNED_BYTE, &front());
    }
}

void LevelOfDetailDrawElementsUByte::accept(PrimitiveFunctor& functor) const
{
	if (!empty()) functor.drawElements(_mode,_end,&front());
}

void LevelOfDetailDrawElementsUByte::accept(PrimitiveIndexFunctor& functor) const
{
    if (!empty()) functor.drawElements(_mode,_end,&front());
}

void LevelOfDetailDrawElementsUShort::draw(State& state, bool useVertexBufferObjects) const
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
			if (_numInstances>=1) state.glDrawElementsInstanced(mode, _end, GL_UNSIGNED_SHORT, (const GLvoid *)(ebo->getOffset(getBufferIndex())), _numInstances);
            else glDrawElements(mode, _end, GL_UNSIGNED_SHORT, (const GLvoid *)(ebo->getOffset(getBufferIndex())));
        }
        else
        {
            if (_numInstances>=1) state.glDrawElementsInstanced(mode, _end, GL_UNSIGNED_SHORT, &front(), _numInstances);
            else glDrawElements(mode, _end, GL_UNSIGNED_SHORT, &front());
        }
    }
    else
    {
        if (_numInstances>=1) state.glDrawElementsInstanced(mode, _end, GL_UNSIGNED_SHORT, &front(), _numInstances);
        else glDrawElements(mode, _end, GL_UNSIGNED_SHORT, &front());
    }
}

void LevelOfDetailDrawElementsUShort::accept(PrimitiveFunctor& functor) const
{
	if (!empty()) functor.drawElements(_mode,_end,&front());
}

void LevelOfDetailDrawElementsUShort::accept(PrimitiveIndexFunctor& functor) const
{
    if (!empty()) functor.drawElements(_mode,_end,&front());
}

void LevelOfDetailDrawElementsUInt::draw(State& state, bool useVertexBufferObjects) const
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
			if (_numInstances>=1) state.glDrawElementsInstanced(mode, _end, GL_UNSIGNED_INT, (const GLvoid *)(ebo->getOffset(getBufferIndex())), _numInstances);
            else glDrawElements(mode, _end, GL_UNSIGNED_INT, (const GLvoid *)(ebo->getOffset(getBufferIndex())));
        }
        else
        {
            if (_numInstances>=1) state.glDrawElementsInstanced(mode, _end, GL_UNSIGNED_INT, &front(), _numInstances);
            else glDrawElements(mode, _end, GL_UNSIGNED_INT, &front());
        }
    }
    else
    {
        if (_numInstances>=1) state.glDrawElementsInstanced(mode, _end, GL_UNSIGNED_INT, &front(), _numInstances);
        else glDrawElements(mode, _end, GL_UNSIGNED_INT, &front());
    }
}

void LevelOfDetailDrawElementsUInt::accept(PrimitiveFunctor& functor) const
{
	if (!empty()) functor.drawElements(_mode,_end,&front());
}

void LevelOfDetailDrawElementsUInt::accept(PrimitiveIndexFunctor& functor) const
{
    if (!empty()) functor.drawElements(_mode,_end,&front());
}

}

