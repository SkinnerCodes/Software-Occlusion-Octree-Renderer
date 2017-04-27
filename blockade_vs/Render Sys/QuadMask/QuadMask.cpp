#pragma once
#include "QuadMask.h"
#include <string>
#include <iostream>
#include <sstream>

namespace SoftRendering
{

    QuadMask::QuadMask(int width, int height) {
		Initialize(0, 0, width, height);
    }
	void QuadMask::Initialize(int ScreenRegionStartX, int ScreenRegionStartY, int width, int height){
		this->width = width; this->height = height;
		this->ScreenRegionWidth = width; this->ScreenRegionHeight = height;
		this->ScreenRegionStartX = ScreenRegionStartX; this->ScreenRegionStartY = ScreenRegionStartY;
		width = width + 100; height = height + 100; //testcode
        BlocksInRow = (width/32 + ((width & 31) > 0));
        BlocksInColumn = height/2;
        MaskBuffer.reset((int8_t*) malloc(BlocksInRow * BlocksInColumn * sizeof(int64_t)), free);
    }
    QuadMask::~QuadMask() {
    }
	void QuadMask::Clear()
	{
		int64_t* buffer = (int64_t*)MaskBuffer.get();
		for (int j = 0; j < BlocksInColumn; j++)
		{
			for (int i = 0; i < BlocksInRow; i++)
			{
				buffer[i + j* BlocksInRow] = 0;
			}
		}
	}

    void QuadMask::ConsolePrint2()
    {
        stringstream s;
        s << endl;
        string blank = "x";
        string filled = "f";
        int64_t* buf = (int64_t*)MaskBuffer.get();
//        buf[0] = 0xFFFFFFFFFFFFFFFF;
//        buf[1] = 0xFFFFFFFFFFFFFFFF;
//        buf[2] = 0xFFFFFFFFFFFFFFFF;
//        buf[3] = 0xFFFFFFFFFFFFFFFF;
		for (int y = height - 1; y >= 0; y--) { //added test offsets
            for(int x = 0; x < width; x++) {
                int index = ((x/32)+(y/2)*BlocksInRow);
                int64_t bloc = buf[index];
                s << (((bloc >> ((x & 31)*2 + (y&1)))&1) == 1 ? filled : blank);
				//s << " ";
            }
            s << endl;
        }
		printf(s.str().c_str(), "%s");
        //cout << s.str();
//        int64_t test = 0;
//        for(int i = 0; i < BlocksInRow * BlocksInColumn; i++){
//            test |= buf[i];
//        }
//        cout << endl << test;
    }

	SDL_Surface &QuadMask::PrintToSurface(SDL_Surface &surface)
	{
		SDL_LockSurface(&surface);
		Uint32 *pixels = (Uint32 *)surface.pixels;
		uint32 blank = 0;
		uint32 filled = ~0;
		int64_t* buf = (int64_t*)MaskBuffer.get();
		for (int y = 0; y < height; y++) { //added test offsets
			for (int x = 0; x < width; x++) {
				int index = ((x / 32) + (y / 2)*BlocksInRow);
				int64_t bloc = buf[index];
				pixels[((height-1-y) * surface.w) + x] = (((bloc >> ((x & 31) * 2 + (y & 1))) & 1) == 1 ? filled : blank);
	
			}
		}
		SDL_UnlockSurface(&surface);
		return surface;
	}
	void QuadMask::drawWithSDL(SDL_Renderer *renderer)
	{
		static int start = 0;
		static SDL_Surface *surfacePointer;
		static SDL_Texture *newtexture;
		static SDL_Rect texture_rect;
		if (start == 0) {
			surfacePointer = SDL_CreateRGBSurface(0, width, height, 32, 0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000ff);
			if (surfacePointer == NULL) {
				fprintf(stderr, "CreateRGBSurface failed: %s\n", SDL_GetError());
				exit(1);
			}
			newtexture = SDL_CreateTextureFromSurface(renderer, surfacePointer);
			texture_rect.x = 0;  //the x coordinate
			texture_rect.y = 0; // the y coordinate
			texture_rect.w = width; //the width of the texture
			texture_rect.h = height; //the height of the texture
			start = 1;
		}

		PrintToSurface(*surfacePointer);
		if(SDL_UpdateTexture(newtexture, &texture_rect, surfacePointer->pixels, surfacePointer->pitch))
		{
			texture_rect.w = 3;
			return;
		};
		SDL_RenderCopy(renderer, newtexture, NULL, &texture_rect);
	}
}

