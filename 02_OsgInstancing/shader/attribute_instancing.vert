#version 150 compatibility

uniform mat4 osg_ModelViewProjectionMatrix;
uniform mat3 osg_NormalMatrix;
uniform vec3 lightDirection;

in vec3 vPosition;
in vec3 vNormal;
in vec2 vTexCoord;
in mat4 vInstanceModelMatrix;

smooth out vec2 texCoord;
smooth out vec3 normal;
smooth out vec3 lightDir;

void main()
{
	gl_Position = osg_ModelViewProjectionMatrix * vInstanceModelMatrix * vec4(vPosition, 1.0);
	texCoord = vTexCoord;

	mat3 instanceNormalMatrix = mat3(vInstanceModelMatrix[0][0], vInstanceModelMatrix[0][1], vInstanceModelMatrix[0][2],
									 vInstanceModelMatrix[1][0], vInstanceModelMatrix[1][1], vInstanceModelMatrix[1][2],
									 vInstanceModelMatrix[2][0], vInstanceModelMatrix[2][1], vInstanceModelMatrix[2][2]);

	normal = osg_NormalMatrix * instanceNormalMatrix * vNormal;
	lightDir = lightDirection;
}