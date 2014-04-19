#include "ConvertToPopGeometryVisitor.h"
#include "PopDrawElements.h"
#include "HalfEdge.h"

#include <osg/Array>
#include <osg/Geode>
#include <osg/TriangleIndexFunctor>

#include <iostream>
#include <memory>
#include <cmath>

using namespace std;
using namespace osg;

namespace osgPop
{

void ConvertToPopGeometryVisitor::apply(Geode& geode)
{
	// convert all geometries to pop geometries
	vector<ref_ptr<PopGeometry> > popGeometries;
	for (size_t i = 0; i < geode.getNumDrawables(); ++i)
	{
		ref_ptr<Geometry> geometry = dynamic_cast<Geometry*>(geode.getDrawable(i));

		if (geometry)
		{
			popGeometries.push_back(convert(geometry));			
		}
	}

	// replace geometries of the geode
	geode.removeDrawables(0, geode.getNumDrawables());

	for (auto it: popGeometries)
	{
		geode.addDrawable(it);
	}
}

ref_ptr<PopGeometry> ConvertToPopGeometryVisitor::convert(ref_ptr<Geometry> geometry)
{
	// assertions
	if (!geometry) { return NULL; }
	if (!geometry->getVertexArray()) { return NULL; }

	// create pop geometry
	ref_ptr<PopGeometry> popGeometry = new PopGeometry();

	// merge state sets
	if (geometry->getStateSet())
	{	
		popGeometry->getOrCreateStateSet()->merge(*geometry->getStateSet());
	}
	
	// compute bounding box and set min and max bounds
	BoundingBox bounds = geometry->getBound();
	float min =  std::min(bounds.xMin(), std::min(bounds.yMin(), bounds.zMin()));
	float max =  std::max(bounds.xMax(), std::max(bounds.yMax(), bounds.zMax()));
	popGeometry->setMinBounds(min);
	popGeometry->setMaxBounds(max);

    // collect half edges
    osg::ref_ptr<osg::DrawElementsUInt> drawElements = NULL;
    shared_ptr<vector<HalfEdge> > halfEdges;
    if (!collectHalfEdges(geometry, drawElements, halfEdges)) { return NULL; }
    popGeometry->addPrimitiveSet(drawElements);

    // find half edges opposites sort protected vertices to the front
	findHalfEdgeOpposite(halfEdges);    
    findAndSortProtectedVertices(geometry, popGeometry, halfEdges);
    
	// collect triangles and create list sorted by LODs
	shared_ptr<vector<ref_ptr<DrawElementsUInt> > > lodDrawElements;
    if (!collectLod(popGeometry, lodDrawElements, min, max, popGeometry->getNumberOfFixedVertices())) { return NULL; }

    // create draw primitive
    createAndAddDrawPrimitive(lodDrawElements, popGeometry);

    // recompute bounds
	popGeometry->computeBound();

	return popGeometry;
}

template<class VertexArray, class Vector> void _collectHalfEdges(osg::ref_ptr<osg::Geometry> geometry,
                                                                 osg::ref_ptr<osg::DrawElementsUInt>&     drawElements,
                                                                 std::shared_ptr<std::vector<HalfEdge> >& halfEdges)
{
	TriangleIndexFunctor<HalfEdgeTriangleCollector<VertexArray, Vector> > triangleCollector;
	triangleCollector._vertexArray = dynamic_cast<VertexArray*>(geometry->getVertexArray());

	for (size_t i = 0; i < geometry->getNumPrimitiveSets(); ++i)
	{
		geometry->getPrimitiveSet(i)->accept(triangleCollector);		
	}

    halfEdges = triangleCollector._halfEdges;
    drawElements = triangleCollector._drawElements;
}

bool ConvertToPopGeometryVisitor::collectHalfEdges(osg::ref_ptr<osg::Geometry> geometry,
                                                   osg::ref_ptr<osg::DrawElementsUInt>&     drawElements,
                                                   std::shared_ptr<std::vector<HalfEdge> >& halfEdges)
{
    switch(geometry->getVertexArray()->getType())
	{
		case Array::Vec3ArrayType:
		{
            _collectHalfEdges<Vec3Array, Vec3>(geometry, drawElements, halfEdges);
		} break;
		case Array::Vec3dArrayType:
		{
			_collectHalfEdges<Vec3dArray, Vec3d>(geometry, drawElements, halfEdges);
		} break;
		case Array::Vec3bArrayType:
		{
			_collectHalfEdges<Vec3bArray, Vec3b>(geometry, drawElements, halfEdges);
		} break;
		case Array::Vec3sArrayType:
		{
			_collectHalfEdges<Vec3sArray, Vec3s>(geometry, drawElements, halfEdges);
		} break;
		default:
			// unknown vertex format
			return false;
			break;
	}

	return true;
}

template<class VertexArray, class Vector> void _collectLod(ref_ptr<Geometry> geometry,
														   shared_ptr<vector<ref_ptr<DrawElementsUInt> > >& lodDrawElements,
														   float min,
														   float max,
                                                           int numProtectedVertices)
{
	TriangleIndexFunctor<LodTriangleCollector<VertexArray, Vector> > triangleCollector;
	triangleCollector._vertexArray = dynamic_cast<VertexArray*>(geometry->getVertexArray());
	triangleCollector._min = min;
	triangleCollector._max = max;
    triangleCollector._numProtectedVertices = numProtectedVertices;

	for (size_t i = 0; i < geometry->getNumPrimitiveSets(); ++i)
	{
		geometry->getPrimitiveSet(i)->accept(triangleCollector);		
	}

	lodDrawElements = triangleCollector._lodDrawElements;
}

bool ConvertToPopGeometryVisitor::collectLod(ref_ptr<Geometry> geometry,
											 shared_ptr<vector<ref_ptr<DrawElementsUInt> > >& lodDrawElements,
											 float min,
											 float max,
                                             int numProtectedVertices)
{
	switch(geometry->getVertexArray()->getType())
	{
		case Array::Vec3ArrayType:
		{
			_collectLod<Vec3Array, Vec3>(geometry, lodDrawElements, min, max, numProtectedVertices);
		} break;
		case Array::Vec3dArrayType:
		{
			_collectLod<Vec3dArray, Vec3d>(geometry, lodDrawElements, min, max, numProtectedVertices);
		} break;
		case Array::Vec3bArrayType:
		{
			_collectLod<Vec3bArray, Vec3b>(geometry, lodDrawElements, min, max, numProtectedVertices);
		} break;
		case Array::Vec3sArrayType:
		{
			_collectLod<Vec3sArray, Vec3s>(geometry, lodDrawElements, min, max, numProtectedVertices);
		} break;
		default:
			// unknown vertex format
			return false;
			break;
	}

	return true;
}

void ConvertToPopGeometryVisitor::findHalfEdgeOpposite(shared_ptr<vector<HalfEdge> > halfEdges)
{
	map<pair<unsigned int, unsigned int>, size_t> oppositeMap;

	for (size_t i = 0; i < halfEdges->size(); ++i)
	{
		HalfEdge* halfEdge = &halfEdges->at(i);
		pair<unsigned int, unsigned int> index(halfEdge->vertexID, halfEdges->at(halfEdge->next).vertexID);
		pair<unsigned int, unsigned int> oppositeIndex(halfEdges->at(halfEdge->next).vertexID, halfEdge->vertexID);

		oppositeMap[index] = i;

		// try to find the opposite half edge
		auto oppositeIt = oppositeMap.find(oppositeIndex);
		if (oppositeIt != oppositeMap.end())
		{
			halfEdge->opposite = oppositeIt->second;
			halfEdges->at(oppositeIt->second).opposite = i;
		}
	}	
}

void ConvertToPopGeometryVisitor::createAndAddDrawPrimitive(std::shared_ptr<std::vector<ref_ptr<DrawElementsUInt> > > lodDrawElements,
                                                            ref_ptr<PopGeometry> popGeometry)
{
    // first merge geometry in a UInt draw element
	ref_ptr<PopDrawElementsUInt> popDrawElements = new PopDrawElementsUInt(GL_TRIANGLES);

	for (size_t j = 0; j < lodDrawElements->size(); ++j)
	{
		popDrawElements->insert(popDrawElements->end(), lodDrawElements->at(j)->begin(), lodDrawElements->at(j)->end());
		popGeometry->getLodRange()[j] = popDrawElements->size();
	}
	popDrawElements->setStart(0);
	popDrawElements->setEnd(popGeometry->getLodRange()[31]);

	// then create a draw element that fits the vertexbuffer size
	size_t numVertices = popGeometry->getVertexArray()->getNumElements();

	if (numVertices <= UCHAR_MAX)
	{
		ref_ptr<PopDrawElementsUByte> drawElements = new PopDrawElementsUByte(GL_TRIANGLES);
		drawElements->reserve(popDrawElements->size());

		for (size_t j = 0; j < popDrawElements->size(); ++j)
		{
			drawElements->push_back(popDrawElements->at(j));
		}
		popGeometry->removePrimitiveSet(0);
		popGeometry->addPrimitiveSet(drawElements);
	} 
	else if (numVertices <= USHRT_MAX) 
	{
		ref_ptr<PopDrawElementsUShort> drawElements = new PopDrawElementsUShort(GL_TRIANGLES);
		drawElements->reserve(popDrawElements->size());

		for (size_t j = 0; j < popDrawElements->size(); ++j)
		{
			drawElements->push_back(popDrawElements->at(j));
		}
		popGeometry->removePrimitiveSet(0);
		popGeometry->addPrimitiveSet(drawElements);
	}
	else
	{
		popGeometry->removePrimitiveSet(0);
		popGeometry->addPrimitiveSet(popDrawElements);
	}
}

void ConvertToPopGeometryVisitor::findAndSortProtectedVertices(ref_ptr<Geometry> geometry, ref_ptr<PopGeometry> popGeometry, std::shared_ptr<std::vector<HalfEdge> > halfEdges)
{
    ref_ptr<Array> vertexArray = geometry->getVertexArray();
	ref_ptr<Array> normalArray = geometry->getNormalArray();
	ref_ptr<Array> colorArray = geometry->getColorArray();
	ref_ptr<Array> secondaryColorArray = geometry->getSecondaryColorArray();
	ref_ptr<Array> fogCoordArray = geometry->getFogCoordArray();
	Geometry::ArrayList& texCoordArrays = geometry->getTexCoordArrayList();
	Geometry::ArrayList& vertexAttribArrays = geometry->getVertexAttribArrayList();	

	ref_ptr<Array> protectedVertices = createArrayOfType(vertexArray);
	ref_ptr<Array> regularVertices = createArrayOfType(vertexArray);
	ref_ptr<Array> protectedNormals = createArrayOfType(normalArray);
	ref_ptr<Array> regularNormals = createArrayOfType(normalArray);
	ref_ptr<Array> protectedColors = createArrayOfType(colorArray);
	ref_ptr<Array> regularColors = createArrayOfType(colorArray);
	ref_ptr<Array> protectedSecondaryColors = createArrayOfType(secondaryColorArray);
	ref_ptr<Array> regularSecondaryColors = createArrayOfType(secondaryColorArray);
	ref_ptr<Array> protectedFogCoords = createArrayOfType(fogCoordArray);
	ref_ptr<Array> regularFogCoords = createArrayOfType(fogCoordArray);
	vector<ref_ptr<Array> > protectedTexCoords;
	vector<ref_ptr<Array> > regularTexCoords;
	for (auto texCoordArray: texCoordArrays)
	{
		protectedTexCoords.push_back(createArrayOfType(texCoordArray));
		regularTexCoords.push_back(createArrayOfType(texCoordArray));
	}
	vector<ref_ptr<Array> > protectedVertexAttribs;
	vector<ref_ptr<Array> > regularVertexAttribs;
	for (auto vertexAttribArray: vertexAttribArrays)
	{
		protectedVertexAttribs.push_back(createArrayOfType(vertexAttribArray));
		regularVertexAttribs.push_back(createArrayOfType(vertexAttribArray));
	}

	// first find protected vertices
    set<unsigned int> protectedVertexSet;
    for (size_t i = 0; i < halfEdges->size(); ++i)
	{
		HalfEdge* halfEdge = &halfEdges->at(i);
        HalfEdge* nextEdge = &halfEdges->at(halfEdge->next);
        HalfEdge* prevEdge = &halfEdges->at(halfEdge->prev);
       
        if (halfEdge->opposite == LLONG_MAX)
		{
			// no opposite add this half edge and the next to the set
			protectedVertexSet.insert(halfEdge->vertexID);
            protectedVertexSet.insert(nextEdge->vertexID);          
		}
        else if (prevEdge->opposite == LLONG_MAX)
        {
            // prev half edge has no opposite add this and the previous to the set
			protectedVertexSet.insert(halfEdge->vertexID);
            protectedVertexSet.insert(prevEdge->vertexID);
        }
	}
    
    map<unsigned int, unsigned int> protectedVertexIDMap;
   	map<unsigned int, unsigned int> regularVertexIDMap;

	for (size_t i = 0; i < halfEdges->size(); ++i)
	{
		HalfEdge* halfEdge = &halfEdges->at(i);
		auto protectedIt = protectedVertexIDMap.find(halfEdge->originalVertexID);
		auto regularIt = regularVertexIDMap.find(halfEdge->originalVertexID);
        
        if (protectedVertexSet.find(halfEdge->vertexID) != protectedVertexSet.end() &&
			protectedIt == protectedVertexIDMap.end())
		{
			// protected vertex buffer
			addElementTo(protectedVertices, vertexArray, halfEdge->originalVertexID);
			addElementTo(protectedNormals, normalArray, halfEdge->originalVertexID);
			addElementTo(protectedColors, colorArray, halfEdge->originalVertexID);
			addElementTo(protectedSecondaryColors, secondaryColorArray, halfEdge->originalVertexID);
			addElementTo(protectedFogCoords, fogCoordArray, halfEdge->originalVertexID);
			for (size_t j = 0; j < protectedTexCoords.size(); ++j)
			{
				addElementTo(protectedTexCoords[j], texCoordArrays[j], halfEdge->originalVertexID);
			}
			for (size_t j = 0; j < protectedVertexAttribs.size(); ++j)
			{
				addElementTo(protectedVertexAttribs[j], vertexAttribArrays[j], halfEdge->originalVertexID);
			}

			protectedVertexIDMap[halfEdge->originalVertexID] = protectedVertices->getNumElements()-1;
		}
		else if (regularIt == regularVertexIDMap.end())
        {
			// regular vertex buffer
			addElementTo(regularVertices, vertexArray, halfEdge->originalVertexID);
			addElementTo(regularNormals, normalArray, halfEdge->originalVertexID);
			addElementTo(regularColors, colorArray, halfEdge->originalVertexID);
			addElementTo(regularSecondaryColors, secondaryColorArray, halfEdge->originalVertexID);
			addElementTo(regularFogCoords, fogCoordArray, halfEdge->originalVertexID);
			for (size_t j = 0; j < regularTexCoords.size(); ++j)
			{
				addElementTo(regularTexCoords[j], texCoordArrays[j], halfEdge->originalVertexID);
			}
			for (size_t j = 0; j < regularVertexAttribs.size(); ++j)
			{
				addElementTo(regularVertexAttribs[j], vertexAttribArrays[j], halfEdge->originalVertexID);
			}

			regularVertexIDMap[halfEdge->originalVertexID] = regularVertices->getNumElements()-1;
		}
	}
    
	// merge both buckets and replace old vertex array
	size_t numFixedVertices = protectedVertices->getNumElements();
	mergeArrays(protectedVertices, regularVertices);
	mergeArrays(protectedNormals, regularNormals);
	mergeArrays(protectedColors, regularColors);
	mergeArrays(protectedSecondaryColors, regularSecondaryColors);
	mergeArrays(protectedFogCoords, regularFogCoords);
	for (size_t j = 0; j < regularTexCoords.size(); ++j)
	{
		mergeArrays(protectedTexCoords[j], regularTexCoords[j]);
	}
	for (size_t j = 0; j < regularVertexAttribs.size(); ++j)
	{
		mergeArrays(protectedVertexAttribs[j], regularVertexAttribs[j]);
	}

	popGeometry->setVertexArray(protectedVertices);
	popGeometry->setNormalArray(protectedNormals);
	popGeometry->setNormalBinding(geometry->getNormalBinding());
	popGeometry->setColorArray(protectedColors);
	popGeometry->setColorBinding(geometry->getColorBinding());
	popGeometry->setSecondaryColorArray(protectedSecondaryColors);
	popGeometry->setSecondaryColorBinding(geometry->getSecondaryColorBinding());
	popGeometry->setFogCoordArray(protectedFogCoords);
	popGeometry->setFogCoordBinding(geometry->getFogCoordBinding());
	for (size_t j = 0; j < protectedTexCoords.size(); ++j)
	{
		popGeometry->setTexCoordArray(j, protectedTexCoords[j]);
	}
	for (size_t j = 0; j < protectedVertexAttribs.size(); ++j)
	{
		popGeometry->setVertexAttribArray(j, protectedVertexAttribs[j]);
		popGeometry->setVertexAttribBinding(j, geometry->getVertexAttribBinding(j));
	}

    ref_ptr<DrawElementsUInt> popDrawElements = dynamic_cast<DrawElementsUInt*>(popGeometry->getPrimitiveSet(0));
	
	// translate vertexIDs to new ID
	for (size_t i = 0; i < popDrawElements->size(); ++i)
	{
		auto protectedIt = protectedVertexIDMap.find(popDrawElements->at(i));
		auto normalIt = regularVertexIDMap.find(popDrawElements->at(i));

		if (protectedIt != protectedVertexIDMap.end())
		{
			popDrawElements->at(i) = protectedIt->second;
		}
		else if (normalIt != regularVertexIDMap.end())
		{
			popDrawElements->at(i) = normalIt->second + numFixedVertices;
		}
		else
		{
			// this should never happen
			cout << "No valid mapping found for vertex." << endl;
			break;
		}
	}

    // set number of fixed vertices for the shader    
	popGeometry->setNumberOfFixedVertices(numFixedVertices);
}

ref_ptr<Array> ConvertToPopGeometryVisitor::createArrayOfType(osg::ref_ptr<osg::Array> rhs)
{
	if (rhs == NULL)
		return NULL;

	switch (rhs->getType())
	{
	case Array::ByteArrayType:
		return new ByteArray();
		break;
	case Array::ShortArrayType:
		return new ShortArray();
		break;
	case Array::IntArrayType:
		return new IntArray();
		break;
	case Array::UByteArrayType:
		return new UByteArray();
		break;
	case Array::UShortArrayType:
		return new UShortArray();
		break;
	case Array::UIntArrayType:
		return new UIntArray();
		break;
	case Array::Vec4ubArrayType:
		return new Vec4ubArray();
		break;
	case Array::FloatArrayType:
		return new FloatArray();
		break;
	case Array::Vec2ArrayType:
		return new Vec2Array();
		break;
	case Array::Vec3ArrayType:
		return new Vec3Array();
		break;
	case Array::Vec4ArrayType:
		return new Vec4Array();
		break;
	case Array::Vec2sArrayType:
		return new Vec2sArray();
		break;
	case Array::Vec3sArrayType:
		return new Vec3sArray();
		break;
	case Array::Vec4sArrayType:
		return new Vec4sArray();
		break;
    case Array::Vec2bArrayType:
		return new Vec2bArray();
		break;
	case Array::Vec3bArrayType:
		return new Vec3bArray();
		break;
	case Array::Vec4bArrayType:
		return new Vec4bArray();
		break;
    case Array::DoubleArrayType:
		return new DoubleArray();
		break;
	case Array::Vec2dArrayType:
		return new Vec2dArray();
		break;
	case Array::Vec3dArrayType:
		return new Vec3dArray();
		break;
	case Array::Vec4dArrayType:
		return new Vec4dArray();
		break;  
	case Array::MatrixArrayType:
		return new MatrixfArray();
		break;
	default:
		return NULL;
		break;
	}
}


template<class ArrayType> void _addElementTo(osg::ref_ptr<osg::Array> dst, osg::ref_ptr<osg::Array> src, size_t element)
{
	// cast the arrays to the right type
	ArrayType* source = dynamic_cast<ArrayType*>(src.get());
	ArrayType* destination = dynamic_cast<ArrayType*>(dst.get());

	destination->push_back(source->at(element));
}

void ConvertToPopGeometryVisitor::addElementTo(osg::ref_ptr<osg::Array> dst, osg::ref_ptr<osg::Array> src, size_t element)
{
	// assert that we have an source and destination array, that they have the same type and we access an existing element
	if (!dst || ! src) { return; }
	if (dst->getType() != src->getType()) { return; }
	if (element >= src->getNumElements()) { return; }

	switch(src->getType())
	{
	case Array::ByteArrayType:
		_addElementTo<ByteArray>(dst, src, element);
		break;
	case Array::ShortArrayType:
		_addElementTo<ShortArray>(dst, src, element);
		break;
	case Array::IntArrayType:
		_addElementTo<IntArray>(dst, src, element);
		break;
	case Array::UByteArrayType:
		_addElementTo<UByteArray>(dst, src, element);
		break;
	case Array::UShortArrayType:
		_addElementTo<UShortArray>(dst, src, element);
		break;
	case Array::UIntArrayType:
		_addElementTo<UIntArray>(dst, src, element);
		break;
	case Array::Vec4ubArrayType:
		_addElementTo<Vec4ubArray>(dst, src, element);
		break;
	case Array::FloatArrayType:
		_addElementTo<FloatArray>(dst, src, element);
		break;
	case Array::Vec2ArrayType:
		_addElementTo<Vec2Array>(dst, src, element);
		break;
	case Array::Vec3ArrayType:
		_addElementTo<Vec3Array>(dst, src, element);
		break;
	case Array::Vec4ArrayType:
		_addElementTo<Vec4Array>(dst, src, element);
		break;
	case Array::Vec2sArrayType:
		_addElementTo<Vec2sArray>(dst, src, element);
		break;
	case Array::Vec3sArrayType:
		_addElementTo<Vec3sArray>(dst, src, element);
		break;
	case Array::Vec4sArrayType:
		_addElementTo<Vec4sArray>(dst, src, element);
		break;
    case Array::Vec2bArrayType:
		_addElementTo<Vec2bArray>(dst, src, element);
		break;
	case Array::Vec3bArrayType:
		_addElementTo<Vec3bArray>(dst, src, element);
		break;
	case Array::Vec4bArrayType:
		_addElementTo<Vec4bArray>(dst, src, element);
		break;
    case Array::DoubleArrayType:
		_addElementTo<DoubleArray>(dst, src, element);
		break;
	case Array::Vec2dArrayType:
		_addElementTo<Vec2dArray>(dst, src, element);
		break;
	case Array::Vec3dArrayType:
		_addElementTo<Vec3dArray>(dst, src, element);
		break;
	case Array::Vec4dArrayType:
		_addElementTo<Vec4dArray>(dst, src, element);
		break;  
	case Array::MatrixArrayType:
		_addElementTo<MatrixfArray>(dst, src, element);
		break;
	}
}

template<class ArrayType> void _mergeArrays(osg::ref_ptr<osg::Array> first, osg::ref_ptr<osg::Array> second)
{
	ArrayType* _first = dynamic_cast<ArrayType*>(first.get());
	ArrayType* _second = dynamic_cast<ArrayType*>(second.get());

	_first->reserve(_first->size() + _second->size());
	_first->insert(_first->end(), _second->begin(), _second->end());
	_first->dirty();
}

void ConvertToPopGeometryVisitor::mergeArrays(osg::ref_ptr<osg::Array> first, osg::ref_ptr<osg::Array> second)
{
	// assert that we have an source and destination array, and that they have the same type
	if (!first || ! second) { return; }
	if (first->getType() != second->getType()) { return; }

	switch(first->getType())
	{
	case Array::ByteArrayType:
		_mergeArrays<ByteArray>(first, second);
		break;
	case Array::ShortArrayType:
		_mergeArrays<ShortArray>(first, second);
		break;
	case Array::IntArrayType:
		_mergeArrays<IntArray>(first, second);
		break;
	case Array::UByteArrayType:
		_mergeArrays<UByteArray>(first, second);
		break;
	case Array::UShortArrayType:
		_mergeArrays<UShortArray>(first, second);
		break;
	case Array::UIntArrayType:
		_mergeArrays<UIntArray>(first, second);
		break;
	case Array::Vec4ubArrayType:
		_mergeArrays<Vec4ubArray>(first, second);
		break;
	case Array::FloatArrayType:
		_mergeArrays<FloatArray>(first, second);
		break;
	case Array::Vec2ArrayType:
		_mergeArrays<Vec2Array>(first, second);
		break;
	case Array::Vec3ArrayType:
		_mergeArrays<Vec3Array>(first, second);
		break;
	case Array::Vec4ArrayType:
		_mergeArrays<Vec4Array>(first, second);
		break;
	case Array::Vec2sArrayType:
		_mergeArrays<Vec2sArray>(first, second);
		break;
	case Array::Vec3sArrayType:
		_mergeArrays<Vec3sArray>(first, second);
		break;
	case Array::Vec4sArrayType:
		_mergeArrays<Vec4sArray>(first, second);
		break;
    case Array::Vec2bArrayType:
		_mergeArrays<Vec2bArray>(first, second);
		break;
	case Array::Vec3bArrayType:
		_mergeArrays<Vec3bArray>(first, second);
		break;
	case Array::Vec4bArrayType:
		_mergeArrays<Vec4bArray>(first, second);
		break;
    case Array::DoubleArrayType:
		_mergeArrays<DoubleArray>(first, second);
		break;
	case Array::Vec2dArrayType:
		_mergeArrays<Vec2dArray>(first, second);
		break;
	case Array::Vec3dArrayType:
		_mergeArrays<Vec3dArray>(first, second);
		break;
	case Array::Vec4dArrayType:
		_mergeArrays<Vec4dArray>(first, second);
		break;  
	case Array::MatrixArrayType:
		_mergeArrays<MatrixfArray>(first, second);
		break;
	}
}

}