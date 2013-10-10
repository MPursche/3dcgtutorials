#version 150 compatibility
smooth in float depth;

void main()
{
	gl_FragColor = vec4(depth, 0.0, 0.0, 1.0);
}