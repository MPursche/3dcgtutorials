#include "ConvertToLevelOfDetailGeometryVisitor.h"
#include "LevelOfDetailDrawElements.h"
#include "HalfEdge.h"

#include <osg/Array>
#include <osg/Geode>
#include <osg/TriangleIndexFunctor>

#include <iostream>
#include <memory>
#include <cmath>

using namespace std;
using namespace osg;

namespace osgUtil
{

void ConvertToLevelOfDetailGeometryVisitor::apply(Geode& geode)
{
	// convert all geometries to lod geometries
	vector<ref_ptr<LevelOfDetailGeometry> > lodGeometries;
	for (size_t i = 0; i < geode.getNumDrawables(); ++i)
	{
		ref_ptr<Geometry> geometry = dynamic_cast<Geometry*>(geode.getDrawable(i));

		if (geometry)
		{
			lodGeometries.push_back(convert(geometry));			
		}
	}

	// replace geometries of the geode
	geode.removeDrawables(0, geode.getNumDrawables());

	for (auto it: lodGeometries)
	{
		geode.addDrawable(it);
	}
}

ref_ptr<LevelOfDetailGeometry> ConvertToLevelOfDetailGeometryVisitor::convert(ref_ptr<Geometry> geometry) const
{
	// assertions
	if (!geometry) { return NULL; }
	if (!geometry->getVertexArray()) { return NULL; }

	// create lod geometry
	ref_ptr<LevelOfDetailGeometry> lodGeometry = new LevelOfDetailGeometry();

	// merge state sets
	if (geometry->getStateSet())
	{	
		lodGeometry->getOrCreateStateSet()->merge(*geometry->getStateSet());
	}
	
	// compute bounding box and set min and max bounds
	BoundingBox bounds = geometry->getBound();
	float min =  std::min(bounds.xMin(), std::min(bounds.yMin(), bounds.zMin()));
	float max =  std::max(bounds.xMax(), std::max(bounds.yMax(), bounds.zMax()));
	lodGeometry->setMinBounds(min);
	lodGeometry->setMaxBounds(max);

    // collect half edges
    vector<HalfEdge> halfEdges;
    if (!collectHalfEdges(geometry, lodGeometry, &halfEdges)) { return NULL; }

    // find half edges opposites sort protected vertices to the front
	findHalfEdgeOpposite(&halfEdges);    
    findAndSortProtectedVertices(geometry, lodGeometry, &halfEdges);
    
	// collect triangles and create list sorted by LODs
    if (!collectLod(lodGeometry, min, max, lodGeometry->getNumberOfProtectedVertices())) { return NULL; }

    // recompute bounds
	lodGeometry->computeBound();

	return lodGeometry;
}

template<class VertexArray, class Vector> void _collectHalfEdges(osg::ref_ptr<osg::Geometry> geometry,
                                                                 osg::ref_ptr<osg::LevelOfDetailGeometry>  lodGeometry,
                                                                 std::vector<HalfEdge>* halfEdges)
{
	TriangleIndexFunctor<HalfEdgeTriangleCollector<VertexArray, Vector> > triangleCollector;
	triangleCollector._vertexArray = dynamic_cast<VertexArray*>(geometry->getVertexArray());
    triangleCollector._halfEdges = halfEdges;

	for (size_t i = 0; i < geometry->getNumPrimitiveSets(); ++i)
	{
        ref_ptr<PrimitiveSet> primtive = geometry->getPrimitiveSet(i);
        triangleCollector._drawElements = new DrawElementsUInt(GL_TRIANGLES);
        triangleCollector._drawElements->reserve(primtive->getNumIndices());
        primtive->accept(triangleCollector);
        lodGeometry->addPrimitiveSet(triangleCollector._drawElements);
	}
}

bool ConvertToLevelOfDetailGeometryVisitor::collectHalfEdges(osg::ref_ptr<osg::Geometry> geometry,
                                                   osg::ref_ptr<osg::LevelOfDetailGeometry>  lodGeometry,
                                                   std::vector<HalfEdge>*      halfEdges) const
{
    switch(geometry->getVertexArray()->getType())
	{
		case Array::Vec3ArrayType:
		{
            _collectHalfEdges<Vec3Array, Vec3>(geometry, lodGeometry, halfEdges);
		} break;
		case Array::Vec3dArrayType:
		{
			_collectHalfEdges<Vec3dArray, Vec3d>(geometry, lodGeometry, halfEdges);
		} break;
		case Array::Vec3bArrayType:
		{
			_collectHalfEdges<Vec3bArray, Vec3b>(geometry, lodGeometry, halfEdges);
		} break;
		case Array::Vec3sArrayType:
		{
			_collectHalfEdges<Vec3sArray, Vec3s>(geometry, lodGeometry, halfEdges);
		} break;
		default:
			// unknown vertex format
			return false;
			break;
	}

	return true;
}

ref_ptr<PrimitiveSet>  createLevelOfDetailDrawPrimitive(vector<ref_ptr<DrawElementsUInt> >* drawElements, size_t numVertices)
{
    // first merge geometry in a UInt draw element
	ref_ptr<LevelOfDetailDrawElementsUInt> lodDrawElements = new LevelOfDetailDrawElementsUInt(GL_TRIANGLES);

    vector<GLint> lodRange(32);
	for (size_t j = 0; j < drawElements->size(); ++j)
	{
		lodDrawElements->insert(lodDrawElements->end(), drawElements->at(j)->begin(), drawElements->at(j)->end());
        lodRange[j] = lodDrawElements->size();
	}
    lodDrawElements->setLodRanges(lodRange);
    lodDrawElements->setLod(31);

	// then create a draw element that fits the vertexbuffer size
	if (numVertices <= UCHAR_MAX)
	{
		ref_ptr<LevelOfDetailDrawElementsUByte> drawElements = new LevelOfDetailDrawElementsUByte(GL_TRIANGLES);
		drawElements->reserve(lodDrawElements->size());

		for (size_t j = 0; j < lodDrawElements->size(); ++j)
		{
			drawElements->push_back(lodDrawElements->at(j));
		}
        drawElements->setLodRanges(lodRange);
        drawElements->setLod(31);
		
        return drawElements;
	} 
	else if (numVertices <= USHRT_MAX) 
	{
		ref_ptr<LevelOfDetailDrawElementsUShort> drawElements = new LevelOfDetailDrawElementsUShort(GL_TRIANGLES);
		drawElements->reserve(lodDrawElements->size());

		for (size_t j = 0; j < lodDrawElements->size(); ++j)
		{
			drawElements->push_back(lodDrawElements->at(j));
		}
        drawElements->setLodRanges(lodRange);
        drawElements->setLod(31);

		return drawElements;
	}
	else
	{
        return lodDrawElements;
	}
}

template<class VertexArray, class Vector> void _collectLod(ref_ptr<Geometry> geometry,
														   float min,
														   float max,
                                                           int numProtectedVertices)
{
    vector<ref_ptr<PrimitiveSet> > drawElements;
    size_t numVertices = geometry->getVertexArray()->getNumElements();
    for (size_t i = 0; i < geometry->getNumPrimitiveSets(); ++i)
	{

        TriangleIndexFunctor<LodTriangleCollector<VertexArray, Vector> > triangleCollector;
	    triangleCollector._vertexArray = dynamic_cast<VertexArray*>(geometry->getVertexArray());
	    triangleCollector._min = min;
	    triangleCollector._max = max;
        triangleCollector._numProtectedVertices = numProtectedVertices;

        vector<ref_ptr<DrawElementsUInt> > lodDrawElements;
        for (size_t j = 0; j < 32; ++j)
	    {
		    lodDrawElements.push_back(new DrawElementsUInt(GL_TRIANGLES));
	    }
        triangleCollector._lodDrawElements = &lodDrawElements;
	
		
        geometry->getPrimitiveSet(i)->accept(triangleCollector);        
        drawElements.push_back(createLevelOfDetailDrawPrimitive(&lodDrawElements, numVertices));
	}

    // switch draw primitives
    geometry->removePrimitiveSet(0, geometry->getNumPrimitiveSets());

    for (auto primtive: drawElements)
    {
        geometry->addPrimitiveSet(primtive);
    }
}

bool ConvertToLevelOfDetailGeometryVisitor::collectLod(ref_ptr<Geometry> geometry,
											 float min,
											 float max,
                                             int numProtectedVertices) const
{
	switch(geometry->getVertexArray()->getType())
	{
		case Array::Vec3ArrayType:
		{
			_collectLod<Vec3Array, Vec3>(geometry, min, max, numProtectedVertices);
		} break;
		case Array::Vec3dArrayType:
		{
			_collectLod<Vec3dArray, Vec3d>(geometry, min, max, numProtectedVertices);
		} break;
		case Array::Vec3bArrayType:
		{
			_collectLod<Vec3bArray, Vec3b>(geometry, min, max, numProtectedVertices);
		} break;
		case Array::Vec3sArrayType:
		{
			_collectLod<Vec3sArray, Vec3s>(geometry, min, max, numProtectedVertices);
		} break;
		default:
			// unknown vertex format
			return false;
			break;
	}

	return true;
}

void ConvertToLevelOfDetailGeometryVisitor::findHalfEdgeOpposite(vector<HalfEdge>* halfEdges) const
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

void ConvertToLevelOfDetailGeometryVisitor::findAndSortProtectedVertices(ref_ptr<Geometry> geometry, ref_ptr<LevelOfDetailGeometry> lodGeometry, std::vector<HalfEdge>* halfEdges) const
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

