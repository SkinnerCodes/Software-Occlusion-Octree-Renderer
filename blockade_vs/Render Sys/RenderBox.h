//
// Created by Thomas on 4/3/2015.
//
#pragma once
#ifndef _BLOACKADE_RENDERBOX_H_
#define _BLOACKADE_RENDERBOX_H_

#include "VoxelRenderBase.h"
#include <minwindef.h>
#include <glm util\Frustum.h>
#undef max

namespace SoftRendering {
	
	struct ProjectionData {

		bareVec4f cameraPos;
		bareVec8f cameraPosX_SOA;
		bareVec8f cameraPosY_SOA;
		bareVec8f cameraPosZ_SOA;
		int windowWidth, windowHeight;
		bareMat4 viewProjection;
		mat4 glmViewProjection;
		bareMat4 invVP;

		ProjectionData(mat4 viewMat, mat4 projectionMat, int windowWidth, int windowHeight) {
			
			mat4 viewProjection;
			mat4 invView = inverse(viewMat);
			viewProjection = projectionMat * viewMat;
			glmViewProjection = viewProjection;
			this->viewProjection = viewProjection; //using conversion assignment glm to baremat
			this->windowWidth = windowWidth;
			this->windowHeight = windowHeight;
			this->invVP = inverse(viewProjection);
			vec4 camerapos = invView * vec4(0.0f, 0.0f, 0.0f, 1.0f);
			camerapos = camerapos / camerapos.w;
			cameraPos = bareVec4f(camerapos);
			cameraPosX_SOA = _mm256_set1_ps(cameraPos.get(0));
			cameraPosY_SOA = _mm256_set1_ps(cameraPos.get(1));
			cameraPosZ_SOA = _mm256_set1_ps(cameraPos.get(2));	
		}
	};



	struct QuadBorder {
		bareVec4f A;
		bareVec4f B;
		bareVec4f C;
		bareVec4f minMaxBounds; // a bounding box for the face stored in this order, minx, miny, max x, maxy, this is used to optimize rasterizing
	};
	struct BorderEightLines {
		bareVec8f A;
		bareVec8f B;
		bareVec8f C;
		bareVec4f minMaxBounds; // a bounding box for the face stored in this order, minx, miny, max x, maxy, this is used to optimize rasterizing
	};
	struct QuadCoords //SoA
	{
		bareVec4f x, y, z, w;
	};
	struct EightCoords //SoA
	{
		bareVec8f x, y, z, w;
	};
	struct RenderBoxFaceProjection {
		//the corresponding components of each line equation are stored together, SoA style
		union
		{
			QuadBorder border;
			BorderEightLines clippedBorder;
		};
		RenderBoxFaceProjection() : border() {} //this is necessary other wise the union causes default ctors to be deleted, but why?
		bool clipped;
	};
	struct RenderBoxProjection
	{
		BorderEightLines windowBorder;
	};
	struct RenderBox { //seen faces are not predetermined
		bareVec4f worldMin; //min and max of box in world space
		bareVec4f worldMax; //stored x y z, z most significant, w should be 1, 0 is for directions, homogenious coordinates bro

		bareVec4f clipSpaceMin;
		RenderBox(){}
		union {
			RenderBoxFaceProjection faces[3]; //either with projection per face
			RenderBoxProjection entireProjection; //or one whole structure that uses vec8s
		};
		int isFaceSeenOnAxisBits; //bools encoded in first 3 bits, x first
		int posOrNegFaceSeenBits; //bools encoded in first 3 bits 

		void setSeenFacesAndBorderOptimizations(ProjectionData &projectData, bool &threeFaceOptimization); //prepares for border generation by setting the seen faces, and 

		void projectFace(int axis, int posFace, ProjectionData &projectData);
		void getProjectedFaceBorder(int axis, int posFace, ProjectionData &projectData);
		void getProjectedFaceBorderFromClipSpaceVerts(RenderBoxFaceProjection &face, ProjectionData &projectData, __m128 &packedX, __m128 &packedY, __m128 &packedZ, __m128 &packedW);
		void getProjectedOneFacePerAxisBorder(int posFaceIsSeen, ProjectionData &projectData);

	};

	
//	//two render boxes, one where the seen faces are predetermined, and one where it needs to be determined
//	// the broader algorithm will start off using the non-predetermined, until the direction from eye to the box's min and max both agree which faces should be seen\
//
//	struct RenderBoxBorder {
//		//the corresponding components of each line equation are stored together, SoA style
//		//only first 6 are used
//		bareVec8f A;
//		bareVec8f B;
//		bareVec8f C;
//	};
//	struct RenderBoxFast { // seen faces are predetermined
//		bareVec4f cameraMin; //min and max in camera space
//		bareVec4f cameraMax;
//		bareVec4f seenFaceMask; // all 1s where the positive face is seen, 0s where negative face is seen, (positive side is in positve axis direction in world space)
//		RenderBoxBorder border;
//
//		void GenerateBorder(ProjectionData &projectData);
//	};
}

#endif //_BLOACKADE_RENDERBOX_H_
