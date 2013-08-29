#version 150 compatibility

in vec3 vPosition;
in vec3 vNormal;
in vec2 vTexCoord;
in mat4 vInstanceModelMatrix;

smooth out vec2 texCoord;
smooth out vec3 normal;
smooth out vec3 lightDir;

void main()
{
	gl_Position = gl_ModelViewProjectionMatrix * vInstanceModelMatrix * vec4(vPosition, 1.0);
	texCoord = vTexCoord;

	mat3 normalMatrix = mat3(vInstanceModelMatrix[0][0], vInstanceModelMatrix[0][1], vInstanceModelMatrix[0][2],
							 vInstanceModelMatrix[1][0], vInstanceModelMatrix[1][1], vInstanceModelMatrix[1][2],
							 vInstanceModelMatrix[2][0], vInstanceModelMatrix[2][1], vInstanceModelMatrix[2][2]);

	normal = gl_NormalMatrix * normalMatrix * vNormal;
	lightDir = gl_LightSource[0].position.xyz;
}