//
// Created by Thomas on 4/3/2015.
//
#pragma once
#include "RenderBox.h"
namespace SoftRendering {
	void RenderBox::setSeenFacesAndBorderOptimizations(ProjectionData &projectData, bool &threeFaceOptimization) {

		//first determine which faces are seen by taking dot product boxPos - cameraPos with cube normals (unit vectors cause axis aligned)
		// dot the positibve normals with the max corner of the cube, vice versa for min corner
		bareVec4f toMaxCorner = _mm_sub_ps(worldMax, projectData.cameraPos);
		bareVec4f toMinCorner = _mm_sub_ps(worldMin, projectData.cameraPos);
		bareVec4f dotForMaxFaces = toMaxCorner;
		bareVec4f dotForMinFaces = _mm_sub_ps(_mm_set1_ps(0), toMinCorner);

		//movemask works because it moves the sign bits into the integer, 1s where it was negative, so a bit of 1 means it seen
		int posfaces = _mm_movemask_ps(dotForMaxFaces);
		int negfaces = _mm_movemask_ps(dotForMinFaces);
		int isFaceSeen = posfaces ^ negfaces;
		int posOrNegSeen = posfaces;

		posOrNegFaceSeenBits = posOrNegSeen; isFaceSeenOnAxisBits = isFaceSeen;

		if((isFaceSeen & 0b111) == 0b111) //1 face per axii, either positive or negative. this means no pos x AND neg x, etc. Now it's possible to construct 6 line border for entire projected box
		{                                 //should be the same for the rest of the subtree too, this is important assumption
			threeFaceOptimization = true;
		}
	}

