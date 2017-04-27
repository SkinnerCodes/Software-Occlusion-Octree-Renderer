#version 330

flat in vec4 color;

out vec4 outputColor;

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
	
	outputColor = vec4(color.xyz * (1.8 - (gl_FragCoord.z * gl_FragCoord.z * gl_FragCoord.z * gl_FragCoord.z * gl_FragCoord.z)), 1);
}
