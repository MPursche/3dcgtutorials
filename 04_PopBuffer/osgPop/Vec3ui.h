#pragma once

namespace osgPop
{

/**
	@brief Helper class for vertices with discrete position 
*/
class Vec3ui {
public:
	Vec3ui()
		: _x(0)
		, _y(0)
		, _z(0)
	{}

	Vec3ui(unsigned int v)
		: _x(v)
		, _y(v)
		, _z(v)
	{}

	Vec3ui(unsigned int v1, unsigned int v2, unsigned int v3)
		: _x(v1)
		, _y(v2)
		, _z(v3)
	{}

	bool operator==(const Vec3ui& rhs) const
	{
		return (_x == rhs._x) && (_y == rhs._y) && (_z == rhs._z);
	}

	bool operator!=(const Vec3ui& rhs) const
	{
		return !(*this == rhs);
	}

	unsigned int& x() { return _x; }
	unsigned int& y() { return _y; }
	unsigned int& z() { return _z; }

    unsigned int x() const { return _x; }
	unsigned int y() const { return _y; }
	unsigned int z() const { return _z; }
protected:
	unsigned int _x;
	unsigned int _y;
	unsigned int _z;
};


/**
 @brief quantizes vertex position with different bit precissions(used for vertex clustering)
*/
template<class Vector> Vec3ui quantize(int bits, float min, float max, const Vector& vertex)
{
	float factor = (pow(2.0f, bits) - 1.0f) / (max-min);

	return Vec3ui(	unsigned int(factor * (vertex.x() - min) + 0.5f),
					unsigned int(factor * (vertex.y() - min) + 0.5f),
					unsigned int(factor * (vertex.z() - min) + 0.5f));
}

template<class Vector> Vector dequantize(int bits, float min, float max, const Vec3ui& vertex)
{
	float invFactor = (max-min)/pow(2.0f, bits);

	return Vector(  invFactor * vertex.x() + min,
					invFactor * vertex.y() + min,
					invFactor * vertex.z() + min);
}

}