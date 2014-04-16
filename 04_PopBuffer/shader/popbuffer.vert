#version 150 compatibility

uniform vec3 minBounds;
uniform vec3 maxBounds;
uniform float lod;
uniform vec4 lightDirection[3];
uniform int fixedVertices;
uniform bool visualizeLod;

smooth out vec3 normal;
smooth out vec3 halfVector[3];
smooth out vec3 viewSpace_lightDir[3];
smooth out vec2 texCoord;

smooth out vec4 ambient[3];
smooth out vec4 diffuse[3];
smooth out vec4 specular[3];
smooth out float shininess;

const vec4 ambientLight[3] = vec4[3](vec4(0.1, 0.1, 0.1, 1.0), vec4(0.1, 0.1, 0.1, 1.0), vec4(0.0, 0.1, 0.1, 1.0));
const vec4 diffuseLight[3] = vec4[3](vec4(0.6, 0.6, 0.6, 1.0), vec4(0.6, 0.6, 0.6, 1.0), vec4(0.0, 0.6, 0.6, 1.0));
const vec4 specularLight[3] = vec4[3](vec4(0.9, 0.9, 0.9, 1.0), vec4(0.9, 0.9, 0.9, 1.0), vec4(0.0, 0.5, 0.5, 1.0));

vec4 lodColor(float lod)
{
	return mix(vec4(0.0, 1.0, 0.0, 1.0), vec4(1.0, 0.0, 0.0, 1.0), min(lod, 16.0)/16.0);
}

void main()
{
	vec3 vertex;
	if (gl_VertexID < fixedVertices)
	{
		vertex = gl_Vertex.xyz;
	}
	else
	{
		float factor = (pow(2.0, lod) - 1.0f) / (maxBounds.x-minBounds.x);
		float invFactor = (maxBounds.x-minBounds.x) / pow(2.0, lod);
		uvec3 q_vertex = uvec3(factor * (gl_Vertex.xyz-minBounds) + 0.5);
		vertex = invFactor * vec3(q_vertex) + minBounds;
	}

	gl_Position    = gl_ModelViewProjectionMatrix * vec4(vertex, 1.0);
	
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
			diffuse[i] =  lodColor(lod);
			specular[i] =  gl_FrontMaterial.specular * specularLight[i];
		}
		shininess = gl_FrontMaterial.shininess;
	} else {
		for (int i = 0; i < 3; ++i)
		{
			ambient[i] = gl_FrontMaterial.ambient * ambientLight[i];
			diffuse[i] =  gl_FrontMaterial.diffuse * diffuseLight[i];
			specular[i] =  gl_FrontMaterial.specular * specularLight[i];
		}
		shininess = gl_FrontMaterial.shininess;
	}
}