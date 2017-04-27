#pragma once
#include "avx util.h"
#include "sse util.h"

using namespace avxutil;
using namespace sseutil;

struct bareMat4 //optimized SOA matrix tranformations on multiple vectors
{
	union {
		float members[4 * 4]; //column major storage
		__m128 asdf; //this union is only used to align the  array
	};
	inline bareMat4()
	{
	}	
	inline bareMat4(glm::mat4 matrix)
	{
		for (int i = 0; i < 4; i++)
		{
			for (int j = 0; j < 4; j++)
			{
				members[i * 4 + j] = matrix[i][j];
			}
		}
	}
	bareVec8f member8(int col, int row)
	{
		return _mm256_set1_ps(members[4 * col + row]);
	}
	bareVec4f member4(int col, int row)
	{
		return _mm_set1_ps(members[4 * col + row]);
	}
	bareVec4f column(int col)
	{
		return _mm_load_ps(&members[col * 4]);
	}
	bareVec4f ProjectPoint(bareVec4f point) //not in place
	{
		__m128 x = _mm_shuffle_ps(point, point, 0b00000000);
		__m128 y = _mm_shuffle_ps(point, point, 0b01010101);
		__m128 z = _mm_shuffle_ps(point, point, 0b10101010);
		__m128 w = _mm_shuffle_ps(point, point, 0b11111111);
		return
			_mm_add_ps(
				_mm_add_ps(_mm_mul_ps(x, column(0)), _mm_mul_ps(y, column(1))),
				_mm_add_ps(_mm_mul_ps(z, column(2)), _mm_mul_ps(w, column(3))));
	}
	void Project_Four_SOA_Points(bareVec4f &x, bareVec4f &y, bareVec4f &z, bareVec4f &w) //inplace
	{
		bareVec4f newx = _mm_add_ps(
			_mm_add_ps(_mm_mul_ps(x, member4(0, 0)), _mm_mul_ps(y, member4(1, 0))),
			_mm_add_ps(_mm_mul_ps(z, member4(2, 0)), _mm_mul_ps(w, member4(3, 0)))
			);
		bareVec4f newy = _mm_add_ps(
			_mm_add_ps(_mm_mul_ps(x, member4(0, 1)), _mm_mul_ps(y, member4(1, 1))),
			_mm_add_ps(_mm_mul_ps(z, member4(2, 1)), _mm_mul_ps(w, member4(3, 1)))
			);
		bareVec4f newz = _mm_add_ps(
			_mm_add_ps(_mm_mul_ps(x, member4(0, 2)), _mm_mul_ps(y, member4(1, 2))),
			_mm_add_ps(_mm_mul_ps(z, member4(2, 2)), _mm_mul_ps(w, member4(3, 2)))
			);
		bareVec4f neww = _mm_add_ps(
			_mm_add_ps(_mm_mul_ps(x, member4(0, 3)), _mm_mul_ps(y, member4(1, 3))),
			_mm_add_ps(_mm_mul_ps(z, member4(2, 3)), _mm_mul_ps(w, member4(3, 3)))
			);
		x = newx; y = newy; z = newz; w = neww;
	}
	void Project_Eight_Points(__m256 &x, __m256 &y, __m256 &z, __m256 &w)
	{
		bareVec8f newx = _mm256_add_ps(
			_mm256_add_ps(_mm256_mul_ps(x, member8(0, 0)), _mm256_mul_ps(y, member8(1, 0))),
			_mm256_add_ps(_mm256_mul_ps(z, member8(2, 0)), _mm256_mul_ps(w, member8(3, 0)))
			);
		bareVec8f newy = _mm256_add_ps(
			_mm256_add_ps(_mm256_mul_ps(x, member8(0, 1)), _mm256_mul_ps(y, member8(1, 1))),
			_mm256_add_ps(_mm256_mul_ps(z, member8(2, 1)), _mm256_mul_ps(w, member8(3, 1)))
			);
		bareVec8f newz = _mm256_add_ps(
			_mm256_add_ps(_mm256_mul_ps(x, member8(0, 2)), _mm256_mul_ps(y, member8(1, 2))),
			_mm256_add_ps(_mm256_mul_ps(z, member8(2, 2)), _mm256_mul_ps(w, member8(3, 2)))
			);
		bareVec8f neww = _mm256_add_ps(
			_mm256_add_ps(_mm256_mul_ps(x, member8(0, 3)), _mm256_mul_ps(y, member8(1, 3))),
			_mm256_add_ps(_mm256_mul_ps(z, member8(2, 3)), _mm256_mul_ps(w, member8(3, 3)))
			);
		x = newx; y = newy; z = newz; w = neww;
		
	}
	void Project_Eight_Points_out(const bareVec8f &x, const bareVec8f &y, const bareVec8f &z, const bareVec8f &w, bareVec8f &outX, bareVec8f &outY, bareVec8f &outZ, bareVec8f &outW)
	{
		outX = _mm256_add_ps(
			_mm256_add_ps(_mm256_mul_ps(x, member8(0, 0)), _mm256_mul_ps(y, member8(1, 0))),
			_mm256_add_ps(_mm256_mul_ps(z, member8(2, 0)), _mm256_mul_ps(w, member8(3, 0)))
			);
		outY = _mm256_add_ps(
			_mm256_add_ps(_mm256_mul_ps(x, member8(0, 1)), _mm256_mul_ps(y, member8(1, 1))),
			_mm256_add_ps(_mm256_mul_ps(z, member8(2, 1)), _mm256_mul_ps(w, member8(3, 1)))
			);
		outZ = _mm256_add_ps(
			_mm256_add_ps(_mm256_mul_ps(x, member8(0, 2)), _mm256_mul_ps(y, member8(1, 2))),
			_mm256_add_ps(_mm256_mul_ps(z, member8(2, 2)), _mm256_mul_ps(w, member8(3, 2)))
			);
		outW = _mm256_add_ps(
			_mm256_add_ps(_mm256_mul_ps(x, member8(0, 3)), _mm256_mul_ps(y, member8(1, 3))),
			_mm256_add_ps(_mm256_mul_ps(z, member8(2, 3)), _mm256_mul_ps(w, member8(3, 3)))
			);
	}
};