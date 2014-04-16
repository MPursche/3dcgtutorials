#version 150 compatibility

uniform vec3 minBounds;
uniform vec3 maxBounds;
uniform float lod;
uniform vec4 lightPosition;
uniform int fixedVertices;
uniform bool visualizeLod;

smooth out vec3 normal;
smooth out vec3 halfVector;
smooth out vec3 viewSpace_lightDir;
smooth out vec2 texCoord;
smooth out vec4 diffuse;
smooth out vec4 specular;
smooth out float shininess;
smooth out vec4 ambient;

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
	viewSpace_lightDir = normalize(gl_NormalMatrix * lightPosition.xyz);
	halfVector = normalize(viewSpace_lightDir + eye);
	texCoord = gl_MultiTexCoord0.st;

	if (visualizeLod)
	{
		diffuse =  lodColor(lod);
		specular =  gl_FrontMaterial.specular * gl_LightSource[0].specular;
		shininess = gl_FrontMaterial.shininess;
		ambient =  gl_FrontMaterial.ambient * gl_LightSource[0].ambient;
	} else {
		diffuse = gl_FrontMaterial.diffuse * gl_LightSource[0].diffuse;
		//diffuse = (gl_VertexID < fixedVertices) ? vec4(1.0, 0.0, 0.0, 1.0) : vec4(0.0, 0.0, 0.0, 1.0);
		specular = gl_FrontMaterial.specular * gl_LightSource[0].specular;
		//specular = (gl_VertexID < fixedVertices) ? vec4(1.0, 0.0, 0.0, 1.0) : vec4(0.0, 0.0, 0.0, 1.0);
		shininess = gl_FrontMaterial.shininess;
		ambient = gl_FrontMaterial.ambient * gl_LightSource[0].ambient;
		//ambient = (gl_VertexID < fixedVertices) ? vec4(1.0, 0.0, 0.0, 1.0) : vec4(0.0, 0.0, 0.0, 1.0);
	}
}