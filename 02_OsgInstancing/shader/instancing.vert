#version 150 compatibility
uniform mat4 instanceModelMatrix[MAX_INSTANCES];
uniform mat4 osg_ModelViewProjectionMatrix;
uniform mat3 osg_NormalMatrix;
uniform vec3 lightDirection;

smooth out vec2 texCoord;
smooth out vec3 normal;
smooth out vec3 lightDir;

void main()
{
	mat4 _instanceModelMatrix = instanceModelMatrix[gl_InstanceID];
	gl_Position = osg_ModelViewProjectionMatrix * _instanceModelMatrix * gl_Vertex;
	texCoord = gl_MultiTexCoord0.xy;

	mat3 normalMatrix = mat3(_instanceModelMatrix[0][0], _instanceModelMatrix[0][1], _instanceModelMatrix[0][2],
							 _instanceModelMatrix[1][0], _instanceModelMatrix[1][1], _instanceModelMatrix[1][2],
							 _instanceModelMatrix[2][0], _instanceModelMatrix[2][1], _instanceModelMatrix[2][2]);

	normal   = osg_NormalMatrix * normalMatrix * gl_Normal;
	lightDir = lightDirection;
}