/*
	The MIT License (MIT)

	Copyright (c) 2013 Marcel Pursche

	Permission is hereby granted, free of charge, to any person obtaining a copy of
	this software and associated documentation files (the "Software"), to deal in
	the Software without restriction, including without limitation the rights to
	use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
	the Software, and to permit persons to whom the Software is furnished to do so,
	subject to the following conditions:

	The above copyright notice and this permission notice shall be included in all
	copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
	FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
	COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
	IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
	CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "ASCFileLoader.h"

// std
#include <iostream>
#include <osgDB/fstream>
#include <cmath>

namespace osgExample
{

ASCFileLoader::ASCFileLoader()
	:	m_heightMap(NULL),
	m_width(0u),
	m_height(0u)
{
}

ASCFileLoader::~ASCFileLoader()
{
	if (m_heightMap)
		delete[] m_heightMap;
}

void ASCFileLoader::loadFromFile(const std::string& fileName)
{
	std::ifstream fileStream;
	try
	{
        fileStream.open(fileName.c_str(), std::ios::in);

		// first we try to parse width and height
		m_width = 0u;
		m_height = 0u;
		fileStream >> m_width >> m_height;

		// make sure we have a valid file
		if (!m_width || !m_height)
			return;

		// delete old heightmap and create new one
		if (m_heightMap)
			delete[] m_heightMap;

		m_heightMap = new float[m_width * m_height];

		float* iterator = m_heightMap;

		for (unsigned int y = 0; y < m_height; ++y)
		{
			for (unsigned int x = 0; x < m_width; ++x)
			{
				fileStream >> *iterator;
				++iterator;
			}
		}
	} catch(std::exception e) {
		std::cout << "Error could not load file: " << fileName << std::endl;
	}
	fileStream.close();
}

float ASCFileLoader::getNearestHeight(float x, float y) const
{
	// make sure we have a loaded file
	if (!m_heightMap)
		return 0.0f;
	
	// get nearest sample coordinate and clamp it to [0-width],[0-height]
	int nearestX = (int)floor(x + 0.5f);
	int nearestY = (int)floor(y + 0.5f);
	nearestX = std::max(std::min(nearestX, (int)m_width-1), 0);
	nearestY = std::max(std::min(nearestY, (int)m_height-1), 0);

	return m_heightMap[nearestX+nearestY*m_width];
}

}
