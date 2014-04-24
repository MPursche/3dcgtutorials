#version 150 compatibility

uniform sampler2D colorTexture;
uniform sampler2D depthTexture;

smooth in vec3 normal;
smooth in vec3 halfVector;
smooth in vec3 viewSpace_lightDir;
smooth in float shadowDepth;

const float texelSize = 0.0009765625;

void main()
{
	
	float visibility = 0.0;
	float screenSizeFactor = gl_FragCoord.z / gl_FragCoord.w;

	for (float offsetY = -1.5; offsetY <= 1.5; offsetY += 1.0)
	{
		for(float offsetX = -1.5; offsetX <= 1.5; offsetX += 1.0)
		{
			float depth = texture2D(depthTexture, gl_TexCoord[1].xy + (vec2(offsetX, offsetY) * texelSize * screenSizeFactor)).x;
			visibility += (shadowDepth <= (depth + 0.5)) ? 0.0625 : 0.00625;
		}
	}

	float specular = clamp(pow(dot(normalize(halfVector), normalize(normal)), 35.0), 0.0, 1.0);
	float diffuse = clamp(dot(normalize(normal), normalize(viewSpace_lightDir)), 0.0, 1.0);
	vec3 color = vec3(0.6, 0.6, 0.6) * diffuse +
	             texture2D(colorTexture, gl_TexCoord[0].st).rgb * specular;

	gl_FragColor.rgb = vec3(0.1, 0.1, 0.1)  + visibility * color.rgb;
	gl_FragColor.a   = 1.0;
}