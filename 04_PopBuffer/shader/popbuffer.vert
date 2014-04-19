#version 150 compatibility

uniform vec3 osg_MinBounds;
uniform vec3 osg_MaxBounds;
uniform float osg_VertexLod;
uniform int osg_ProtectedVertices;
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

const vec4 ambientLight[3] = vec4[3](vec4(0.1, 0.1, 0.1, 1.0), vec4(0.1, 0.1, 0.1, 1.0), vec4(0.0, 0.1, 0.1, 1.0));
const vec4 diffuseLight[3] = vec4[3](vec4(0.6, 0.6, 0.6, 1.0), vec4(0.6, 0.6, 0.6, 1.0), vec4(0.0, 0.6, 0.6, 1.0));
const vec4 specularLight[3] = vec4[3](vec4(0.9, 0.9, 0.9, 1.0), vec4(0.9, 0.9, 0.9, 1.0), vec4(0.0, 0.5, 0.5, 1.0));

vec4 lodColor(float lod)
{
	return mix(vec4(0.0, 1.0, 0.0, 1.0), vec4(1.0, 0.0, 0.0, 1.0), min(lod, 16.0)/16.0);
}

vec4 quantizeVertex(vec4 vertex)
{
	if (gl_VertexID < osg_ProtectedVertices)
	{
		return vertex;
	}
	else
	{
		float factor = (pow(2.0, osg_VertexLod) - 1.0f) / (osg_MaxBounds.x-osg_MinBounds.x);
		float invFactor = (osg_MaxBounds.x-osg_MinBounds.x) / pow(2.0, osg_VertexLod);
		uvec3 q_vertex = uvec3(factor * (vertex.xyz-osg_MinBounds) + 0.5);
		return vec4(invFactor * vec3(q_vertex) + osg_MinBounds, 1.0);
	}
}

void main()
{
	vec4 vertex = quantizeVertex(gl_Vertex);
	gl_Position = gl_ModelViewProjectionMatrix * vertex;
	
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