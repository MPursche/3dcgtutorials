#version 150 compatibility
uniform sampler2D colorTexture;
uniform sampler2D normalTexture;

smooth in vec2 texCoord;
smooth in vec3 light;
smooth in vec3 halfVector;
smooth in vec3 normal;
smooth in vec3 eye;

void main()
{
	vec4 textureColor = texture(colorTexture, texCoord);
	vec3 textureNormal = texture(normalTexture, texCoord).xyz * 2.0 - 1.0;

	gl_FragColor.rgb = textureColor.rgb * clamp(dot(textureNormal, light), 0.0, 1.0) +
					   vec3(0.4, 0.4, 0.4) * clamp(pow(dot(textureNormal, halfVector), 15.0), 0.0, 1.0);
	gl_FragColor.a = textureColor.a * abs(dot(normalize(normal), normalize(eye)));
}