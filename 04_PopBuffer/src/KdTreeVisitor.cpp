#include "KdTreeVisitor.h"

#include <algorithm>

#include <osg/Geode>
#include <osg/TriangleIndexFunctor>

namespace osgExample {

template<class VertexArray, class Vector> struct TriangleCollector
{
	VertexArray* _vertexArray;
	osg::ref_ptr<osg::DrawElementsUInt> _drawElements;

    TriangleCollector()
        : _vertexArray(NULL)
		, _drawElements(NULL)
    {
	}
	                    
    void operator()(unsigned int pos1, unsigned int pos2, unsigned int pos3)
    {
			// skip collapsed triangles
			/*if (_vertexArray->at(pos1) == _vertexArray->at(pos2)  ||
				_vertexArray->at(pos1) == _vertexArray->at(pos3)  ||
				_vertexArray->at(pos2) == _vertexArray->at(pos3))
			{
				return;
			}*/
			
			// add new triangle to draw primitive
            _drawElements->push_back(pos1);
            _drawElements->push_back(pos2);
            _drawElements->push_back(pos3);
    }
};

template<class Vector> struct VectorCompare
{
    VectorCompare(KdTreeVisitor::Axis splitAxis)
        : _splitAxis(splitAxis)
    {
    }

    bool operator() (const Vector& lhs, const Vector& rhs)
    {
        switch(_splitAxis)
        {
        case KdTreeVisitor::X_AXIS:
            return lhs.x() < rhs.x();
        case KdTreeVisitor::Y_AXIS:
            return lhs.y() < rhs.y();
        case KdTreeVisitor::Z_AXIS:
            return lhs.z() < rhs.z();
        default:
            return false;
        }
    }
    
    KdTreeVisitor::Axis _splitAxis;
};

void KdTreeVisitor::apply(osg::Geode& geode)
{
    std::vector<osg::ref_ptr<osg::Drawable> > newDrawables;
    
    // convert all drawables, if necessary
    for (size_t i = 0; i < geode.getNumDrawables(); ++i)
    {
        osg::ref_ptr<osg::Geometry> geometry = dynamic_cast<osg::Geometry*>(geode.getDrawable(i));

        if (geometry)
        {
            if (geometry->getVertexArray() && geometry->getVertexArray()->getNumElements() > m_maxVertices)
            {
                // split geometries and add them to the list of new drawables
                std::vector<osg::ref_ptr<osg::Drawable> > temp = splitGeometry(geometry);
                newDrawables.insert(newDrawables.end(), temp.begin(), temp.end());
            } else {
                // geometry has the right size simply add it
                newDrawables.push_back(geometry);
            }
        } else {
            newDrawables.push_back(geode.getDrawable(i));
        }
    }

    // replace old drawables
    geode.removeDrawables(0, geode.getNumDrawables());
    for (auto drawable: newDrawables)
    {
        geode.addDrawable(drawable);
    }

    traverse(geode);
}

std::vector<osg::ref_ptr<osg::Drawable> > KdTreeVisitor::splitGeometry(osg::ref_ptr<osg::Geometry> geometry, Axis splitAxis)
{
    std::vector<osg::ref_ptr<osg::Drawable> > geometries;

    if (geometry->getVertexArray() && geometry->getVertexArray()->getNumElements() <= m_maxVertices)
    {
        // geometry has the right size, stop the recursion
        geometries.push_back(geometry);
    } else {
        // we need to split the geometry
        osg::ref_ptr<osg::Array> vertexArray = geometry->getVertexArray();
        osg::ref_ptr<osg::Array> normalArray = geometry->getNormalArray();
        osg::ref_ptr<osg::Array> colorArray = geometry->getColorArray();
        osg::ref_ptr<osg::Array> secondaryColorArray = geometry->getSecondaryColorArray();
        osg::ref_ptr<osg::Array> fogCoordArray = geometry->getFogCoordArray();
        osg::Geometry::ArrayDataList& texCoordArrays = geometry->getTexCoordArrayList();
        osg::Geometry::ArrayDataList& vertexAttribArrays = geometry->getVertexAttribArrayList();	
        osg::ref_ptr<osg::DrawElementsUInt> drawElements = collectTriangles(geometry);

        // create two new geomtries and vertex attribute arrays
        osg::ref_ptr<osg::Array> leftVertices = createArrayOfType(vertexArray);
	    osg::ref_ptr<osg::Array> rightVertices = createArrayOfType(vertexArray);
	    osg::ref_ptr<osg::Array> leftNormals = createArrayOfType(normalArray);
	    osg::ref_ptr<osg::Array> rightNormals = createArrayOfType(normalArray);
	    osg::ref_ptr<osg::Array> leftColors = createArrayOfType(colorArray);
	    osg::ref_ptr<osg::Array> rightColors = createArrayOfType(colorArray);
	    osg::ref_ptr<osg::Array> leftSecondaryColors = createArrayOfType(secondaryColorArray);
	    osg::ref_ptr<osg::Array> rightSecondaryColors = createArrayOfType(secondaryColorArray);
	    osg::ref_ptr<osg::Array> leftFogCoords = createArrayOfType(fogCoordArray);
	    osg::ref_ptr<osg::Array> rightFogCoords = createArrayOfType(fogCoordArray);
	    std::vector<osg::ref_ptr<osg::Array> > leftTexCoords;
	    std::vector<osg::ref_ptr<osg::Array> > rightTexCoords;
	    for (auto texCoordArray: texCoordArrays)
	    {
		    leftTexCoords.push_back(createArrayOfType(texCoordArray.array));
		    rightTexCoords.push_back(createArrayOfType(texCoordArray.array));
	    }
	    std::vector<osg::ref_ptr<osg::Array> > leftVertexAttribs;
	    std::vector<osg::ref_ptr<osg::Array> > rightVertexAttribs;
	    for (auto vertexAttribArray: vertexAttribArrays)
	    {
		    leftVertexAttribs.push_back(createArrayOfType(vertexAttribArray.array));
		    rightVertexAttribs.push_back(createArrayOfType(vertexAttribArray.array));
	    }

        osg::ref_ptr<osg::Geometry> left = new osg::Geometry;
        left->setStateSet(geometry->getStateSet());
        osg::ref_ptr<osg::DrawElementsUInt> leftDrawElements = new osg::DrawElementsUInt(GL_TRIANGLES);
        leftDrawElements->reserve(drawElements->size() / 2);
        left->addPrimitiveSet(leftDrawElements);
        std::map<unsigned int, unsigned int> leftIndicesMapping;
        left->setVertexArray(leftVertices);
        left->setNormalArray(leftNormals);
        left->setNormalBinding(geometry->getNormalBinding());
        left->setColorArray(leftColors);
        left->setColorBinding(geometry->getColorBinding());
        left->setSecondaryColorArray(leftSecondaryColors);
        left->setSecondaryColorBinding(geometry->getSecondaryColorBinding());
        left->setFogCoordArray(leftFogCoords);
	    left->setFogCoordBinding(geometry->getFogCoordBinding());
        for (size_t j = 0; j < leftTexCoords.size(); ++j)
	    {
		    left->setTexCoordArray(j, leftTexCoords[j]);
	    }
        for (size_t j = 0; j < leftVertexAttribs.size(); ++j)
	    {
		    left->setVertexAttribArray(j, leftVertexAttribs[j]);
		    left->setVertexAttribBinding(j, geometry->getVertexAttribBinding(j));
	    }

        osg::ref_ptr<osg::Geometry> right = new osg::Geometry;
        right->setStateSet(geometry->getStateSet());
        osg::ref_ptr<osg::DrawElementsUInt> rightDrawElements = new osg::DrawElementsUInt(GL_TRIANGLES);
        rightDrawElements->reserve(drawElements->size() / 2);
        right->addPrimitiveSet(rightDrawElements);
        std::map<unsigned int, unsigned int> rightIndicesMapping;
        right->setVertexArray(rightVertices);
        right->setNormalArray(rightNormals);
        right->setNormalBinding(geometry->getNormalBinding());
        right->setColorArray(rightColors);
        right->setColorBinding(geometry->getColorBinding());
        right->setSecondaryColorArray(rightSecondaryColors);
        right->setSecondaryColorBinding(geometry->getSecondaryColorBinding());
        right->setFogCoordArray(rightFogCoords);
	    right->setFogCoordBinding(geometry->getFogCoordBinding());
        for (size_t j = 0; j < rightTexCoords.size(); ++j)
	    {
            if (!rightTexCoords[j]) { continue; }
            right->setTexCoordArray(j, rightTexCoords[j]);
	    }
        for (size_t j = 0; j < rightVertexAttribs.size(); ++j)
	    {
		    if (!rightVertexAttribs[j]) { continue; }
            right->setVertexAttribArray(j, rightVertexAttribs[j]);
		    right->setVertexAttribBinding(j, geometry->getVertexAttribBinding(j));
	    }

        // sort vertex array
        osg::ref_ptr<osg::Vec3Array> originalVertices = static_cast<osg::Vec3Array*>(vertexArray.get());
        osg::ref_ptr<osg::Vec3Array> sortedVertices = static_cast<osg::Vec3Array*>(vertexArray->clone(osg::CopyOp::DEEP_COPY_ALL));
        sortByAxis(sortedVertices, splitAxis);

        // find split position
        float splitPosition = 0.0f;
        switch(splitAxis)
        {
        case X_AXIS:
            splitPosition = sortedVertices->at(sortedVertices->getNumElements() / 2).x();
            break;
        case Y_AXIS:
            splitPosition = sortedVertices->at(sortedVertices->getNumElements() / 2).y();
            break;
        case Z_AXIS:
            splitPosition = sortedVertices->at(sortedVertices->getNumElements() / 2).z();
            break;
        }

        VectorCompare<osg::Vec3> compare(splitAxis);

        // sort triangles in left or right geometry depending on their vertices
        for (size_t i = 0 ; i < drawElements->size(); i+=3)
        {
            unsigned int indices[3] = { drawElements->at(i),
                                        drawElements->at(i+1),
                                        drawElements->at(i+2) };

            unsigned int inLeft = 0, inRight = 0;
            for (size_t j = 0; j < 3; ++j)
            {
                if (compare(originalVertices->at(indices[j]), osg::Vec3(splitPosition, splitPosition, splitPosition)))
                {
                    ++inLeft;
                } else {
                    ++inRight;
                }
            }

            // chose left or right geometry depending on the number of vertices that fit in the bucket
            if (inLeft > inRight)
            {
                // check if we need to add vertices
                for (size_t j = 0; j < 3; ++j)
                {
                    auto it = leftIndicesMapping.find(indices[j]);

                    if (it == leftIndicesMapping.end())
                    {
                        // add a new vertex
                        leftIndicesMapping[indices[j]] = leftVertices->getNumElements();

                        addElementTo(leftVertices, vertexArray, indices[j]);
                        addElementTo(leftNormals, normalArray, indices[j]);
                        addElementTo(leftColors, colorArray, indices[j]);
                        addElementTo(leftSecondaryColors, secondaryColorArray, indices[j]);
                        addElementTo(leftFogCoords, fogCoordArray, indices[j]);
                        for (size_t k = 0; k < leftTexCoords.size(); ++k)
                        {
                            addElementTo(leftTexCoords[k], texCoordArrays[k].array, indices[j]);
                        }
                        for (size_t k = 0; k < leftVertexAttribs.size(); ++k)
                        {
                            addElementTo(leftVertexAttribs[k], vertexAttribArrays[k].array, indices[j]);
                        }
                    }
                }

                // add triangle with mapped indices
                leftDrawElements->push_back(leftIndicesMapping[indices[0]]);
                leftDrawElements->push_back(leftIndicesMapping[indices[1]]);
                leftDrawElements->push_back(leftIndicesMapping[indices[2]]);
            } else {
                // check if we need to add vertices
                for (size_t j = 0; j < 3; ++j)
                {
                    auto it = rightIndicesMapping.find(indices[j]);

                    if (it == rightIndicesMapping.end())
                    {
                        // add a new vertex
                        rightIndicesMapping[indices[j]] = rightVertices->getNumElements();

                        addElementTo(rightVertices, vertexArray, indices[j]);
                        addElementTo(rightNormals, normalArray, indices[j]);
                        addElementTo(rightColors, colorArray, indices[j]);
                        addElementTo(rightSecondaryColors, secondaryColorArray, indices[j]);
                        addElementTo(rightFogCoords, fogCoordArray, indices[j]);
                        for (size_t k = 0; k < rightTexCoords.size(); ++k)
                        {
                            addElementTo(rightTexCoords[k], texCoordArrays[k].array, indices[j]);
                        }
                        for (size_t k = 0; k < rightVertexAttribs.size(); ++k)
                        {
                            addElementTo(rightVertexAttribs[k], vertexAttribArrays[k].array, indices[j]);
                        }
                    }
                }

                // add triangle with mapped indices
                rightDrawElements->push_back(rightIndicesMapping[indices[0]]);
                rightDrawElements->push_back(rightIndicesMapping[indices[1]]);
                rightDrawElements->push_back(rightIndicesMapping[indices[2]]);
            }
        }

        // recursivly split them again
        std::vector<osg::ref_ptr<osg::Drawable> > temp1 = splitGeometry(left, (Axis)((splitAxis + 1) % 3));
        std::vector<osg::ref_ptr<osg::Drawable> > temp2 = splitGeometry(right, (Axis)((splitAxis + 1) % 3));
       
        geometries.insert(geometries.end(), temp1.begin(), temp1.end());
        geometries.insert(geometries.end(), temp2.begin(), temp2.end());
    }

    return geometries;
}

template<class VertexArray, class Vector> void _sortByAxis(osg::ref_ptr<osg::Array> sortedVertices, KdTreeVisitor::Axis splitAxis)
{
    osg::ref_ptr<VertexArray> vertices = dynamic_cast<VertexArray*>(sortedVertices.get());

    if (!vertices) { return; }
    
    std::sort(vertices->begin(), vertices->end(), VectorCompare<Vector>(splitAxis));
}


void KdTreeVisitor::sortByAxis(osg::ref_ptr<osg::Array> sortedVertices, Axis splitAxis)
{
    switch(sortedVertices->getType())
	{
        case osg::Array::Vec3ArrayType:
		{
			_sortByAxis<osg::Vec3Array, osg::Vec3>(sortedVertices, splitAxis);
		} break;
		case osg::Array::Vec3dArrayType:
		{
			_sortByAxis<osg::Vec3dArray, osg::Vec3d>(sortedVertices, splitAxis);
		} break;
		case osg::Array::Vec3bArrayType:
		{
			_sortByAxis<osg::Vec3bArray, osg::Vec3b>(sortedVertices, splitAxis);
		} break;
		case osg::Array::Vec3sArrayType:
		{
			_sortByAxis<osg::Vec3sArray, osg::Vec3s>(sortedVertices, splitAxis);
		} break;
		default:
			// unknown vertex format
			break;
	}
}

template<class VertexArray, class Vector> void _collectTriangles(osg::ref_ptr<osg::Geometry> geometry, osg::ref_ptr<osg::DrawElementsUInt> drawElements)
{
	osg::TriangleIndexFunctor<TriangleCollector<VertexArray, Vector> > triangleCollector;
	triangleCollector._vertexArray = dynamic_cast<VertexArray*>(geometry->getVertexArray());
    triangleCollector._drawElements = drawElements;

	for (size_t i = 0; i < geometry->getNumPrimitiveSets(); ++i)
	{
		geometry->getPrimitiveSet(i)->accept(triangleCollector);		
	}
}

osg::ref_ptr<osg::DrawElementsUInt> KdTreeVisitor::collectTriangles(osg::ref_ptr<osg::Geometry> geometry)
{
    osg::ref_ptr<osg::DrawElementsUInt> drawElements = new osg::DrawElementsUInt(GL_TRIANGLES);

    switch(geometry->getVertexArray()->getType())
	{
        case osg::Array::Vec3ArrayType:
		{
			_collectTriangles<osg::Vec3Array, osg::Vec3>(geometry, drawElements);
		} break;
		case osg::Array::Vec3dArrayType:
		{
			_collectTriangles<osg::Vec3dArray, osg::Vec3d>(geometry, drawElements);
		} break;
		case osg::Array::Vec3bArrayType:
		{
			_collectTriangles<osg::Vec3bArray, osg::Vec3b>(geometry, drawElements);
		} break;
		case osg::Array::Vec3sArrayType:
		{
			_collectTriangles<osg::Vec3sArray, osg::Vec3s>(geometry, drawElements);
		} break;
		default:
			// unknown vertex format
			break;
	}

    return drawElements;
}

osg::ref_ptr<osg::Array> KdTreeVisitor::createArrayOfType(osg::ref_ptr<osg::Array> rhs)
{
    if (rhs == NULL)
		return NULL;

	switch (rhs->getType())
	{
    case osg::Array::ByteArrayType:
		return new osg::ByteArray();
		break;
	case osg::Array::ShortArrayType:
		return new osg::ShortArray();
		break;
	case osg::Array::IntArrayType:
		return new osg::IntArray();
		break;
	case osg::Array::UByteArrayType:
		return new osg::UByteArray();
		break;
	case osg::Array::UShortArrayType:
		return new osg::UShortArray();
		break;
	case osg::Array::UIntArrayType:
		return new osg::UIntArray();
		break;
	case osg::Array::Vec4ubArrayType:
		return new osg::Vec4ubArray();
		break;
	case osg::Array::FloatArrayType:
		return new osg::FloatArray();
		break;
	case osg::Array::Vec2ArrayType:
		return new osg::Vec2Array();
		break;
	case osg::Array::Vec3ArrayType:
		return new osg::Vec3Array();
		break;
	case osg::Array::Vec4ArrayType:
		return new osg::Vec4Array();
		break;
	case osg::Array::Vec2sArrayType:
		return new osg::Vec2sArray();
		break;
	case osg::Array::Vec3sArrayType:
		return new osg::Vec3sArray();
		break;
	case osg::Array::Vec4sArrayType:
		return new osg::Vec4sArray();
		break;
    case osg::Array::Vec2bArrayType:
		return new osg::Vec2bArray();
		break;
	case osg::Array::Vec3bArrayType:
		return new osg::Vec3bArray();
		break;
	case osg::Array::Vec4bArrayType:
		return new osg::Vec4bArray();
		break;
    case osg::Array::DoubleArrayType:
		return new osg::DoubleArray();
		break;
	case osg::Array::Vec2dArrayType:
		return new osg::Vec2dArray();
		break;
	case osg::Array::Vec3dArrayType:
		return new osg::Vec3dArray();
		break;
	case osg::Array::Vec4dArrayType:
		return new osg::Vec4dArray();
		break;  
	case osg::Array::MatrixArrayType:
		return new osg::MatrixfArray();
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

void KdTreeVisitor::addElementTo(osg::ref_ptr<osg::Array> dst, osg::ref_ptr<osg::Array> src, size_t element)
{
	// assert that we have an source and destination array, that they have the same type and we access an existing element
	if (!dst || ! src) { return; }
	if (dst->getType() != src->getType()) { return; }
	if (element >= src->getNumElements()) { return; }

	switch(src->getType())
	{
	case osg::Array::ByteArrayType:
		_addElementTo<osg::ByteArray>(dst, src, element);
		break;
	case osg::Array::ShortArrayType:
		_addElementTo<osg::ShortArray>(dst, src, element);
		break;
	case osg::Array::IntArrayType:
		_addElementTo<osg::IntArray>(dst, src, element);
		break;
	case osg::Array::UByteArrayType:
		_addElementTo<osg::UByteArray>(dst, src, element);
		break;
	case osg::Array::UShortArrayType:
		_addElementTo<osg::UShortArray>(dst, src, element);
		break;
	case osg::Array::UIntArrayType:
		_addElementTo<osg::UIntArray>(dst, src, element);
		break;
	case osg::Array::Vec4ubArrayType:
		_addElementTo<osg::Vec4ubArray>(dst, src, element);
		break;
	case osg::Array::FloatArrayType:
		_addElementTo<osg::FloatArray>(dst, src, element);
		break;
	case osg::Array::Vec2ArrayType:
		_addElementTo<osg::Vec2Array>(dst, src, element);
		break;
	case osg::Array::Vec3ArrayType:
		_addElementTo<osg::Vec3Array>(dst, src, element);
		break;
	case osg::Array::Vec4ArrayType:
		_addElementTo<osg::Vec4Array>(dst, src, element);
		break;
	case osg::Array::Vec2sArrayType:
		_addElementTo<osg::Vec2sArray>(dst, src, element);
		break;
	case osg::Array::Vec3sArrayType:
		_addElementTo<osg::Vec3sArray>(dst, src, element);
		break;
	case osg::Array::Vec4sArrayType:
		_addElementTo<osg::Vec4sArray>(dst, src, element);
		break;
    case osg::Array::Vec2bArrayType:
		_addElementTo<osg::Vec2bArray>(dst, src, element);
		break;
	case osg::Array::Vec3bArrayType:
		_addElementTo<osg::Vec3bArray>(dst, src, element);
		break;
	case osg::Array::Vec4bArrayType:
		_addElementTo<osg::Vec4bArray>(dst, src, element);
		break;
    case osg::Array::DoubleArrayType:
		_addElementTo<osg::DoubleArray>(dst, src, element);
		break;
	case osg::Array::Vec2dArrayType:
		_addElementTo<osg::Vec2dArray>(dst, src, element);
		break;
	case osg::Array::Vec3dArrayType:
		_addElementTo<osg::Vec3dArray>(dst, src, element);
		break;
	case osg::Array::Vec4dArrayType:
		_addElementTo<osg::Vec4dArray>(dst, src, element);
		break;  
	case osg::Array::MatrixArrayType:
		_addElementTo<osg::MatrixfArray>(dst, src, element);
		break;
	}
}



}