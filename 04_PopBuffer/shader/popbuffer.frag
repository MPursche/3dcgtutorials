#version 150 compatibility
uniform sampler2D colorTexture;
uniform bool textureActive;

smooth in vec3 normal;
smooth in vec3 halfVector;
smooth in vec3 viewSpace_lightDir;
smooth in vec2 texCoord;
smooth in vec4 diffuse;
smooth in vec4 specular;
smooth in float shininess;
smooth in vec4 ambient;

void main()
{
	vec4 textureColor = textureActive ? texture2D(colorTexture, texCoord) : vec4(1.0);

	float NdotH = clamp(pow(dot(normalize(halfVector), normalize(normal)), shininess), 0.0, 1.0);
	float NdotL = clamp(dot(normalize(normal), normalize(viewSpace_lightDir)), 0.0, 1.0);
	vec3 color = diffuse.rgb * textureColor.rgb * NdotL +
	             specular.rgb * NdotH +
				 ambient.rgb * textureColor.rgb;

	gl_FragColor.rgb = clamp(color.rgb, vec3(0.0), vec3(1.0));
	gl_FragColor.a   = 1.0;
}