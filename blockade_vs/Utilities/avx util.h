//
// Created by Thomas on 3/21/2015.
//

#ifndef _BLOACKADE_AVX_UTIL_H_
#define _BLOACKADE_AVX_UTIL_H_

#include <iostream>
#include <immintrin.h>

namespace avxutil {
    union bareVec8f { // things that come first are layed out firs tin memory, coming first meaning declared first, more up.
        __m256 m256;
        float floats[8];
		int ints[8];
		struct {
			__m128 first128;
			__m128 second128;
		};
        inline bareVec8f() {
        }

        inline bareVec8f(__m256 vec) : m256(vec) {
            //m256 = vec;
        }
//		inline bareVec8f(const __m256 &vec) : m256(vec) { //this fucks shit up
//			//m256 = vec;
//		}
		float& operator [] (int index) {

//			bareVec8f t;
//			_mm256_store_ps((float*)&(t.m256), m256);
//			floats[index] = t.get(index);
			return floats[index];
		}
        inline float get(int index) { //you can swap out these functions for load / store instinsic functions if you find that to be cheaper performancewise
            return floats[index];
        }

        inline void set(int index, float value) {
            floats[index] = value;
        }
		inline void set(int index, int value) {
            ints[index] = value;
        }
		inline void set(int i7, int i6, int i5, int i4, int i3, int i2, int i1, int i0)
        {
			ints[0] = i0;
			ints[1] = i1;
			ints[2] = i2;
			ints[3] = i3;
			ints[4] = i4;
			ints[5] = i5;
			ints[6] = i6;
			ints[7] = i7;
        }
		inline void set(float i7, float i6, float i5, float i4, float i3, float i2, float i1, float i0)
		{
			floats[0] = i0;
			floats[1] = i1;
			floats[2] = i2;
			floats[3] = i3;
			floats[4] = i4;
			floats[5] = i5;
			floats[6] = i6;
			floats[7] = i7;
		}
        inline bareVec8f &operator=(__m256 m256) {
            this->m256 = m256;
            return *this;
        }

        inline operator __m256 &() {
            return m256;
        }
		inline operator const __m256 &() const {
			return m256;
		}

    };
	union bareVec8i { // things that come first are layed out firs tin memory, coming first meaning declared first, more up.
		__m256i m256i;
		float floats[8];
		int ints[8];
		struct {
			__m128 first128;
			__m128 second128;
		};
		inline bareVec8i() {
		}

		inline bareVec8i(__m256i vec) : m256i(vec) {

		}

		int& operator [] (int index) {
			return ints[index];
		}
		inline int get(int index) { //you can swap out these functions for load / store instinsic functions if you find that to be cheaper performancewise
			return ints[index];
		}

		inline void set(int index, float value) {
			floats[index] = value;
		}
		inline void set(int index, int value) {
			ints[index] = value;
		}
		inline void set(int i7, int i6, int i5, int i4, int i3, int i2, int i1, int i0)
		{
			ints[0] = i0;
			ints[1] = i1;
			ints[2] = i2;
			ints[3] = i3;
			ints[4] = i4;
			ints[5] = i5;
			ints[6] = i6;
			ints[7] = i7;
		}
		inline void set(float i7, float i6, float i5, float i4, float i3, float i2, float i1, float i0)
		{
			floats[0] = i0;
			floats[1] = i1;
			floats[2] = i2;
			floats[3] = i3;
			floats[4] = i4;
			floats[5] = i5;
			floats[6] = i6;
			floats[7] = i7;
		}
		inline bareVec8i &operator=(__m256i m256i) {
			this->m256i = m256i;
			return *this;
		}

		inline operator __m256i &() {
			return m256i;
		}
		inline operator const __m256i &() const {
			return m256i;
		}

	};

}
#endif //_BLOACKADE_AVX_UTIL_H_