	void RenderBox::projectFace(int axis, int posFace, ProjectionData &projectData) { // axis is ordered 0 1 2 -> x y z, posFace is either 0 or 1 for neg or pos

		int otherAxis, otherOtherAxis;
		otherAxis = (axis ^ 1) & 1; //this actually works to get other numbers from the set 0-2, unintuitive, but works
		otherOtherAxis = (axis ^ 2) & 2;

		//instead of projecting just the border, project all 8 vertices, and select the border vertices in order to create the border lines
		//but this is possibly a bad idea because it requires an extra stage to generate all 8 corners
		//so instead were going to select correct vertices of the face first and then pack them into SoA for projecting

		bareVec4f selectionMask;

		//mask doesn't actually have to be all 1s for blend intrinsics, just a single 1 will do
		selectionMask.set(axis, ~(posFace - 1)); // that expression will EXPAND least significant bit and spread it to the other bits
		selectionMask.set(otherAxis, 0);
		selectionMask.set(otherOtherAxis, 0);

		bareVec4f corner = _mm_blendv_ps(worldMin, worldMax, selectionMask); // blend is basically a select function

		bareVec4f packedX = _mm_set1_ps(0.0f), packedY = _mm_set1_ps(0.0f), packedZ = _mm_set1_ps(0.0f);
		packedX.set(0, corner.get(0));
		packedY.set(0, corner.get(1));
		packedZ.set(0, corner.get(2));

		//next corner packing
		corner.set(otherAxis, worldMax.get(otherAxis));
		packedX.set(1, corner.get(0));
		packedY.set(1, corner.get(1));
		packedZ.set(1, corner.get(2));
		//next corner packing
		corner.set(otherOtherAxis, worldMax.get(otherOtherAxis));
		packedX.set(2, corner.get(0));
		packedY.set(2, corner.get(1));
		packedZ.set(2, corner.get(2));
		//next corner packing
		corner.set(otherAxis, worldMin.get(otherAxis));
		packedX.set(3, corner.get(0));
		packedY.set(3, corner.get(1));
		packedZ.set(3, corner.get(2));

		//now corners of the face are fully packed and ready for projection
		bareVec4f packedW = _mm_set1_ps(1.0f); //variable corner should have 1 as w so could do it the same way as others, but whatever
		projectData.viewProjection.Project_Four_SOA_Points(packedX, packedY, packedZ, packedW);

		getProjectedFaceBorderFromClipSpaceVerts(faces[axis], projectData, packedX, packedY, packedZ, packedW);

	}
	void RenderBox::getProjectedFaceBorderFromClipSpaceVerts(RenderBoxFaceProjection &face, ProjectionData &projectData, __m128 &packedX, __m128 &packedY, __m128 &packedZ, __m128 &packedW)
	{
		bareVec4f invW = _mm_rcp_ps(packedW);
		packedX = _mm_mul_ps(packedX, invW);
		packedY = _mm_mul_ps(packedY, invW);
		packedZ = _mm_mul_ps(packedZ, invW);
		//now in NDC space

		packedX = _mm_add_ps(packedX, _mm_set1_ps(1.0f));
		packedY = _mm_add_ps(packedY, _mm_set1_ps(1.0f));
		packedX = _mm_mul_ps(packedX, _mm_set1_ps(projectData.windowWidth * 0.5f));
		packedY = _mm_mul_ps(packedY, _mm_set1_ps(projectData.windowHeight * 0.5f));

		//from this point on shits in window space
		bareVec4f x = packedX;
		bareVec4f y = packedY;

		//set the bounding box to be used during rasterisation
		bareVec4f max = _mm_shuffle_ps(x, y, 0);
		max = _mm_max_ps(max, _mm_shuffle_ps(x, y, 0b01010101));
		max = _mm_max_ps(max, _mm_shuffle_ps(x, y, 0b10101010));
		max = _mm_max_ps(max, _mm_shuffle_ps(x, y, 0b11111111));
		bareVec4f min = _mm_shuffle_ps(x, y, 0);
		min = _mm_min_ps(min, _mm_shuffle_ps(x, y, 0b01010101));
		min = _mm_min_ps(min, _mm_shuffle_ps(x, y, 0b10101010));
		min = _mm_min_ps(min, _mm_shuffle_ps(x, y, 0b11111111));
		bareVec4f minmax = _mm_shuffle_ps(min, max, 0b10001000); //this shuffles in, min x, min y, max x, max y

		//window clipping
		//minmax = _mm_min_ps(minmax, _mm_set_ps((float)projectData.windowHeight, (float)projectData.windowWidth, (float)projectData.windowHeight, (float)projectData.windowWidth));
		//minmax = _mm_max_ps(minmax, _mm_set1_ps(0));
		face.border.minMaxBounds = minmax;

		bareVec4f xShifted = _mm_shuffle_ps(x, x, 0b10010011);
		bareVec4f yShifted = _mm_shuffle_ps(y, y, 0b10010011);

		bareVec4f xdif = _mm_sub_ps(x, xShifted);
		bareVec4f ydif = _mm_sub_ps(y, yShifted);

		bareVec4f xdif2 = _mm_mul_ps(xdif, xdif);
		bareVec4f ydif2 = _mm_mul_ps(ydif, ydif);
		bareVec4f rsqrt = _mm_rsqrt_ps(_mm_add_ps(xdif2, ydif2));

		bareVec4f normx = _mm_mul_ps(xdif, rsqrt);
		bareVec4f normy = _mm_mul_ps(ydif, rsqrt);

		bareVec4f copyx = normx;
		normx = _mm_mul_ps(normy, _mm_set1_ps(-1));
		normy = copyx;

		//line equation component creation
		bareVec4f A = normx;
		bareVec4f B = normy;
		bareVec4f C = _mm_mul_ps(_mm_add_ps(_mm_mul_ps(x, normx), _mm_mul_ps(y, normy)), _mm_set1_ps(-1.f)); //negation operator work? hopefully
																			 //an idea for winding order determination for creating correct "outside" line normal. Just create the line normal either way, then test if a known inside point is considered inside or not, if it's consider outside flip the normal																								 //implemented here to reverse normals as necessary
		float centerx = 0, centery = 0;
		for (int i = 0; i < 4; i++) {
			centerx += x.get(i);
			centery += y.get(i);
		}
		centerx /= 4;
		centery /= 4;
		bareVec4f dotCenter = _mm_add_ps(_mm_add_ps(_mm_mul_ps(A, _mm_set1_ps(centerx)), _mm_mul_ps(B, _mm_set1_ps(centery))), C);
		//if negative, flip normal, and the C component too i think, of line equation

		bareVec4f flipIndices = _mm_cmplt_ps(dotCenter, _mm_set1_ps(0));
		A = _mm_blendv_ps(_mm_sub_ps(_mm_set1_ps(0.0), A), A, flipIndices);
		B = _mm_blendv_ps(_mm_sub_ps(_mm_set1_ps(0.0), B), B, flipIndices);
		C = _mm_blendv_ps(_mm_sub_ps(_mm_set1_ps(0.0), C), C, flipIndices);

		face.border.A = A;
		face.border.B = B;
		face.border.C = C;
	}
	void RenderBox::getProjectedFaceBorder(int axis, int posFace, ProjectionData &projectData) { // axis is ordered 0 1 2 -> x y z, posFace is either 0 or 1 for neg or pos

		int otherAxis, otherOtherAxis;
		otherAxis = (axis ^ 1) & 1; //this actually works to get other numbers from the set 0-2, unintuitive, but works
		otherOtherAxis = (axis ^ 2) & 2;

		//instead of projecting just the border, project all 8 vertices, and select the border vertices in order to create the border lines
		//but this is possibly a bad idea because it requires an extra stage to generate all 8 corners
		//so instead were going to select correct vertices of the face first and then pack them into SoA for projecting

		bareVec4f selectionMask;

		//mask doesn't actually have to be all 1s for blend intrinsics, just a single 1 will do
		selectionMask.set(axis, ~(posFace - 1)); // that expression will EXPAND least significant bit and spread it to the other bits
		selectionMask.set(otherAxis, 0);
		selectionMask.set(otherOtherAxis, 0);

		bareVec4f corner = _mm_blendv_ps(worldMin, worldMax, selectionMask); // blend is basically a select function

		bareVec4f packedX = _mm_set1_ps(0.0f), packedY = _mm_set1_ps(0.0f), packedZ = _mm_set1_ps(0.0f);
		packedX.set(0, corner.get(0));
		packedY.set(0, corner.get(1));
		packedZ.set(0, corner.get(2));

		//next corner packing
		corner.set(otherAxis, worldMax.get(otherAxis));
		packedX.set(1, corner.get(0));
		packedY.set(1, corner.get(1));
		packedZ.set(1, corner.get(2));
		//next corner packing
		corner.set(otherOtherAxis, worldMax.get(otherOtherAxis));
		packedX.set(2, corner.get(0));
		packedY.set(2, corner.get(1));
		packedZ.set(2, corner.get(2));
		//next corner packing
		corner.set(otherAxis, worldMin.get(otherAxis));
		packedX.set(3, corner.get(0));
		packedY.set(3, corner.get(1));
		packedZ.set(3, corner.get(2));

		//now corners of the face are fully packed and ready for projection
		bareVec4f packedW = _mm_set1_ps(1.0f); //variable corner should have 1 as w so could do it the same way as others, but whatever
		projectData.viewProjection.Project_Four_SOA_Points(packedX, packedY, packedZ, packedW);

		//now in clip space
		bareVec4f negW = _mm_sub_ps(_mm_set1_ps(0.0f),packedW); //mul by -1 can be optimized by bitwise or on the sign bits
		bareVec4f behindZnearV = _mm_cmplt_ps(packedZ, negW);
		int behindZnear = _mm_movemask_ps(behindZnearV);

		if(behindZnear == 0)
		{
			faces[axis].clipped = false;
			getProjectedFaceBorderFromClipSpaceVerts(faces[axis], projectData, packedX, packedY, packedZ, packedW);
			return;
		}
		else if(behindZnear == 0b1111)
		{
			isFaceSeenOnAxisBits &= ~(1 << axis); return;
		}
		//border must be clipped if it makes it to to this point
		faces[axis].clipped = true;
		bareVec8f clippedXs, clippedYs, clippedZs, clippedWs;
		int clipVertStart = 4, clipVertEnd = -1;
		bool gapDetected = false, hit_a_clipped_vert = false, hit_a_nonclip_vert_after_hitting_a_clipped_vert = false;

		for (int i = 0; i < 4; i++)
		{
			if(behindZnearV.ints[i]) //if index of vertex that needs to be clipped
			{
				if (hit_a_nonclip_vert_after_hitting_a_clipped_vert) {
					gapDetected = true;
					clipVertStart = i;
					break;
				}
				hit_a_clipped_vert = true;
				clipVertStart = i < clipVertStart ? i : clipVertStart;
				clipVertEnd = i > clipVertEnd ? i : clipVertEnd;
			}
			else {
				if (hit_a_clipped_vert) hit_a_nonclip_vert_after_hitting_a_clipped_vert = true;
			}
		}
		//rearrange so that the range of clipped vertices is at the end of the 4 vertex array
		int shiftAmount = 3 - clipVertEnd;
		for (int i = 0; i < 4; i++) {
			int previousIndex = i - shiftAmount >= 0 ? i - shiftAmount : (i - shiftAmount) + 4;
			clippedXs[i] = packedX[previousIndex];
			clippedYs[i] = packedY[previousIndex];
			clippedZs[i] = packedZ[previousIndex];
			clippedWs[i] = packedW[previousIndex];
		}
		clipVertEnd = 3;
		clipVertStart = clipVertStart + shiftAmount > 3 ? clipVertStart + shiftAmount - 4 : clipVertStart + shiftAmount;
		//by shifting we ensure no gaps, a continuous segment of clipped vertices in arrary that doesn't wrap around the end to the start

		bareVec4f lineStart;
		int lineStartIndex = clipVertStart ? clipVertStart - 1 : 3;
		lineStart.x = clippedXs.get(lineStartIndex);
		lineStart.y = clippedYs.get(lineStartIndex);
		lineStart.z = clippedZs.get(lineStartIndex);
		lineStart.w = clippedWs.get(lineStartIndex);
		bareVec4f lineEnd;
		int lineEndIndex = clipVertStart;
		lineEnd.x = clippedXs.get(lineEndIndex);
		lineEnd.y = clippedYs.get(lineEndIndex);
		lineEnd.z = clippedZs.get(lineEndIndex);
		lineEnd.w = clippedWs.get(lineEndIndex);
		bareVec4f lineDelta = _mm_sub_ps(lineEnd, lineStart);
		float intersectTime = (lineStart.w + lineStart.z) / (-lineDelta.z - lineDelta.w);
		bareVec4f intersectPointOne = _mm_add_ps(lineStart, _mm_mul_ps(lineDelta, _mm_set1_ps(intersectTime)));
		lineStartIndex = clipVertEnd;
		lineStart.x = clippedXs.get(lineStartIndex);
		lineStart.y = clippedYs.get(lineStartIndex);
		lineStart.z = clippedZs.get(lineStartIndex);
		lineStart.w = clippedWs.get(lineStartIndex);
		lineEndIndex = clipVertEnd >= 3 ? 0 : clipVertEnd + 1;
		lineEnd.x = clippedXs.get(lineEndIndex);
		lineEnd.y = clippedYs.get(lineEndIndex);
		lineEnd.z = clippedZs.get(lineEndIndex);
		lineEnd.w = clippedWs.get(lineEndIndex);
		lineDelta = _mm_sub_ps(lineEnd, lineStart);
		intersectTime = (lineStart.w + lineStart.z) / (-lineDelta.z - lineDelta.w);
		bareVec4f intersectPointTwo = _mm_add_ps(lineStart, _mm_mul_ps(lineDelta, _mm_set1_ps(intersectTime)));
		int lastNonClipped;
		if(clipVertStart == clipVertEnd) //only 1 vertex inside clipped region
		{
			clippedXs[3] = intersectPointOne[0];
			clippedYs[3] = intersectPointOne[1];
			clippedZs[3] = intersectPointOne[2];
			clippedWs[3] = intersectPointOne[3];
			clippedXs[4] = intersectPointTwo[0];
			clippedYs[4] = intersectPointTwo[1];
			clippedZs[4] = intersectPointTwo[2];
			clippedWs[4] = intersectPointTwo[3];
			lastNonClipped = 4;
		}
		else //multiple vertices inside clipped region
		{
			clippedXs[clipVertStart] = intersectPointOne[0];
			clippedYs[clipVertStart] = intersectPointOne[1];
			clippedZs[clipVertStart] = intersectPointOne[2];
			clippedWs[clipVertStart] = intersectPointOne[3];
			clippedXs[clipVertStart + 1] = intersectPointTwo[0];
			clippedYs[clipVertStart + 1] = intersectPointTwo[1];
			clippedZs[clipVertStart + 1] = intersectPointTwo[2];
			clippedWs[clipVertStart + 1] = intersectPointTwo[3];
			lastNonClipped = clipVertStart + 1;
		}

		bareVec8f invW = _mm256_rcp_ps(clippedWs);
		clippedXs = _mm256_mul_ps(clippedXs, invW);
		clippedYs = _mm256_mul_ps(clippedYs, invW);
		clippedZs = _mm256_mul_ps(clippedZs, invW);
		//now in NDC space
		clippedXs = _mm256_add_ps(clippedXs, _mm256_set1_ps(1.0f));
		clippedYs = _mm256_add_ps(clippedYs, _mm256_set1_ps(1.0f));
		clippedXs = _mm256_mul_ps(clippedXs, _mm256_set1_ps(projectData.windowWidth * 0.5f));
		clippedYs = _mm256_mul_ps(clippedYs, _mm256_set1_ps(projectData.windowHeight * 0.5f));

		//from this point on shits in window space
		bareVec8f x = clippedXs;
		bareVec8f y = clippedYs;

		//set the bounding box to be used during rasterisation
		bareVec4f minmax = _mm_set_ps(-9999999.0f,-9999999.0f,9999999.0f,9999999.0f); //this is, min x, min y, max x, max y
		for (int i = 0; i <= lastNonClipped; i++)
		{
			minmax[0] = x[i] < minmax[0] ? x[i] : minmax[0];
			minmax[2] = x[i] > minmax[2] ? x[i] : minmax[2];
			minmax[1] = y[i] < minmax[1] ? y[i] : minmax[1];
			minmax[3] = y[i] > minmax[3] ? y[i] : minmax[3];
		}
		faces[axis].clippedBorder.minMaxBounds = minmax;

		bareVec8f xShifted; //shifted over 1, in preperation of line calculations
		bareVec8f yShifted;
		for (int i = 0; i <= lastNonClipped; i++)
		{
			int prevIndex = i - 1 < 0 ? lastNonClipped : i - 1;
			xShifted[i] = x[prevIndex];
			yShifted[i] = y[prevIndex];
		}

		bareVec8f xdif = _mm256_sub_ps(x, xShifted);
		bareVec8f ydif = _mm256_sub_ps(y, yShifted);

		bareVec8f xdif2 = _mm256_mul_ps(xdif, xdif);
		bareVec8f ydif2 = _mm256_mul_ps(ydif, ydif);
		bareVec8f rsqrt = _mm256_rsqrt_ps(_mm256_add_ps(xdif2, ydif2));

		bareVec8f normx = _mm256_mul_ps(xdif, rsqrt);
		bareVec8f normy = _mm256_mul_ps(ydif, rsqrt);

		bareVec8f copyx = normx;
		normx = _mm256_mul_ps(normy, _mm256_set1_ps(-1));
		normy = copyx;

		//line equation component creation
		bareVec8f A = normx;
		bareVec8f B = normy;
		bareVec8f C = _mm256_mul_ps(_mm256_add_ps(_mm256_mul_ps(x, normx), _mm256_mul_ps(y, normy)), _mm256_set1_ps(-1.f)); 

																											 //an idea for winding order determination for creating correct "outside" line normal. Just create the line normal either way, then test if a known inside point is considered inside or not, if it's consider outside flip the normal
																											 //implemented here to reverse normals as necessary
		float centerx = 0, centery = 0;
		for (int i = 0; i <= lastNonClipped; i++) {
			centerx += x.get(i);
			centery += y.get(i);
		}
		centerx /= lastNonClipped +1;
		centery /= lastNonClipped+1;
		bareVec8f dotCenter = _mm256_add_ps(_mm256_add_ps(_mm256_mul_ps(A, _mm256_set1_ps(centerx)), _mm256_mul_ps(B, _mm256_set1_ps(centery))), C);
		//if positive, flip normal, and the C component too i think, of line equation

		bareVec8f flipIndices = _mm256_cmp_ps(dotCenter, _mm256_set1_ps(0) , 0x11); // compare lessthan
		A = _mm256_blendv_ps(_mm256_sub_ps(_mm256_set1_ps(0.0), A), A, flipIndices);
		B = _mm256_blendv_ps(_mm256_sub_ps(_mm256_set1_ps(0.0), B), B, flipIndices);
		C = _mm256_blendv_ps(_mm256_sub_ps(_mm256_set1_ps(0.0), C), C, flipIndices);
		for (int i = lastNonClipped+1; i < 8; i++) {
			A[i] = 0.f;
			B[i] = 0.f;
			C[i] = -3.14f; //negative, so every point is always considered on the "inside" of this dummy line
		}
		faces[axis].clippedBorder.A = A;
		faces[axis].clippedBorder.B = B;
		faces[axis].clippedBorder.C = C;
	}
	void RenderBox::getProjectedOneFacePerAxisBorder(int posFaceIsSeen, ProjectionData &projectData) { // axis is ordered 0 1 2 -> x y z, posFace is either 0 or 1 for neg or pos

		int xPosOrNegFace = (posFaceIsSeen) & 1, yPosOrNegFace = (posFaceIsSeen >> 1) & 1, zPosOrNegFace = (posFaceIsSeen >> 2)& 1; //which is seen, 1 on each axis

		bareVec4f corner;
		corner.x = xPosOrNegFace ? worldMax.x : worldMin.x;
		corner.y = yPosOrNegFace ? worldMin.y : worldMax.y;
		corner.z = zPosOrNegFace ? worldMin.z : worldMax.z;

		bareVec8f packedX = _mm256_set1_ps(0.0f), packedY = _mm256_set1_ps(0.0f), packedZ = _mm256_set1_ps(0.0f);
		packedX.set(0, corner.get(0));
		packedY.set(0, corner.get(1));
		packedZ.set(0, corner.get(2));
		//next corner packing
		corner.y = yPosOrNegFace ? worldMax.y : worldMin.y; 
		packedX.set(1, corner.get(0));
		packedY.set(1, corner.get(1));
		packedZ.set(1, corner.get(2));
		//next corner packing
		corner.x = xPosOrNegFace ? worldMin.x : worldMax.x;
		packedX.set(2, corner.get(0));
		packedY.set(2, corner.get(1));
		packedZ.set(2, corner.get(2));
		//next corner packing
		corner.z = zPosOrNegFace ? worldMax.z : worldMin.z;
		packedX.set(3, corner.get(0));
		packedY.set(3, corner.get(1));
		packedZ.set(3, corner.get(2));
		//next corner packing
		corner.y = yPosOrNegFace ? worldMin.y : worldMax.y;
		packedX.set(4, corner.get(0));
		packedY.set(4, corner.get(1));
		packedZ.set(4, corner.get(2));
		//next corner packing
		corner.x = xPosOrNegFace ? worldMax.x : worldMin.x;
		packedX.set(5, corner.get(0));
		packedY.set(5, corner.get(1));
		packedZ.set(5, corner.get(2));

		//now corners of the face are fully packed and ready for projection
		bareVec8f packedW = _mm256_set1_ps(1.0f); 
		projectData.viewProjection.Project_Eight_Points(packedX, packedY, packedZ, packedW);

		bareVec8f negW = _mm256_sub_ps(_mm256_set1_ps(0.0f), packedW); 
		bareVec8f behindZnearV = _mm256_cmp_ps(packedZ, negW, 0x11);
		int behindZnear = _mm256_movemask_ps(behindZnearV) & 0b111111;

		if (behindZnear == 0)
		{
			//starting with 6 clip space verts, need to create a window border for the projected box. the window border will have 6 lines, there was no clipping
			bareVec8f invW = _mm256_rcp_ps(packedW);
			packedX = _mm256_mul_ps(packedX, invW);
			packedY = _mm256_mul_ps(packedY, invW);
			packedZ = _mm256_mul_ps(packedZ, invW);
			//now in NDC space
			packedX = _mm256_add_ps(packedX, _mm256_set1_ps(1.0f));
			packedY = _mm256_add_ps(packedY, _mm256_set1_ps(1.0f));
			packedX = _mm256_mul_ps(packedX, _mm256_set1_ps(projectData.windowWidth * 0.5f));
			packedY = _mm256_mul_ps(packedY, _mm256_set1_ps(projectData.windowHeight * 0.5f));
			//from this point on shits in window space
			bareVec8f x = packedX;
			bareVec8f y = packedY;
			//set the bounding box to be used during rasterisation
			float maxX = packedX[0], maxY = packedY[0], minX = packedX[0], minY = packedY[0]; //todo optimize this getting min/max with vector instructions somehow
			for (int i = 1; i < 6; i++ )
			{
				maxX = packedX[i] > maxX ? packedX[i] : maxX;
				maxY = packedY[i] > maxY ? packedY[i] : maxY;
				minX = packedX[i] < minX ? packedX[i] : minX;
				minY = packedY[i] < minY ? packedY[i] : minY;
				
			}
			bareVec4f minmax = _mm_set_ps(maxY, maxX, minY, minX); //this sets in, min x, min y, max x, max y

			entireProjection.windowBorder.minMaxBounds = minmax;

			bareVec8f xShifted = _mm256_permutevar_ps(x, _mm256_set_epi32(0b0,0b0,0b0,0b1,0b0,0b11,0b10,0b1));
			bareVec8f temp1 = _mm256_permute_ps(x, 0b0000);
			bareVec8f temp2 = _mm256_permute2f128_ps(xShifted, temp1, 0b00100001); //indices in the immediate are 4 bits in this
			xShifted = _mm256_blend_ps(xShifted, _mm256_permute2f128_ps(xShifted, _mm256_permute_ps(x, 0b0000), 0b00100001), 0b101000);
			bareVec8f yShifted = _mm256_permutevar_ps(y, _mm256_set_epi32(0b0, 0b0, 0b0, 0b1, 0b0, 0b11, 0b10, 0b1));
			yShifted = _mm256_blend_ps(yShifted, _mm256_permute2f128_ps(yShifted, _mm256_permute_ps(y, 0b0000), 0b00100001), 0b101000);

			bareVec8f xdif = _mm256_sub_ps(x, xShifted);
			bareVec8f ydif = _mm256_sub_ps(y, yShifted);

			bareVec8f xdif2 = _mm256_mul_ps(xdif, xdif);
			bareVec8f ydif2 = _mm256_mul_ps(ydif, ydif);
			bareVec8f rsqrt = _mm256_rsqrt_ps(_mm256_add_ps(xdif2, ydif2));

			bareVec8f normx = _mm256_mul_ps(xdif, rsqrt);
			bareVec8f normy = _mm256_mul_ps(ydif, rsqrt);

			bareVec8f copyx = normx;
			normx = _mm256_mul_ps(normy, _mm256_set1_ps(-1));
			normy = copyx;

			//line equation component creation
			bareVec8f A = normx;
			bareVec8f B = normy;
			bareVec8f C = _mm256_mul_ps(_mm256_add_ps(_mm256_mul_ps(x, normx), _mm256_mul_ps(y, normy)), _mm256_set1_ps(-1.f));
			//make a dummy "always inside" line for 7tha nd8th line which aren't used
			A.set(6, 0.0f);
			A.set(7, 0.0f);
			B.set(6, 0.0f);
			B.set(7, 0.0f);
			C.set(6, -1.0f);
			C.set(7, -1.0f);
		    //an idea for winding order determination for creating correct "outside" line normal. Just create the line normal either way, then test if a known inside point is considered inside or not, if it's consider outside flip the normal																								 //implemented here to reverse normals as necessary
			float centerx = 0, centery = 0;
			for (int i = 0; i < 6; i++) {
				centerx += x.get(i);
				centery += y.get(i);
			}
			centerx /= 6;
			centery /= 6;
			bareVec8f dotCenter = _mm256_add_ps(_mm256_add_ps(_mm256_mul_ps(A, _mm256_set1_ps(centerx)), _mm256_mul_ps(B, _mm256_set1_ps(centery))), C);
			//above variable contains "classifications" for whether or not the known inside point is considered inside for the line or not, if it's considered outiside, normal must be flipped
			// a negative value will insidcate it's behind the normal thus unside, so were defining normals to point outward and negative classification tomean inside
			
			bareVec8f flipIndices = _mm256_cmp_ps(dotCenter, _mm256_set1_ps(0), 0x11); //if true, don't flip, misleading name
			A = _mm256_blendv_ps(_mm256_sub_ps(_mm256_set1_ps(0.0), A), A, flipIndices);
			B = _mm256_blendv_ps(_mm256_sub_ps(_mm256_set1_ps(0.0), B), B, flipIndices);
			C = _mm256_blendv_ps(_mm256_sub_ps(_mm256_set1_ps(0.0), C), C, flipIndices);

			entireProjection.windowBorder.A = A;
			entireProjection.windowBorder.B = B;
			entireProjection.windowBorder.C = C;
			return;
		}
		else if ((behindZnear & 0b111111) == 0b111111)
		{
			//this should never happen, all box not visible, should be handled by frustum culling
		}
		//border must be clipped if it makes it to to this point
		bareVec8f clippedXs, clippedYs, clippedZs, clippedWs;
		int clipVertStart = 6, clipVertEnd = -1;
		bool gapDetected = false, hit_a_clipped_vert = false, hit_a_nonclip_vert_after_hitting_a_clipped_vert = false;

		for (int i = 0; i < 6; i++)
		{
			if (behindZnearV.ints[i]) //if index of vertex that needs to be clipped
			{
				if (hit_a_nonclip_vert_after_hitting_a_clipped_vert) {
					gapDetected = true;
					clipVertStart = i;
					break;
				}
				hit_a_clipped_vert = true;
				clipVertStart = i < clipVertStart ? i : clipVertStart;
				clipVertEnd = i > clipVertEnd ? i : clipVertEnd;
			}
			else {
				if (hit_a_clipped_vert) hit_a_nonclip_vert_after_hitting_a_clipped_vert = true;
			}
		}
		//rearrange so that the range of clipped vertices is at the end of the 8 vertex array
		int shiftAmount = 5 - clipVertEnd;
		for (int i = 0; i < 6; i++) {
			int previousIndex = i - shiftAmount >= 0 ? i - shiftAmount : (i - shiftAmount) + 6;
			clippedXs[i] = packedX[previousIndex];
			clippedYs[i] = packedY[previousIndex];
			clippedZs[i] = packedZ[previousIndex];
			clippedWs[i] = packedW[previousIndex];
		}
		clipVertEnd = 5;
		clipVertStart = clipVertStart + shiftAmount > 5 ? clipVertStart + shiftAmount - 6 : clipVertStart + shiftAmount;
		//by shifting we ensure no gaps, a continuous segment of clipped vertices in arrary that doesn't wrap around the end to the start

		bareVec4f lineStart;
		int lineStartIndex = clipVertStart ? clipVertStart - 1 : 5;
		lineStart.x = clippedXs.get(lineStartIndex);
		lineStart.y = clippedYs.get(lineStartIndex);
		lineStart.z = clippedZs.get(lineStartIndex);
		lineStart.w = clippedWs.get(lineStartIndex);
		bareVec4f lineEnd;
		int lineEndIndex = clipVertStart;
		lineEnd.x = clippedXs.get(lineEndIndex);
		lineEnd.y = clippedYs.get(lineEndIndex);
		lineEnd.z = clippedZs.get(lineEndIndex);
		lineEnd.w = clippedWs.get(lineEndIndex);
		bareVec4f lineDelta = _mm_sub_ps(lineEnd, lineStart);
		float intersectTime = (lineStart.w + lineStart.z) / (-lineDelta.z - lineDelta.w);
		bareVec4f intersectPointOne = _mm_add_ps(lineStart, _mm_mul_ps(lineDelta, _mm_set1_ps(intersectTime)));
		lineStartIndex = clipVertEnd;
		lineStart.x = clippedXs.get(lineStartIndex);
		lineStart.y = clippedYs.get(lineStartIndex);
		lineStart.z = clippedZs.get(lineStartIndex);
		lineStart.w = clippedWs.get(lineStartIndex);
		lineEndIndex = clipVertEnd >= 5 ? 0 : clipVertEnd + 1;
		lineEnd.x = clippedXs.get(lineEndIndex);
		lineEnd.y = clippedYs.get(lineEndIndex);
		lineEnd.z = clippedZs.get(lineEndIndex);
		lineEnd.w = clippedWs.get(lineEndIndex);
		lineDelta = _mm_sub_ps(lineEnd, lineStart);
		intersectTime = (lineStart.w + lineStart.z) / (-lineDelta.z - lineDelta.w);
		bareVec4f intersectPointTwo = _mm_add_ps(lineStart, _mm_mul_ps(lineDelta, _mm_set1_ps(intersectTime)));
		int lastVertexIndex;
		if (clipVertStart == clipVertEnd) //only 1 vertex inside clipped region
		{
			clippedXs[5] = intersectPointOne[0];
			clippedYs[5] = intersectPointOne[1];
			clippedZs[5] = intersectPointOne[2];
			clippedWs[5] = intersectPointOne[3];
			clippedXs[6] = intersectPointTwo[0];
			clippedYs[6] = intersectPointTwo[1];
			clippedZs[6] = intersectPointTwo[2];
			clippedWs[6] = intersectPointTwo[3];
			lastVertexIndex = 6;
		}
		else //multiple vertices inside clipped region
		{
			clippedXs[clipVertStart] = intersectPointOne[0];
			clippedYs[clipVertStart] = intersectPointOne[1];
			clippedZs[clipVertStart] = intersectPointOne[2];
			clippedWs[clipVertStart] = intersectPointOne[3];
			clippedXs[clipVertStart + 1] = intersectPointTwo[0];
			clippedYs[clipVertStart + 1] = intersectPointTwo[1];
			clippedZs[clipVertStart + 1] = intersectPointTwo[2];
			clippedWs[clipVertStart + 1] = intersectPointTwo[3];
			lastVertexIndex = clipVertStart + 1;
		}

		bareVec8f invW = _mm256_rcp_ps(clippedWs);
		clippedXs = _mm256_mul_ps(clippedXs, invW);
		clippedYs = _mm256_mul_ps(clippedYs, invW);
		clippedZs = _mm256_mul_ps(clippedZs, invW);
		//now in NDC space
		clippedXs = _mm256_add_ps(clippedXs, _mm256_set1_ps(1.0f));
		clippedYs = _mm256_add_ps(clippedYs, _mm256_set1_ps(1.0f));
		clippedXs = _mm256_mul_ps(clippedXs, _mm256_set1_ps(projectData.windowWidth * 0.5f));
		clippedYs = _mm256_mul_ps(clippedYs, _mm256_set1_ps(projectData.windowHeight * 0.5f));

		//from this point on shits in window space
		bareVec8f x = clippedXs;
		bareVec8f y = clippedYs;

		//set the bounding box to be used during rasterisation
		bareVec4f minmax = _mm_set_ps(-9999999.0f, -9999999.0f, 9999999.0f, 9999999.0f); //this is, min x, min y, max x, max y
		for (int i = 0; i <= lastVertexIndex; i++)
		{
			minmax[0] = x[i] < minmax[0] ? x[i] : minmax[0];
			minmax[2] = x[i] > minmax[2] ? x[i] : minmax[2];
			minmax[1] = y[i] < minmax[1] ? y[i] : minmax[1];
			minmax[3] = y[i] > minmax[3] ? y[i] : minmax[3];
		}
		entireProjection.windowBorder.minMaxBounds = minmax;

		bareVec8f xShifted; //shifted over 1, in preperation of line calculations
		bareVec8f yShifted;
		for (int i = 0; i <= lastVertexIndex; i++)
		{
			int prevIndex = i - 1 < 0 ? lastVertexIndex : i - 1;
			xShifted[i] = x[prevIndex];
			yShifted[i] = y[prevIndex];
		}

		bareVec8f xdif = _mm256_sub_ps(x, xShifted);
		bareVec8f ydif = _mm256_sub_ps(y, yShifted);

		bareVec8f xdif2 = _mm256_mul_ps(xdif, xdif);
		bareVec8f ydif2 = _mm256_mul_ps(ydif, ydif);
		bareVec8f rsqrt = _mm256_rsqrt_ps(_mm256_add_ps(xdif2, ydif2));

		bareVec8f normx = _mm256_mul_ps(xdif, rsqrt);
		bareVec8f normy = _mm256_mul_ps(ydif, rsqrt);

		bareVec8f copyx = normx;
		normx = _mm256_mul_ps(normy, _mm256_set1_ps(-1));
		normy = copyx;

		//line equation component creation
		bareVec8f A = normx;
		bareVec8f B = normy;
		bareVec8f C = _mm256_mul_ps(_mm256_add_ps(_mm256_mul_ps(x, normx), _mm256_mul_ps(y, normy)), _mm256_set1_ps(-1.f));

		//an idea for winding order determination for creating correct "outside" line normal. Just create the line normal either way, then test if a known inside point is considered inside or not, if it's consider outside flip the normal
		//implemented here to reverse normals as necessary
		float centerx = 0, centery = 0;
		for (int i = 0; i <= lastVertexIndex; i++) {
			centerx += x.get(i);
			centery += y.get(i);
		}
		centerx /= lastVertexIndex + 1;
		centery /= lastVertexIndex + 1;
		bareVec8f dotCenter = _mm256_add_ps(_mm256_add_ps(_mm256_mul_ps(A, _mm256_set1_ps(centerx)), _mm256_mul_ps(B, _mm256_set1_ps(centery))), C);
		//if positive, flip normal, and the C component too i think, of line equation

		bareVec8f flipIndices = _mm256_cmp_ps(dotCenter, _mm256_set1_ps(0), 0x11); // compare lessthan
		A = _mm256_blendv_ps(_mm256_sub_ps(_mm256_set1_ps(0.0), A), A, flipIndices);
		B = _mm256_blendv_ps(_mm256_sub_ps(_mm256_set1_ps(0.0), B), B, flipIndices);
		C = _mm256_blendv_ps(_mm256_sub_ps(_mm256_set1_ps(0.0), C), C, flipIndices);
		for (int i = lastVertexIndex + 1; i < 8; i++) {
			A[i] = 0.f;
			B[i] = 0.f;
			C[i] = -3.14f; //negative, so every point is always considered on the "inside" of this dummy line
		}
		entireProjection.windowBorder.A = A;
		entireProjection.windowBorder.B = B;
		entireProjection.windowBorder.C = C;
	}
}