	lodGeometry->setVertexArray(protectedVertices);
    if (normalArray && normalArray->getBinding() == Array::BIND_PER_VERTEX) { lodGeometry->setNormalArray(protectedNormals); }
    if (colorArray && colorArray->getBinding() == Array::BIND_PER_VERTEX) { lodGeometry->setColorArray(protectedColors); }
    if (secondaryColorArray && secondaryColorArray->getBinding() == Array::BIND_PER_VERTEX) { lodGeometry->setSecondaryColorArray(protectedSecondaryColors); }
    if (fogCoordArray && fogCoordArray->getBinding() == Array::BIND_PER_VERTEX) { lodGeometry->setFogCoordArray(protectedFogCoords); }

    for (size_t j = 0; j < protectedTexCoords.size(); ++j)
	{
        if (texCoordArrays[j] && texCoordArrays[j]->getBinding() == Array::BIND_PER_VERTEX) { lodGeometry->setTexCoordArray(j, protectedTexCoords[j]); }
	}
	for (size_t j = 0; j < protectedVertexAttribs.size(); ++j)
	{
        if (vertexAttribArrays[j] && vertexAttribArrays[j]->getBinding() == Array::BIND_PER_VERTEX) { lodGeometry->setVertexAttribArray(j, protectedVertexAttribs[j]); }
	}


