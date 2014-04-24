#pragma once

#include "Vec3ui.h"

// std
#include <memory>
#include <vector>
#include <map>

// osg
#include <osg/ref_ptr>
#include <osg/PrimitiveSet>

namespace osgUtil
{

/**
 @brief Helper class for half edges
*/
struct HalfEdge
{
	HalfEdge(unsigned int vertex, unsigned int originalVertex)
		: vertexID(vertex)
		, originalVertexID(originalVertex)
		, next(LLONG_MAX)
		, prev(LLONG_MAX)
		, opposite(LLONG_MAX)
	{}

	unsigned int vertexID;
	unsigned int originalVertexID;
	size_t next;
	size_t prev;
	size_t opposite;
};

/**
 TriangleCollector template to collect half edges and sort triangles for op buffer
*/

template<class VertexArray, class Vector> struct HalfEdgeTriangleCollector
{
	osg::ref_ptr<VertexArray>				_vertexArray;
	std::map<Vector, unsigned int>          _vertexIDMap;
	unsigned int                            _vertexIDCounter;
	std::vector<HalfEdge>*                  _halfEdges;
    osg::ref_ptr<osg::DrawElementsUInt>     _drawElements;

    HalfEdgeTriangleCollector()
        : _vertexArray(NULL)
		, _halfEdges(NULL)
		, _vertexIDCounter(0)
        , _drawElements(NULL)
    {
	}
	                    
    void operator()(unsigned int pos1, unsigned int pos2, unsigned int pos3)
    {
			// skip collapsed triangles
			if (_vertexArray->at(pos1) == _vertexArray->at(pos2)  ||
				_vertexArray->at(pos1) == _vertexArray->at(pos3)  ||
				_vertexArray->at(pos2) == _vertexArray->at(pos3))
			{
				return;
			}
			
			// try to find vertices in ID map
			auto it = _vertexIDMap.find(_vertexArray->at(pos1));
			if (it == _vertexIDMap.end()) { _vertexIDMap[_vertexArray->at(pos1)] = _vertexIDCounter++; }
			it = _vertexIDMap.find(_vertexArray->at(pos2));
			if (it == _vertexIDMap.end()) { _vertexIDMap[_vertexArray->at(pos2)] = _vertexIDCounter++; }
			it = _vertexIDMap.find(_vertexArray->at(pos3));
			if (it == _vertexIDMap.end()) { _vertexIDMap[_vertexArray->at(pos3)] = _vertexIDCounter++; }

			// add half edges of the triangle
			_halfEdges->push_back(HalfEdge(_vertexIDMap[_vertexArray->at(pos1)], pos1));
			_halfEdges->push_back(HalfEdge(_vertexIDMap[_vertexArray->at(pos2)], pos2));
			_halfEdges->push_back(HalfEdge(_vertexIDMap[_vertexArray->at(pos3)], pos3));
			size_t lastIndex = _halfEdges->size() - 1;
			_halfEdges->at(lastIndex-2).next = lastIndex-1;
			_halfEdges->at(lastIndex-2).prev = lastIndex;
			_halfEdges->at(lastIndex-1).next = lastIndex;
			_halfEdges->at(lastIndex-1).prev = lastIndex-2;
			_halfEdges->at(lastIndex).next = lastIndex-2;
			_halfEdges->at(lastIndex).prev = lastIndex-1;

            // add triangle to the draw elements
            _drawElements->push_back(pos1);
            _drawElements->push_back(pos2);
            _drawElements->push_back(pos3);
    }
};

template<class VertexArray, class Vector> struct LodTriangleCollector
{
    osg::ref_ptr<VertexArray>							_vertexArray;
	std::vector<osg::ref_ptr<osg::DrawElementsUInt> >*  _lodDrawElements;
	float _min;
	float _max;
    unsigned int _numProtectedVertices;

    LodTriangleCollector()
        : _vertexArray(NULL)
		, _lodDrawElements(NULL)
        , _numProtectedVertices(0)
    {		
	}
	                    
    void operator()(unsigned int pos1, unsigned int pos2, unsigned int pos3)
    {
			// skip collapsed triangles
			if (_vertexArray->at(pos1) == _vertexArray->at(pos2)  ||
				_vertexArray->at(pos1) == _vertexArray->at(pos3)  ||
				_vertexArray->at(pos2) == _vertexArray->at(pos3))
			{
				return;
			}
                      
			
			// quantize vertices with different precisions to determine the lod, where triangles don't collapse
			for (int k = 1; k <= 32; ++k)
			{
				float precision = pow(2.0f, k);
				Vector vertices[3] = { _vertexArray->at(pos1),
									   _vertexArray->at(pos2),
									   _vertexArray->at(pos3) };

				Vec3ui qVertices[3] = {   quantize(k, _min, _max, vertices[0]),
									      quantize(k, _min, _max, vertices[1]),
										  quantize(k, _min, _max, vertices[2]) };

                Vector newVertices[3] = { (pos1 < _numProtectedVertices) ?  vertices[0] : dequantize<Vector>(k, _min, _max, qVertices[0]),
                                          (pos2 < _numProtectedVertices) ?  vertices[1] : dequantize<Vector>(k, _min, _max, qVertices[1]),
                                          (pos3 < _numProtectedVertices) ?  vertices[2] : dequantize<Vector>(k, _min, _max, qVertices[2]) };

				if ((newVertices[0] != newVertices[1] &&
					newVertices[0] != newVertices[2] &&
					newVertices[1] != newVertices[2]) ||
                    k == 32)
				{
					(*_lodDrawElements)[int(k-1)]->push_back(pos1);
					(*_lodDrawElements)[int(k-1)]->push_back(pos2);
					(*_lodDrawElements)[int(k-1)]->push_back(pos3);

					break;
				}
			}
    }
};

}