#pragma once
#include "VoxelRenderBase.h"
#include "camera.h"
#include "RenderBox.h"

namespace SoftRendering {
	class CullFrustum
	{
	public:
		inline CullFrustum(){}
		inline ~CullFrustum(){}
		bareVec8f frustumCornersX, frustumCornersY, frustumCornersZ;
		bareVec8f sepAxisForAABB_NearFace_X, sepAxisForAABB_NearFace_Y, sepAxisForAABB_NearFace_Z;
		float sepAxiiX[29], sepAxiiY[29], sepAxiiZ[29];
		float frustumOnAxiiMax[29], frustumOnAxiiMin[29], nearFaceOnAxiiMin[7], nearFaceOnAxiiMax[7];
		inline void Split_8_way(CullFrustum subFrustumsToInitialize[8])
		{
			vec2 portionStart, portionEnd;
			for (int i = 0; i < 8; i++)
			{
				portionStart.y = i < 4 ? 0.f : .5f;
				portionStart.x = (i%4) * (1.f/4.f);
				portionEnd.x = portionStart.x + (1.f/4.f);
				portionEnd.y = portionStart.y + (1.f/2.f);
				InitSubFrustum(subFrustumsToInitialize[i], portionStart, portionEnd);
			}
		}
		inline void Split_4_way(CullFrustum subFrustumsToInitialize[4])
		{
			vec2 portionStart, portionEnd;
			for (int i = 0; i < 4; i++)
			{
				portionStart.y = i < 2 ? 0.f : .5f;
				portionStart.x = (i % 2) * (1.f / 2.f);
				portionEnd.x = portionStart.x + (1.f / 2.f);
				portionEnd.y = portionStart.y + (1.f / 2.f);
				InitSubFrustum(subFrustumsToInitialize[i], portionStart, portionEnd);
			}
		}
		inline void InitSubFrustum(CullFrustum &subFrustum, vec2 portionStart, vec2 portionEnd) {//portionStart is a 2d normalized coordinate with the origin centered at the bottom left of the near face of the frustum
			vec3 screenBottomLeft, screenBottomRight, screenTopLeft;
			screenBottomLeft.x = frustumCornersX[0];
			screenBottomLeft.y = frustumCornersY[0];
			screenBottomLeft.z = frustumCornersZ[0];
			screenBottomRight.x = frustumCornersX[1];
			screenBottomRight.y = frustumCornersY[1];
			screenBottomRight.z = frustumCornersZ[1];
			screenTopLeft.x = frustumCornersX[2];
			screenTopLeft.y = frustumCornersY[2];
			screenTopLeft.z = frustumCornersZ[2];
			vec3 screenHorizontalDif, screenVerticalDif;
			screenHorizontalDif = screenBottomRight - screenBottomLeft;
			screenVerticalDif = screenTopLeft - screenBottomLeft;
			vec3 newScreenBottomLeft, newScreenBottomRight, newScreenTopLeft, newScreenTopRight;
			newScreenBottomLeft = screenBottomLeft + (portionStart.x * screenHorizontalDif) + (portionStart.y * screenVerticalDif);
			newScreenTopRight = screenBottomLeft + (portionEnd.x * screenHorizontalDif) + (portionEnd.y * screenVerticalDif);
			newScreenTopLeft = screenBottomLeft + (portionStart.x * screenHorizontalDif) + (portionEnd.y * screenVerticalDif);
			newScreenBottomRight = screenBottomLeft + (portionEnd.x * screenHorizontalDif) + (portionStart.y * screenVerticalDif);
			vec3 farBottomLeft, farBottomRight, farTopLeft;
			farBottomLeft.x = frustumCornersX[4];
			farBottomLeft.y = frustumCornersY[4];
			farBottomLeft.z = frustumCornersZ[4];
			farBottomRight.x = frustumCornersX[5];
			farBottomRight.y = frustumCornersY[5];
			farBottomRight.z = frustumCornersZ[5];
			farTopLeft.x = frustumCornersX[6];
			farTopLeft.y = frustumCornersY[6];
			farTopLeft.z = frustumCornersZ[6];
			vec3 farHorizontalDif, farVerticalDif;
			farHorizontalDif = farBottomRight - farBottomLeft;
			farVerticalDif = farTopLeft - farBottomLeft;
			vec3 newFarBottomLeft, newFarBottomRight, newFarTopLeft, newFarTopRight;
			newFarBottomLeft = farBottomLeft + (portionStart.x * farHorizontalDif) + (portionStart.y * farVerticalDif);
			newFarTopRight = farBottomLeft + (portionEnd.x * farHorizontalDif) + (portionEnd.y * farVerticalDif);
			newFarTopLeft = farBottomLeft + (portionStart.x * farHorizontalDif) + (portionEnd.y * farVerticalDif);
			newFarBottomRight = farBottomLeft + (portionEnd.x * farHorizontalDif) + (portionStart.y * farVerticalDif);

			subFrustum.frustumCornersX.set(newFarTopRight.x, newFarTopLeft.x, newFarBottomRight.x, newFarBottomLeft.x, newScreenTopRight.x, newScreenTopLeft.x, newScreenBottomRight.x, newScreenBottomLeft.x);
			subFrustum.frustumCornersY.set(newFarTopRight.y, newFarTopLeft.y, newFarBottomRight.y, newFarBottomLeft.y, newScreenTopRight.y, newScreenTopLeft.y, newScreenBottomRight.y, newScreenBottomLeft.y);
			subFrustum.frustumCornersZ.set(newFarTopRight.z, newFarTopLeft.z, newFarBottomRight.z, newFarBottomLeft.z, newScreenTopRight.z, newScreenTopLeft.z, newScreenBottomRight.z, newScreenBottomLeft.z);
			subFrustum.SetSeperatingAxii();
			
		}
		inline void SetSeperatingAxii() //uses frustrum corners to calculate seperating axxi and projections onto the axii
		{
			//get seperating axxi
			bareVec8f frustumEdgeDirsX = _mm256_permute_ps(frustumCornersX, 0b0010);
			frustumEdgeDirsX = _mm256_permute2f128_ps(frustumCornersX, frustumEdgeDirsX, 0b00100001);
			frustumEdgeDirsX = _mm256_sub_ps(frustumEdgeDirsX, _mm256_permute2f128_ps(frustumCornersX, frustumCornersX, 0b0));
			bareVec8f frustumEdgeDirsY = _mm256_permute_ps(frustumCornersY, 0b0010);
			frustumEdgeDirsY = _mm256_permute2f128_ps(frustumCornersY, frustumEdgeDirsY, 0b00100001);
			frustumEdgeDirsY = _mm256_sub_ps(frustumEdgeDirsY, _mm256_permute2f128_ps(frustumCornersY, frustumCornersY, 0b0));
			bareVec8f frustumEdgeDirsZ = _mm256_permute_ps(frustumCornersZ, 0b0010);
			frustumEdgeDirsZ = _mm256_permute2f128_ps(frustumCornersZ, frustumEdgeDirsZ, 0b00100001);
			frustumEdgeDirsZ = _mm256_sub_ps(frustumEdgeDirsZ, _mm256_permute2f128_ps(frustumCornersZ, frustumCornersZ, 0b0));
			//normalize directions
			bareVec8f x2 = _mm256_mul_ps(frustumEdgeDirsX, frustumEdgeDirsX);
			bareVec8f y2 = _mm256_mul_ps(frustumEdgeDirsY, frustumEdgeDirsY);
			bareVec8f z2 = _mm256_mul_ps(frustumEdgeDirsZ, frustumEdgeDirsZ);
			bareVec8f lengths = _mm256_sqrt_ps(_mm256_add_ps(_mm256_add_ps(x2, y2), z2));
			frustumEdgeDirsX = _mm256_div_ps(frustumEdgeDirsX, lengths);
			frustumEdgeDirsY = _mm256_div_ps(frustumEdgeDirsY, lengths);
			frustumEdgeDirsZ = _mm256_div_ps(frustumEdgeDirsZ, lengths);
			//first 6 elements hold relevant data on the unique frustum edge directions, indices 4 and 5 are the near face edges, 4th is bottom left to top left, 5th is bottom right to bottom left
			bareVec8f sepAxisX, sepAxisY, sepAxisZ, partialDotX, partialDotY, partialDotZ, frustumMax, frustumMin, screenMin, screenMax, boxMin, boxMax;
			//cross the edges with the aabb edges to generate seperating axii. indices 4 and 5 are from near face edges
			for (int a = 0; a < 3; a++)
			{
				if (a == 0) {
					sepAxisX = _mm256_set1_ps(0.f);
					sepAxisY = _mm256_sub_ps(_mm256_set1_ps(0.f), frustumEdgeDirsZ);
					sepAxisZ = frustumEdgeDirsY;
				}
				else if (a == 1)
				{
					sepAxisX = frustumEdgeDirsZ;
					sepAxisY = _mm256_set1_ps(0.f);
					sepAxisZ = _mm256_sub_ps(_mm256_set1_ps(0.f), frustumEdgeDirsX);
				}
				else if (a == 2)
				{
					sepAxisX = _mm256_sub_ps(_mm256_set1_ps(0.f), frustumEdgeDirsY);
					sepAxisY = frustumEdgeDirsX;
					sepAxisZ = _mm256_set1_ps(0.f);
				}
				for (int i = 0; i < 6; i++)
				{
					int firstAfterNearFaceStuff = 7;
					if (i < 4) {
						sepAxiiX[a * 4 + i + firstAfterNearFaceStuff] = sepAxisX[i];
						sepAxiiY[a * 4 + i + firstAfterNearFaceStuff] = sepAxisY[i];
						sepAxiiZ[a * 4 + i + firstAfterNearFaceStuff] = sepAxisZ[i];
					}
					else
					{
						sepAxiiX[(a * 2) + (i - 4)] = sepAxisX[i];
						sepAxiiY[(a * 2) + (i - 4)] = sepAxisY[i];
						sepAxiiZ[(a * 2) + (i - 4)] = sepAxisZ[i];
					}
				}
			}
			//fil in frustrum normals
			bareVec4f tempVec, tempVecTwo, frustumNormal;
			//first fill near face normal to index 6
			tempVec.x = frustumEdgeDirsX[4];
			tempVecTwo.x = frustumEdgeDirsX[5];
			tempVec.y = frustumEdgeDirsY[4];
			tempVecTwo.y = frustumEdgeDirsY[5];
			tempVec.z = frustumEdgeDirsZ[4];
			tempVecTwo.z = frustumEdgeDirsZ[5];
			frustumNormal = tempVec.cross(tempVecTwo);
			frustumNormal.normalize3D();
			sepAxiiX[6] = frustumNormal.x;
			sepAxiiY[6] = frustumNormal.y;
			sepAxiiZ[6] = frustumNormal.z;
			tempVec.x = frustumEdgeDirsX[4];
			tempVecTwo.x = frustumEdgeDirsX[0];
			tempVec.y = frustumEdgeDirsY[4];
			tempVecTwo.y = frustumEdgeDirsY[0];
			tempVec.z = frustumEdgeDirsZ[4];
			tempVecTwo.z = frustumEdgeDirsZ[0];
			frustumNormal = tempVec.cross(tempVecTwo);
			frustumNormal.normalize3D();
			sepAxiiX[19] = frustumNormal.x;
			sepAxiiY[19] = frustumNormal.y;
			sepAxiiZ[19] = frustumNormal.z;
			tempVec.x = frustumEdgeDirsX[4];
			tempVecTwo.x = frustumEdgeDirsX[1];
			tempVec.y = frustumEdgeDirsY[4];
			tempVecTwo.y = frustumEdgeDirsY[1];
			tempVec.z = frustumEdgeDirsZ[4];
			tempVecTwo.z = frustumEdgeDirsZ[1];
			tempVecTwo = _mm_mul_ps(tempVecTwo, _mm_set1_ps(-1.0f));
			frustumNormal = tempVec.cross(tempVecTwo);
			frustumNormal.normalize3D();
			sepAxiiX[20] = frustumNormal.x;
			sepAxiiY[20] = frustumNormal.y;
			sepAxiiZ[20] = frustumNormal.z;
			tempVec.x = frustumEdgeDirsX[5];
			tempVecTwo.x = frustumEdgeDirsX[1];
			tempVec.y = frustumEdgeDirsY[5];
			tempVecTwo.y = frustumEdgeDirsY[1];
			tempVec.z = frustumEdgeDirsZ[5];
			tempVecTwo.z = frustumEdgeDirsZ[1];
			frustumNormal = tempVec.cross(tempVecTwo);
			frustumNormal.normalize3D();
			sepAxiiX[21] = frustumNormal.x;
			sepAxiiY[21] = frustumNormal.y;
			sepAxiiZ[21] = frustumNormal.z;
			tempVec.x = frustumEdgeDirsX[5];
			tempVecTwo.x = frustumEdgeDirsX[3];
			tempVec.y = frustumEdgeDirsY[5];
			tempVecTwo.y = frustumEdgeDirsY[3];
			tempVec.z = frustumEdgeDirsZ[5];
			tempVecTwo.z = frustumEdgeDirsZ[3];
			tempVecTwo = _mm_mul_ps(tempVecTwo, _mm_set1_ps(-1.0f));
			frustumNormal = tempVec.cross(tempVecTwo);
			frustumNormal.normalize3D();
			sepAxiiX[22] = frustumNormal.x;
			sepAxiiY[22] = frustumNormal.y;
			sepAxiiZ[22] = frustumNormal.z;
			//fill in aabb normals
			sepAxiiX[23] = -1.0f;
			sepAxiiY[23] = 0.0f;
			sepAxiiZ[23] = 0.0f;
			sepAxiiX[24] = 0.0f;
			sepAxiiY[24] = -1.0f;
			sepAxiiZ[24] = 0.0f;
			sepAxiiX[25] = 0.0f;
			sepAxiiY[25] = 0.0f;
			sepAxiiZ[25] = -1.0f;
			sepAxiiX[26] = 1.0f;
			sepAxiiY[26] = 0.0f;
			sepAxiiZ[26] = 0.0f;
			sepAxiiX[27] = 0.0f;
			sepAxiiY[27] = 1.0f;
			sepAxiiZ[27] = 0.0f;
			sepAxiiX[28] = 0.0f;
			sepAxiiY[28] = 0.0f;
			sepAxiiZ[28] = 1.0f;
			for (int i = 0; i < 29; i++)
			{
				bareVec8f dots = _mm256_add_ps(_mm256_add_ps(_mm256_mul_ps(frustumCornersX, _mm256_set1_ps(sepAxiiX[i])), _mm256_mul_ps(frustumCornersY, _mm256_set1_ps(sepAxiiY[i]))), _mm256_mul_ps(frustumCornersZ, _mm256_set1_ps(sepAxiiZ[i])));
				float max = numeric_limits<float>::lowest();
				float min = numeric_limits<float>::max();
				for (int j = 0; j < 8; j++)
				{
					min = dots[j] < min ? dots[j] : min;
					max = dots[j] > max ? dots[j] : max;
				}
				frustumOnAxiiMax[i] = max;
				frustumOnAxiiMin[i] = min;

				if (i < 7)
				{
					max = numeric_limits<float>::lowest();
					min = numeric_limits<float>::max();
					for (int j = 0; j < 4; j++) //only consider near face which is first 4 of the vec8
					{
						min = dots[j] < min ? dots[j] : min;
						max = dots[j] > max ? dots[j] : max;
					}
					nearFaceOnAxiiMax[i] = max;
					nearFaceOnAxiiMin[i] = min;
				}
			}
		}
		CullFrustum(ProjectionData projData) {
			frustumCornersX.set(1.f, -1.f, 1.f, -1.f, 1.f, -1.f, 1.f, -1.f);
			frustumCornersY.set(1.f, 1.f, -1.f, -1.f, 1.f, 1.f, -1.f, -1.f);
			frustumCornersZ.set(1.f, 1.f, 1.f, 1.f, -1.f, -1.f, -1.f, -1.f);
			bareVec8f temp = _mm256_set1_ps(1.0f);
			projData.invVP.Project_Eight_Points(frustumCornersX, frustumCornersY, frustumCornersZ, temp);
			frustumCornersX = _mm256_div_ps(frustumCornersX, temp);
			frustumCornersY = _mm256_div_ps(frustumCornersY, temp);
			frustumCornersZ = _mm256_div_ps(frustumCornersZ, temp);
			SetSeperatingAxii();
		}

	};
}
