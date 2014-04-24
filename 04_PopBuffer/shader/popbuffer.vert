#version 150 compatibility

uniform vec4 lightDirection[3];
uniform bool visualizeLod;

smooth out vec3 normal;
smooth out vec3 halfVector[3];
smooth out vec3 viewSpace_lightDir[3];
smooth out vec2 texCoord;

smooth out vec4 ambient[3];
smooth out vec4 diffuse[3];
smooth out vec4 specular[3];
smooth out float shininess;
smooth out float specularNormilization;

const vec4 ambientLight[3] = vec4[3](vec4(0.1, 0.1, 0.1, 1.0), vec4(0.1, 0.1, 0.1, 1.0), vec4(0.0, 0.1, 0.1, 1.0));
const vec4 light[3] = vec4[3](vec4(0.6, 0.6, 0.6, 1.0), vec4(0.6, 0.6, 0.6, 1.0), vec4(0.0, 0.6, 0.6, 1.0));

vec4 lodColor(float lod)
{
	return mix(vec4(0.0, 1.0, 0.0, 1.0), vec4(1.0, 0.0, 0.0, 1.0), min(lod, 16.0)/16.0);
}

void main()
{
	vec4 vertex = quantizeVertex(gl_Vertex);
	gl_Position = gl_ModelViewProjectionMatrix * vertex;
	//gl_Position = vec4(gl_MultiTexCoord0.st * 2.0 - 1.0, -1.0, 1.0);
	
	normal = normalize(gl_NormalMatrix * gl_Normal);

	vec3 eye = normalize(-(gl_ModelViewMatrix * gl_Vertex).xyz);
	viewSpace_lightDir[0] = normalize(gl_NormalMatrix * lightDirection[0].xyz);
	viewSpace_lightDir[1] = normalize(gl_NormalMatrix * lightDirection[1].xyz);
	viewSpace_lightDir[2] = normalize(gl_NormalMatrix * lightDirection[2].xyz);
	halfVector[0] = normalize(viewSpace_lightDir[0] + eye);
	halfVector[1] = normalize(viewSpace_lightDir[1] + eye);
	halfVector[2] = normalize(viewSpace_lightDir[2] + eye);
	texCoord = gl_MultiTexCoord0.st;

	if (visualizeLod)
	{
		for (int i = 0; i < 3; ++i)
		{
			ambient[i] =  gl_FrontMaterial.ambient * ambientLight[i];
			diffuse[i] =  lodColor(osg_VertexLod);
			specular[i] =  gl_FrontMaterial.specular * light[i];
		}
		shininess = gl_FrontMaterial.shininess;
		specularNormilization = (gl_FrontMaterial.shininess + 8.0) * 0.03978873577297383394222094084313; // shiness + 8 / 8 * PI
	} else {
		for (int i = 0; i < 3; ++i)
		{
			ambient[i] = gl_FrontMaterial.ambient * ambientLight[i];

			vec4 diffuseMaterial = gl_FrontMaterial.diffuse, specularMaterial = gl_FrontMaterial.diffuse;
			/*if (any(greaterThan((diffuseMaterial.rgb + specularMaterial.rgb), vec3(1.0))))
			{
				diffuseMaterial = diffuseMaterial / (diffuseMaterial + specularMaterial);
				specularMaterial = specularMaterial / (diffuseMaterial + specularMaterial);
			}*/

			diffuse[i] =  diffuseMaterial * light[i];
			specular[i] =  specularMaterial * light[i];
		}
		shininess = gl_FrontMaterial.shininess;
		specularNormilization = (gl_FrontMaterial.shininess + 8.0) * 0.03978873577297383394222094084313; // shiness + 8 / 8 * PI
	}
}