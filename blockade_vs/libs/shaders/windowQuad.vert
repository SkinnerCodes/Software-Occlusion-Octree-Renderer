#version 330

//layout(location = 0) in float x;
//layout(location = 1) in float y;
//layout(location = 2) in float z;
//layout(location = 3) in float w;
layout(location = 0) in vec3 worldPos;
layout(location = 4) in vec4 colorIn;

flat out vec4 color;

uniform mat4 viewProjection;

//layout(std140) uniform;
//uniform sampler2D diffuseColorTex;
//uniform Projection
//{
//	mat4 view;
//	mat4 projection;
//	mat4 invViewProjection;
//	ivec4 viewPort;
//};

void main()
{
	//gl position in ndc space already, can't sent it in window space cause it needs to perspective divide (no choice with api)
	//gl_Position = vec4(x,y,z,w);
	
	gl_Position = viewProjection * vec4(worldPos, 1);

	color = colorIn;
}
