#version 150 compatibility
uniform sampler2D colorTexture;

smooth in vec2 texCoord;

void main()
{
	vec4 textureColor = textureLod(colorTexture, texCoord, 0.0);
	vec2 texelSize = 1.0 / textureSize(colorTexture, 0);

	if (textureColor.a > 0.1)
	{
		gl_FragColor = textureColor;
	} else {
		float weight = 0.0;
		vec3 color = vec3(0.0);

		for (int y = -2; y <= 2; ++y)
		{
			for (int x = -2; x <= 2; ++x)
			{
				vec4 sample = textureLod(colorTexture, texCoord + texelSize * vec2(x, y), 0.0);

				if (sample.a > 0.1)
				{
					color += sample.rgb;
					weight += 1.0;
				}
			}
		} 

		gl_FragColor.rgb = color / weight;
		gl_FragColor.a = 0.0;
	}
}