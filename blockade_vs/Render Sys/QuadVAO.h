#pragma once
#include "VoxelRenderBase.h"
#include "OpenGL Layer/ShaderProgram.h"

class QuadVAO
{
public:
	QuadVAO();
	int initialize(int maxQuadAmount); //0 is failed
	void draw(int offset, int amount); //assumes correct vao is bound
	void draw(int offset); //assumes correct vao is bound
	void pushQuad(__m128 x, __m128 y, __m128 z, __m128 w, GLuint color); //pushed vertices are in projection space right before perspective divide
	void pushBoxFace(bareVec4f &min, bareVec4f &max, bool posOrNeg, int axis, GLuint &color);
	void reset(mat4 &vp);
	void dummyReset(bool stfu);
	~QuadVAO();
	int pushCount = 0;
	bool mapped = false;
	GLuint quadVAO;
	GLuint indexBuffer;
	GLuint quadBufferGPU;
	static ShaderProgram program; //how to make this static without linker error?
	static int uniformVP;
	int maxQuadAmount;
	uintptr_t quadBufferCPU;
	__m128 *xBufferPos, *yBufferPos, *zBufferPos, *wBufferPos, *posBufferPos;
	GLuint *colorBufferPos;
	int xOffset, yOffset, zOffset, wOffset, colorOffset, posOffset;

private:
	int initVAO(); //0 is failed 
	QuadVAO(const QuadVAO& other) = delete; // non construction-copyable
	QuadVAO& operator=(const QuadVAO&) = delete; // non copyable
};