    for (size_t i = 0; i < lodGeometry->getNumPrimitiveSets(); ++i)
    {
        ref_ptr<DrawElementsUInt> lodDrawElements = dynamic_cast<DrawElementsUInt*>(lodGeometry->getPrimitiveSet(i));
		
        // translate vertexIDs to new ID
	    for (size_t j = 0; j < lodDrawElements->size(); ++j)
	    {
		    auto protectedIt = protectedVertexIDMap.find(lodDrawElements->at(j));
		    auto normalIt = regularVertexIDMap.find(lodDrawElements->at(j));

		    if (protectedIt != protectedVertexIDMap.end())
		    {
			    lodDrawElements->at(j) = protectedIt->second;
		    }
		    else if (normalIt != regularVertexIDMap.end())
		    {
			    lodDrawElements->at(j) = normalIt->second + numFixedVertices;
		    }
		    else
		    {
			    // this should never happen
			    cout << "No valid mapping found for vertex." << endl;
			    break;
		    }
	    }
    }

    // set number of fixed vertices for the shader    
	lodGeometry->setNumberOfProtectedVertices(numFixedVertices);
}

ref_ptr<Array> ConvertToLevelOfDetailGeometryVisitor::createArrayOfType(osg::ref_ptr<osg::Array> rhs) const
{
	if (rhs == NULL)
		return NULL;

    ref_ptr<Array> array = NULL;
	switch (rhs->getType())
	{
	case Array::ByteArrayType:
        array = new ByteArray();
		break;
	case Array::ShortArrayType:
		array = new ShortArray();
		break;
	case Array::IntArrayType:
		array = new IntArray();
		break;
	case Array::UByteArrayType:
		array = new UByteArray();
		break;
	case Array::UShortArrayType:
		array = new UShortArray();
		break;
	case Array::UIntArrayType:
		array = new UIntArray();
		break;
	case Array::Vec4ubArrayType:
		array = new Vec4ubArray();
		break;
	case Array::FloatArrayType:
		array = new FloatArray();
		break;
	case Array::Vec2ArrayType:
		array = new Vec2Array();
		break;
	case Array::Vec3ArrayType:
		array = new Vec3Array();
		break;
	case Array::Vec4ArrayType:
		array = new Vec4Array();
		break;
	case Array::Vec2sArrayType:
		array = new Vec2sArray();
		break;
	case Array::Vec3sArrayType:
		array = new Vec3sArray();
		break;
	case Array::Vec4sArrayType:
		array = new Vec4sArray();
		break;
    case Array::Vec2bArrayType:
		array = new Vec2bArray();
		break;
	case Array::Vec3bArrayType:
		array = new Vec3bArray();
		break;
	case Array::Vec4bArrayType:
		array = new Vec4bArray();
		break;
    case Array::DoubleArrayType:
		array = new DoubleArray();
		break;
	case Array::Vec2dArrayType:
		array = new Vec2dArray();
		break;
	case Array::Vec3dArrayType:
		array = new Vec3dArray();
		break;
	case Array::Vec4dArrayType:
		array = new Vec4dArray();
		break;  
	case Array::MatrixArrayType:
		array = new MatrixfArray();
		break;
	}

    array->setBinding(rhs->getBinding());
    return array;
}


