#include "QuadVAO.h"
#include <stopwatch.h>
#include "RenderBox.h"


ShaderProgram QuadVAO::program; 
int QuadVAO::uniformVP;

QuadVAO::QuadVAO()
{

}
int QuadVAO::initialize(int maxQuadAmount=100)
{
	static bool firstCall = true;
	if (firstCall) {
		program.addShaderFromFile("shaders\\windowQuad.vert", GL_VERTEX_SHADER);
		program.addShaderFromFile("shaders\\windowQuad.frag", GL_FRAGMENT_SHADER);
		program.linkProgram();
		firstCall = false;
		glUseProgram(program.getProgramID());
		uniformVP = glGetUniformLocation(program.mProgramID, "viewProjection");
	}
	glGenVertexArrays(1, &quadVAO);
	this->maxQuadAmount = maxQuadAmount;
	glBindVertexArray(0);
	glGenBuffers(1, &indexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
	GLuint *data = (GLuint*)malloc(sizeof(GLuint) * maxQuadAmount *6);
	for (int i = 0; i < maxQuadAmount; i++)
	{
		data[i * 6] = i * 4;
		data[i * 6 +1] = i * 4 +1;
		data[i * 6 +2] = i * 4 +2;
		data[i * 6 +3] = i * 4 +2;
		data[i * 6 +4] = i * 4 +3;
		data[i * 6 +5] = i * 4 +0;

	}
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * maxQuadAmount * sizeof(GLuint), data, GL_STREAM_DRAW);
	free(data);
	glGenBuffers(1, &quadBufferGPU);
	glBindBuffer(GL_ARRAY_BUFFER, quadBufferGPU);
	glBufferData(GL_ARRAY_BUFFER, 4 * maxQuadAmount * (sizeof(GLfloat) * 4 + sizeof(GLuint)) +16, NULL, GL_STREAM_DRAW); // plus 16 to ensure we can 16 byte align without overflowing
	xOffset = 0;
	yOffset = maxQuadAmount * sizeof(GLfloat);
	zOffset = maxQuadAmount * sizeof(GLfloat) * 2;
	wOffset = maxQuadAmount * sizeof(GLfloat) * 3;
	colorOffset = maxQuadAmount * sizeof(GLfloat) * 4;
	//todo figure out of this worked with no vao bound
	initVAO();
	return quadVAO;
}
int QuadVAO::initVAO()
{
	glBindVertexArray(quadVAO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, quadBufferGPU);
	glEnableVertexAttribArray(0);
//	glEnableVertexAttribArray(1);
//	glEnableVertexAttribArray(2);
//	glEnableVertexAttribArray(3);
	glEnableVertexAttribArray(4);
	glVertexAttribPointer(0, 1, GL_FLOAT, false, sizeof(GLfloat), (void*)xOffset);
	glVertexAttribPointer(1, 1, GL_FLOAT, false, sizeof(GLfloat), (void*)yOffset);
	glVertexAttribPointer(2, 1, GL_FLOAT, false, sizeof(GLfloat), (void*)zOffset);
	glVertexAttribPointer(3, 1, GL_FLOAT, false, sizeof(GLfloat), (void*)wOffset);
	glVertexAttribPointer(4, 4, GL_UNSIGNED_BYTE, true, sizeof(GLuint), (void*)colorOffset);
	//glVertexAttribDivisor(6, 1);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	return quadVAO;
}
void QuadVAO::reset(mat4 &vp)
{
	glUseProgram(program.getProgramID());
	if (!mapped)
	{
		mapped = true;
		glBindBuffer(GL_ARRAY_BUFFER, quadBufferGPU);

		//orphaning approach, realloc buffer storage with glbufferdata(null) then map entire buffer
		//glBufferData(GL_ARRAY_BUFFER, 4 * maxQuadAmount * (sizeof(GLfloat) * 4 + sizeof(GLuint)) + 16, NULL, GL_STREAM_DRAW); quadBufferCPU = (uintptr_t)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
		//more modern forced async map and orphaning may be involved. looks a lot better but may be slower or faster because of fucked up driver situations
		quadBufferCPU = (uintptr_t)glMapBufferRange(GL_ARRAY_BUFFER, 0, 4 * maxQuadAmount * (sizeof(GLfloat) * 4 + sizeof(GLuint)) + 16, GL_MAP_UNSYNCHRONIZED_BIT | GL_MAP_INVALIDATE_BUFFER_BIT | GL_MAP_WRITE_BIT);

		//fix alignment and bindings
		uintptr_t baseAddress = quadBufferCPU;
		uintptr_t alignedAddress = ((uintptr_t)quadBufferCPU + 15) & ~(uintptr_t)0x0F; //get 16 byte alignment
		int alignDif = alignedAddress - baseAddress;
		posOffset = (4 * maxQuadAmount) * sizeof(GLfloat) * 4 + alignDif; //alignment needs to be factored into offset because GL functions take offsets into the unaligned buffer
		colorOffset = posOffset + (sizeof(GLfloat) * 3);
		posBufferPos = (__m128*)(baseAddress + posOffset);
		colorBufferPos = (GLuint*)(baseAddress + colorOffset);
	}
	else //not remapping buffer, writing into the same mapped buffer, used for testing purposes
	{
		uintptr_t baseAddress = quadBufferCPU;
		posBufferPos = (__m128*)(baseAddress + posOffset);
		colorBufferPos = (GLuint*)(baseAddress + colorOffset);
	}
	pushCount = 0;
	
	//set view projection matrix in shader, this should probably be moved out later //todo
	glUniformMatrix4fv(uniformVP, 1, false, glm::value_ptr(vp));

}
void QuadVAO::dummyReset(bool doAlloc)
{
	if (doAlloc) {
		quadBufferCPU = (uintptr_t)malloc(4 * maxQuadAmount * (sizeof(GLfloat) * 4 + sizeof(GLuint)) + 16);
	}	
	uintptr_t baseAddress = quadBufferCPU;
	uintptr_t alignedAddress = ((uintptr_t)quadBufferCPU + 15) & ~(uintptr_t)0x0F; //get 16 byte alignment
	int alignDif = alignedAddress - baseAddress;
	posOffset = (4 * maxQuadAmount) * sizeof(GLfloat) * 4 + alignDif; //alignment needs to be factored into offset because GL functions take offsets into the unaligned buffer
	colorOffset = posOffset + (sizeof(GLfloat) * 3);
	posBufferPos = (__m128*)(baseAddress + posOffset);
	colorBufferPos = (GLuint*)(baseAddress + colorOffset);
	pushCount = 0;
}
void QuadVAO::pushQuad(__m128 x, __m128 y, __m128 z, __m128 w, GLuint color)
{
}
void QuadVAO::pushBoxFace(bareVec4f &min, bareVec4f &max, bool posOrNeg, int axis, GLuint &color)
{
	pushCount++; //quads have pushed
	int otherAxis, otherOtherAxis;
	otherAxis = (axis ^ 1) & 1; 
	otherOtherAxis = (axis ^ 2) & 2;

	bareVec4f corner;
	corner.set(axis, posOrNeg ? max.get(axis) : min.get(axis));
	corner.set(otherAxis, min.get(otherAxis));
	corner.set(otherOtherAxis, min.get(otherOtherAxis));
	posBufferPos--; colorBufferPos -= 4;
	_mm_store_ps((float*)posBufferPos, corner);
	colorBufferPos[0] = color;

	corner.set(otherAxis, max.get(otherAxis));
	posBufferPos--; colorBufferPos -= 4;
	_mm_store_ps((float*)posBufferPos, corner);
	colorBufferPos[0] = color;

	corner.set(otherOtherAxis, max.get(otherOtherAxis));
	posBufferPos--; colorBufferPos -= 4;
	_mm_store_ps((float*)posBufferPos, corner);
	colorBufferPos[0] = color;

	corner.set(otherAxis, min.get(otherAxis));
	posBufferPos--; colorBufferPos -= 4;
	_mm_store_ps((float*)posBufferPos, corner);
	colorBufferPos[0] = color;
}
void QuadVAO::draw(int firstQuad, int quadAmount) 
{
	glBindVertexArray(quadVAO);
	if (mapped)
	{
		mapped = false;
		glBindBuffer(GL_ARRAY_BUFFER, quadBufferGPU);
		glUnmapBuffer(GL_ARRAY_BUFFER);
		int start = (posOffset - (pushCount * 4 * 4 * sizeof(GLfloat)));
		glVertexAttribPointer(0, 3, GL_FLOAT, false, 4 * sizeof(GLfloat), (void*)start);
		glVertexAttribPointer(4, 4, GL_UNSIGNED_BYTE, true, 4 * sizeof(GLuint), (void*)(start + 3 * sizeof(GLfloat)));
	}
	program.bind();
	glDrawElements(GL_TRIANGLES, 6 * quadAmount, GL_UNSIGNED_INT, (void*)(firstQuad * sizeof(GLuint)));//fourth parameter should be in bytes

}
void QuadVAO::draw(int firstQuad) 
{
	glBindVertexArray(quadVAO);
	if (mapped)
	{
		mapped = false;
		glBindBuffer(GL_ARRAY_BUFFER, quadBufferGPU);
		glUnmapBuffer(GL_ARRAY_BUFFER);

		int start = (posOffset - (pushCount * 4 * 4 * sizeof(GLfloat)));
		glVertexAttribPointer(0, 3, GL_FLOAT, false, 4*sizeof(GLfloat), (void*)start);
		glVertexAttribPointer(4, 4, GL_UNSIGNED_BYTE, true, 4*sizeof(GLuint), (void*)(start + 3 * sizeof(GLfloat)));
	}
	else
	{
		int testcode = 1;
	}
	program.bind();
	if(pushCount > 0)glDrawElements(GL_TRIANGLES, 6 * pushCount, GL_UNSIGNED_INT, (void*)(firstQuad * 6 *sizeof(GLuint)));//fourth parameter should be in bytes
	//stoppy.Stop();
	//cout << "draw time bitch: " << stoppy.MilliSeconds() << " and the gl error bitch: " << glGetError() << "\n";
}
QuadVAO::~QuadVAO()
{
	glDeleteBuffers(1, &indexBuffer);
	glDeleteVertexArrays(1, &quadVAO);
}
