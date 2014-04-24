#version 150 compatibility
smooth out float depth;

void main()
{
	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
	vec4 viewspacePosition = gl_ModelViewMatrix * gl_Vertex;
	depth = -viewspacePosition.z;
}