#version 150 compatibility
uniform sampler2D colorTexture;
uniform bool textureActive;

smooth in vec3 normal;
smooth in vec3 halfVector[3];
smooth in vec3 viewSpace_lightDir[3];
smooth in vec2 texCoord;

smooth in vec4 ambient[3];
smooth in vec4 diffuse[3];
smooth in vec4 specular[3];
smooth in float shininess;

vec4 blinnPhongShading(vec3 n, vec3 h, vec3 l, vec4 cambient, vec4 cdiffuse, vec4 cspecular, float mshininess)
{
	float NdotH = clamp(pow(dot(n, h), mshininess), 0.0, 1.0);
	float NdotL = clamp(dot(n, l), 0.0, 1.0);

	return cambient +
		   cdiffuse * NdotL +
		   cspecular * NdotH;
}

void main()
{
	vec4 textureColor = textureActive ? texture2D(colorTexture, texCoord) : vec4(1.0);

	vec4 color = vec4(0.0);
	for (int i = 0; i < 3; ++i)
	{
		color += blinnPhongShading(normalize(normal), normalize(halfVector[i]), normalize(viewSpace_lightDir[i]), ambient[i] * textureColor, diffuse[i] * textureColor, specular[i], shininess);
	}

	gl_FragColor.rgb = clamp(color.rgb, vec3(0.0), vec3(1.0));
	gl_FragColor.a   = 1.0;
}