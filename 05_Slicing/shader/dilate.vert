#version 150 compatibility

smooth out vec2 texCoord;

void main()
{
	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
	texCoord = gl_MultiTexCoord0.st;
}