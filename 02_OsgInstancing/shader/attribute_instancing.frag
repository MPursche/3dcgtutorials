#version 150 compatibility

uniform vec4 ambientLightColor;
uniform vec4 diffuseLightColor;
uniform sampler2D colorTexture;

smooth in vec2 texCoord;
smooth in vec3 normal;
smooth in vec3 lightDir;

void main()
{
	vec3 fragNormal = normalize(normal);
	fragNormal = gl_FrontFacing ? fragNormal : -1.0 * fragNormal;
	float diffuseFactor = clamp(dot(fragNormal, normalize(lightDir)), 0.0, 1.0);
	vec4 textureColor = texture2D(colorTexture, texCoord);

	gl_FragColor = vec4(textureColor.rgb * diffuseLightColor.rgb * diffuseFactor +
						textureColor.rgb * ambientLightColor.rgb, textureColor.a);
}