#pragma once

#include "VoxelRenderBase.h"

#include "Render Sys/RenderBox.h" 



namespace SoftRendering {
	static int testcount = 0;
    class QuadMask {
    public:
        unsigned int width;
        unsigned int height;
        unsigned int BlocksInRow, BlocksInColumn;
		uint32_t ScreenRegionStartX, ScreenRegionStartY, ScreenRegionWidth, ScreenRegionHeight;
        shared_ptr<int8_t> MaskBuffer;
        QuadMask(int width, int height);
        inline QuadMask() {}
        ~QuadMask();
		void Initialize(int ScreenRegionStartX, int ScreenRegionStartY, int width, int height);
        void Clear();
        void ConsolePrint(); void ConsolePrint2();
		SDL_Surface &PrintToSurface(SDL_Surface &surface);
		void drawWithSDL(SDL_Renderer *renderer);
        //bool IsIntersecting(int height, int min, int max);

        inline __m128 getDistances(__m128& A, __m128& B, __m128& C, __m128& pixCoordX, __m128& pixCoordY){
            return _mm_add_ps(_mm_add_ps(_mm_mul_ps(pixCoordX, A), _mm_mul_ps(pixCoordY, B)) , C    );
        }
		inline __m128i getDistances_epi32(__m128i& A, __m128i& B, __m128i& C, __m128i& pixCoordX, __m128i& pixCoordY) {
			return _mm_add_epi32(_mm_add_epi32(_mm_mul_epi32(pixCoordX, A), _mm_mul_epi32(pixCoordY, B)), C);
		}
		inline __m256 getDistances_stamp8(__m256& A, __m256& B, __m256& C, __m256& pixCoordX, __m256& pixCoordY){
            return _mm256_add_ps(_mm256_add_ps(_mm256_mul_ps(pixCoordX, A), _mm256_mul_ps(pixCoordY, B)) , C);
        }
#include "QuadMask_stamp8.h"
#include "QuadMask_stamp4.h"


    };
}