template<class ArrayType> void _addElementTo(osg::ref_ptr<osg::Array> dst, osg::ref_ptr<osg::Array> src, size_t element)
{
	// cast the arrays to the right type
	ArrayType* source = dynamic_cast<ArrayType*>(src.get());
	ArrayType* destination = dynamic_cast<ArrayType*>(dst.get());

	destination->push_back(source->at(element));
}

void ConvertToLevelOfDetailGeometryVisitor::addElementTo(osg::ref_ptr<osg::Array> dst, osg::ref_ptr<osg::Array> src, size_t element) const
{
	// assert that we have an source and destination array, that they have the same type, that we access an existing element, and that the binding type is bind per vertex
	if (!dst || ! src) { return; }
	if (dst->getType() != src->getType()) { return; }
	if (element >= src->getNumElements()) { return; }
    if (dst->getBinding() != Array::BIND_PER_VERTEX) { return; }

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

void ConvertToLevelOfDetailGeometryVisitor::mergeArrays(osg::ref_ptr<osg::Array> first, osg::ref_ptr<osg::Array> second) const
{
	// assert that we have an source and destination array, and that they have the same type
	if (!first || ! second) { return; }
	if (first->getType() != second->getType()) { return; }
    if (first->getBinding() != Array::BIND_PER_VERTEX) { return; }

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