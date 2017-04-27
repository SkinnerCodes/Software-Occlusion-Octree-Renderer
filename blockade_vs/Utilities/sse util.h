//
// Created by Thomas on 3/21/2015.
//

#ifndef _BLOACKADE_SSE2_UTILS_H_
#define _BLOACKADE_SSE2_UTILS_H_

#include <xmmintrin.h>
#include "glm/glm.hpp"
//#include "bareMatrix.h"

namespace sseutil {

    inline __m128 mm_select_ps(__m128 a, __m128 b, __m128 mask) { //sse function blend can replace this i think
        //all the 1 bis in mask end up as bits of b, oherwise a
        return _mm_xor_ps( a, _mm_and_ps( mask, _mm_xor_ps( b, a )));
    }

    union bareVec4f { // things that come first are laid out firs tin memory, coming first meaning declared first, more up.
        __m128 m128;    // SSE 4 x float vector
        float floats[4];  // scalar array of 4 floats
        int ints[4];
        struct {
            float x;
            float y;
            float z;
            float w;
        };


        inline bareVec4f(){
        }
		inline bareVec4f(bareVec4f &&bareVec) {
			x = bareVec.x;
			y = bareVec.y;
			z = bareVec.z;
			w = bareVec.w;
		}
		inline bareVec4f(bareVec4f &bareVec) {
			x = bareVec.x;
			y = bareVec.y;
			z = bareVec.z;
			w = bareVec.w;
		}
		inline bareVec4f& operator =(bareVec4f &bareVec) {
			x = bareVec.x;
			y = bareVec.y;
			z = bareVec.z;
			w = bareVec.w;
			return *this;
		}
		inline bareVec4f(glm::vec4 vec4) {
			x = vec4.x;
			y = vec4.y;
			z = vec4.z;
			w = vec4.w;
		}
		inline bareVec4f(glm::vec3 vec3) {
			x = vec3.x;
			y = vec3.y;
			z = vec3.z;
			w = 1.0f;
		}
        inline bareVec4f(__m128 vec){
            this->m128 = vec;
        }
		inline float X()
        {
			return floats[0];
        }
		inline float Y()
		{
			return floats[1];
		}
		inline float Z()
		{
			return floats[2];
		}
		inline float W()
		{
			return floats[3];
		}
		float& operator [] (int index) {
			return floats[index];
		}
        inline float get(int index) { //you can swap out these functions for load / store instinsic functions if you find that to be cheaper performancewise
            return floats[index];
        }
        inline void set (int index, float value) { //used to dynamically set components of the vector by index
            floats[index] = value;
        }
        inline void set (int index, int value) { //incase u want to pack integers in here
            ints[index] = value;
        }
		inline void set(glm::vec4 vec) {
			x = vec.x;
			y = vec.y;
			z = vec.z;
			w = vec.w;
		}
		inline void set (int w, int z, int y, int x)
        {
			ints[0] = x;
			ints[1] = y;
			ints[2] = z;
			ints[3] = w;
        }
		inline void set (float w, float z, float y, float x)
        {
			floats[0] = x;
			floats[1] = y;
			floats[2] = z;
			floats[3] = w;
        }
        inline bareVec4f& operator=(__m128 m128){
            this->m128 = m128;
            return *this;
        }
        inline operator __m128& () {
            return m128;
        }

		inline bareVec4f& transformBy(glm::mat4 matrix) //this is expensive
        {
			glm::vec4 temp(get(0), get(1), get(2), get(3));
			temp = matrix * temp;
			set(0, temp.x);
			set(1, temp.y);
			set(2, temp.z);
			set(3, temp.w);
			return *this;
        }
//		inline bareVec4f transformBy(bareMat4 matrix) //this is expensive
//		{
//			return matrix.ProjectPoint(this);
//		}
		inline bareVec4f& cross(bareVec4f &operand)
        {
			//x = y*operand.z - z*operand.y;
			m128 = _mm_sub_ps(_mm_mul_ps(_mm_shuffle_ps(m128, m128, 0b001001), _mm_shuffle_ps(operand, operand, 0b010010)), _mm_mul_ps(_mm_shuffle_ps(m128, m128, 0b010010), _mm_shuffle_ps(operand, operand, 0b001001)));
			return *this;
        }
		inline float length3D()
        {
			bareVec4f temp = _mm_mul_ps(m128, m128);
			return sqrt(temp.x + temp.y + temp.z);
        }
		inline bareVec4f& normalize3D()
        {
			m128 = _mm_div_ps(m128, _mm_set1_ps(length3D()));
			return *this;
        }
    };

    union bareVec4i {
        __m128i m128i;    // SSE 4 x float vector
        int ints[4];  // scalar array of 4 floats
        inline bareVec4i(){
        }
        inline bareVec4i(__m128i vec){
            this->m128i = vec;
        }
        inline int get(int index) { //you can swap out these functions for load / store instinsic functions if you find that to be cheaper performancewise
            return ints[index];
        }
        inline void set (int index, int value) {
            ints[index] = value;
        }
        inline bareVec4i& operator=(__m128i m128i){
            this->m128i = m128i;
            return *this;
        }
        inline operator __m128i& () {
            return m128i;
        }
    };


}
#endif //_BLOACKADE_SSE2_UTILS_H_
