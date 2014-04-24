#version 150 compatibility
uniform sampler2D colorTexture;
uniform bool textureActive;

smooth in vec4 color;
smooth in vec3 normal;
smooth in vec2 texCoord;
smooth in float linearDepth;

out vec4 outColor;
out vec4 outNormal;
out vec4 outDepth;

void main()
{
	vec4 textureColor = textureActive ? texture2D(colorTexture, texCoord) : vec4(1.0);

	outColor = color * textureColor;
	outNormal = vec4(normal * 0.5 + 0.5, 1.0);
	outDepth = vec4(linearDepth, linearDepth, linearDepth, 1.0);
}