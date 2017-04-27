#pragma once
#include "VoxelRenderBase.h"
#include "Octree/Node.h"
#include "Render sys/Renderbox.h"
#include "Render sys/QuadVAO.h"
#include "QuadMask/QuadMask.h"
#include "CullFrustum.h"

using namespace Octree;
using namespace SoftRendering;

class VisibilitySys
{
public:
	struct GenData 
	{
		QuadMask *quadMask;
		QuadVAO *quadVAO;
		ProjectionData *projData;
		CullFrustum *cullFrustum;
		int maxTreeLevel;
	};
	//struct copied from recursive call to recursive call
	struct RecurseData {
		bareVec8f boxSizeX;
		bareVec8f boxSizeY;
		bareVec8f boxSizeZ;
		int seperatingAxiiCheckMask; //using 0-28, a bit of 1 indicates the corresponding axis must be checked for culling, used for frustum culling
		int sepAxiiCheckMaskNearFace; //used for keeping track of near face intersection testing					
		int level;
		bool intersectingScreen;
		bool threeFaceOptimization; //if same 3 faces are seen through entire subtree
	};


	static bareVec8f octantMaskSOAx;
	static bareVec8f octantMaskSOAy;
	static bareVec8f octantMaskSOAz;
	inline static void initOctantMaskSOA() {

		octantMaskSOAx.set(~0, 0, ~0, 0, ~0, 0, ~0, 0);
		octantMaskSOAy.set(~0, ~0, 0, 0, ~0, ~0, 0, 0);
		octantMaskSOAz.set(~0, ~0, ~0, ~0, 0, 0, 0, 0);
		// ---, --+, -+-, -++, etc, order: zyx
	}
	inline static void FindVisibileVoxels(GenData &genData, Node &rootNode, bareVec4f rootWorldMin, bareVec4f rootWorldMax)
	{

		static int start = 0;
		if (start == 0)
		{
			start++; initOctantMaskSOA();
		}
		//quadMask->Clear();
		RecurseData recurseData;
		recurseData.level = -1;
		bareVec4f rootNodeWorldMin = rootWorldMin;
		//bareVec4f rootNodeWorldMax = _mm_set_ps(1.0f,0.4f,0.4f,0.4f);
		bareVec4f rootNodeWorldMax = rootWorldMax;
		bareVec4f worldSize = _mm_sub_ps(rootNodeWorldMax, rootNodeWorldMin);
		recurseData.boxSizeX = _mm256_set1_ps(worldSize.X() *2);//needs to be *2 cause that's the fake parents size, needed to start recursion
		recurseData.boxSizeY = _mm256_set1_ps(worldSize.Y() *2);
		recurseData.boxSizeZ = _mm256_set1_ps(worldSize.Z() *2); 

		RenderBox renderBox;
		renderBox.worldMin = rootNodeWorldMin;	
		renderBox.clipSpaceMin = genData.projData->viewProjection.ProjectPoint(renderBox.worldMin);
		renderBox.worldMax = rootNodeWorldMax;

		recurseData.seperatingAxiiCheckMask = 0x1fffffff; //first 29 bits are 1 cause 29 seperating axii for frustum -aabb
		recurseData.sepAxiiCheckMaskNearFace = 0x7f; //only 7 sep axxi for near face]
		for (int i = 0; i < 29; i++)
		{
			bool allZero = (genData.cullFrustum->sepAxiiX[i] == 0) && (genData.cullFrustum->sepAxiiY[i] == 0) && (genData.cullFrustum->sepAxiiZ[i] == 0); //if it's also zero that means there was a parallel cross product, you just don't check these axii
			if(allZero)
			{
				recurseData.seperatingAxiiCheckMask &= ~(1 << i);
				recurseData.sepAxiiCheckMaskNearFace &= ~(1 << i);
			}
		}
		recurseData.intersectingScreen = true;
		recurseData.threeFaceOptimization = false;
		FindVisibleVoxelsRecursive(genData, recurseData, rootNode, renderBox);
	}
	inline static void FindVisibleVoxelsRecursive(GenData &genData, RecurseData const &parentRecurseData, Node &node, RenderBox &renderBox)
 	{
		//forward declarations that are useful through multiple sections of this function
		bareVec8f worldMinX, worldMinY, worldMinZ, worldMaxX, worldMaxY, worldMaxZ;
		bareVec8f worldCornersX, worldCornersY, worldCornersZ;
		RecurseData curRecurseData;
		//fill vec8s with this nodes location
		worldMinX = _mm256_set1_ps(renderBox.worldMin.X());
		worldMinY = _mm256_set1_ps(renderBox.worldMin.Y());
		worldMinZ = _mm256_set1_ps(renderBox.worldMin.Z());
		worldMaxX = _mm256_set1_ps(renderBox.worldMax.X());
		worldMaxY = _mm256_set1_ps(renderBox.worldMax.Y());
		worldMaxZ = _mm256_set1_ps(renderBox.worldMax.Z());
		curRecurseData.boxSizeX = _mm256_mul_ps(parentRecurseData.boxSizeX, _mm256_set1_ps(0.5f));
		curRecurseData.boxSizeY = _mm256_mul_ps(parentRecurseData.boxSizeY, _mm256_set1_ps(0.5f));
		curRecurseData.boxSizeZ = _mm256_mul_ps(parentRecurseData.boxSizeZ, _mm256_set1_ps(0.5f));
		worldCornersX = _mm256_add_ps(worldMinX, _mm256_and_ps(octantMaskSOAx, curRecurseData.boxSizeX)); //the x components of the corners of the current box
		worldCornersY = _mm256_add_ps(worldMinY, _mm256_and_ps(octantMaskSOAy, curRecurseData.boxSizeY));
		worldCornersZ = _mm256_add_ps(worldMinZ, _mm256_and_ps(octantMaskSOAz, curRecurseData.boxSizeZ));

		curRecurseData.level = parentRecurseData.level + 1;
		curRecurseData.seperatingAxiiCheckMask = parentRecurseData.seperatingAxiiCheckMask;
		curRecurseData.sepAxiiCheckMaskNearFace = parentRecurseData.sepAxiiCheckMaskNearFace;
		curRecurseData.threeFaceOptimization = parentRecurseData.threeFaceOptimization;
		curRecurseData.intersectingScreen = parentRecurseData.intersectingScreen; //if parent was intersecting, we assume this is interesting until proven not.
		//end forward declarations

		//level culling
		if (curRecurseData.level > genData.maxTreeLevel)
		{
			return;
		}

		//frustum culling
		if (curRecurseData.seperatingAxiiCheckMask | parentRecurseData.intersectingScreen) //if anything needs to be checked, check it
		{
			if (isBoxOutsideFrustum(genData, curRecurseData, parentRecurseData.intersectingScreen, worldCornersX, worldCornersY, worldCornersZ)) //if parent is intersecting, that means child may be intersecting, when this terminates current screen intersection is set properly
			{
				return; //box outside, cull entire sub tree
			}
			
		}
		//if (recurseData.level == 1) cout << "level 0: " << _CrtCheckMemory() << "\n";
		//end frustum culling

		//start generate border
		//curRecurseData.threeFaceOptimization = false; //testcode
		if (curRecurseData.threeFaceOptimization) {
			//no prep needed
		}
		else {
			//this is set up for the border generation, figure out what optimizations to use, which faces are seen, and set children for later
			renderBox.setSeenFacesAndBorderOptimizations(*genData.projData, curRecurseData.threeFaceOptimization); //this will turn on threefaceoptimization if it should be used for remainder of function
		}
		//curRecurseData.threeFaceOptimization = false;
		float borderArea = 0; bool smallerThanPixel; float minDimension = 9999.0f; //if very small in one dimension, should probably stop traversal here cause it's too thin or whatever
		if(curRecurseData.threeFaceOptimization)
		{
			renderBox.getProjectedOneFacePerAxisBorder(renderBox.posOrNegFaceSeenBits, *genData.projData);
			borderArea = (renderBox.entireProjection.windowBorder.minMaxBounds.z - renderBox.entireProjection.windowBorder.minMaxBounds.x) * (renderBox.entireProjection.windowBorder.minMaxBounds.w - renderBox.entireProjection.windowBorder.minMaxBounds.y);
			float width = (renderBox.entireProjection.windowBorder.minMaxBounds.z - renderBox.entireProjection.windowBorder.minMaxBounds.x);
			float height = (renderBox.entireProjection.windowBorder.minMaxBounds.w - renderBox.entireProjection.windowBorder.minMaxBounds.y);
			minDimension = width < height ? width : height;
		}
		else
		{
			for (int i = 0; i < 3; i++)
			{
				if ((renderBox.isFaceSeenOnAxisBits >> i) & 1)
				{
					renderBox.getProjectedFaceBorder(i, (renderBox.posOrNegFaceSeenBits >> i) & 1, *genData.projData);
					if (renderBox.faces[i].clipped) { borderArea += (renderBox.faces[i].clippedBorder.minMaxBounds.z - renderBox.faces[i].clippedBorder.minMaxBounds.x) * (renderBox.faces[i].clippedBorder.minMaxBounds.w - renderBox.faces[i].clippedBorder.minMaxBounds.y); }
					else {
						borderArea += (renderBox.faces[i].border.minMaxBounds.z - renderBox.faces[i].border.minMaxBounds.x) * (renderBox.faces[i].border.minMaxBounds.w - renderBox.faces[i].border.minMaxBounds.y);
					}
				}
			}
		}
		smallerThanPixel = borderArea < 10.0f || minDimension < 6.0f; //border area is a little bigger than actual area of border cause it uses AABBs. minDimension too but less so, we use higher comparison values than 1.0f (which is length of pixel)
		//end generate border

		//only do occlusion culling when camera near face is not intersecting box
		if (!curRecurseData.intersectingScreen) {
			//occlusion culling
			if (curRecurseData.threeFaceOptimization)
			{
				if (borderArea < 50000.0f) { //do not consider occlusion culling if big node
					int fullyOccluded = genData.quadMask->isQuadFullyOccluded_stamp4<8>(renderBox.entireProjection.windowBorder);
					if (fullyOccluded) return;
				}
//				else
//				{
//					int dicks = 9;
//				}
				//do stuff for leaf
				if (node.isLeaf() || curRecurseData.level >= genData.maxTreeLevel || smallerThanPixel) {
					for (int i = 0; i < 3; i++) {
						genData.quadVAO->pushBoxFace(renderBox.worldMin, renderBox.worldMax, (renderBox.posOrNegFaceSeenBits >> i) & 1, i, node.color); //7 2 4728
					}
					genData.quadMask->DrawQuad_stamp4<8>(renderBox.entireProjection.windowBorder);
					return;
				};
			}
			else
			{
				if (borderArea < 50000.0f) { //do not consider occlusion culling if big node
					int fullyOccluded = 1;
					for (int i = 0; i < 3; i++) {
						if ((renderBox.isFaceSeenOnAxisBits >> i) & 1) {
							if (!renderBox.faces[i].clipped) { fullyOccluded &= genData.quadMask->isQuadFullyOccluded_stamp4<4>(renderBox.faces[i].border); }
							else { fullyOccluded &= genData.quadMask->isQuadFullyOccluded_stamp4<8>(renderBox.faces[i].clippedBorder); } //todo: test changing 8 lines to 6 lines
						}
					}
					if (fullyOccluded) return;
				}
				//do stuff for leaf
				if (node.isLeaf() || curRecurseData.level >= genData.maxTreeLevel || smallerThanPixel) {
					//TODO: implement
					for (int i = 0; i < 3; i++) {
						if ((renderBox.isFaceSeenOnAxisBits >> i) & 1) {
							genData.quadVAO->pushBoxFace(renderBox.worldMin, renderBox.worldMax, (renderBox.posOrNegFaceSeenBits >> i) & 1, i, node.color);
							if (!renderBox.faces[i].clipped) { genData.quadMask->DrawQuad_stamp4<4>(renderBox.faces[i].border); }
							else
							{
								genData.quadMask->DrawQuad_stamp4<8>(renderBox.faces[i].clippedBorder); //todo test changing 8 to 6
							}
						}
					}
					return;
				};
			}
		}
		childrenCode:
		//do stuff for non-leaf
		RenderBox childBoxs[8];
		float distances[8];
		int sortedIndices[8];

		bareVec8f halfBoxSizeX = _mm256_mul_ps(curRecurseData.boxSizeX, _mm256_set1_ps(0.5f));
		bareVec8f halfBoxSizeY = _mm256_mul_ps(curRecurseData.boxSizeY, _mm256_set1_ps(0.5f));
		bareVec8f halfBoxSizeZ = _mm256_mul_ps(curRecurseData.boxSizeZ, _mm256_set1_ps(0.5f));
		bareVec8f childrenWorldMinX, childrenWorldMinY, childrenWorldMinZ, childrenWorldMaxX, childrenWorldMaxY, childrenWorldMaxZ;
		bareVec8f childrenClipSpaceMinX, childrenClipSpaceMinY, childrenClipSpaceMinZ, childrenClipSpaceMinW;
		//get children SOA positions
		childrenWorldMinX = _mm256_add_ps(worldMinX, _mm256_and_ps(octantMaskSOAx, halfBoxSizeX));
		childrenWorldMinY = _mm256_add_ps(worldMinY, _mm256_and_ps(octantMaskSOAy, halfBoxSizeY));
		childrenWorldMinZ = _mm256_add_ps(worldMinZ, _mm256_and_ps(octantMaskSOAz, halfBoxSizeZ));
		//now get max positons
		childrenWorldMaxX = _mm256_add_ps(childrenWorldMinX, halfBoxSizeX);
		childrenWorldMaxY = _mm256_add_ps(childrenWorldMinY, halfBoxSizeY);
		childrenWorldMaxZ = _mm256_add_ps(childrenWorldMinZ, halfBoxSizeZ);
		//now get clipspace positions
		genData.projData->viewProjection.Project_Eight_Points_out(childrenWorldMinX, childrenWorldMinY, childrenWorldMinZ, _mm256_set1_ps(1.0f), childrenClipSpaceMinX, childrenClipSpaceMinY, childrenClipSpaceMinZ, childrenClipSpaceMinW);    //straight projection best way i think
		
		// only the distances to the center work for sorting most infront
		//get squared distances from camera to children
		bareVec8f childrenCenterX = _mm256_mul_ps(_mm256_add_ps(childrenWorldMinX, childrenWorldMaxX), _mm256_set1_ps(0.5f));
		bareVec8f childrenCenterY = _mm256_mul_ps(_mm256_add_ps(childrenWorldMinY, childrenWorldMaxY), _mm256_set1_ps(0.5f));
		bareVec8f childrenCenterZ = _mm256_mul_ps(_mm256_add_ps(childrenWorldMinZ, childrenWorldMaxZ), _mm256_set1_ps(0.5f));
	    bareVec8f temp8 = _mm256_sub_ps(childrenCenterX, genData.projData->cameraPosX_SOA);
		bareVec8f x2 = _mm256_mul_ps(temp8, temp8);
		temp8 = _mm256_sub_ps(childrenCenterY, genData.projData->cameraPosY_SOA);
		bareVec8f y2 = _mm256_mul_ps(temp8, temp8);
		temp8 = _mm256_sub_ps(childrenCenterZ, genData.projData->cameraPosZ_SOA);
		bareVec8f z2 = _mm256_mul_ps(temp8, temp8);
		bareVec8f distancesV = _mm256_add_ps(x2, _mm256_add_ps(y2, z2));
		//get octants in increasing distance sorted order
		for (int i = 0; i < 8; i++)
		{
			distances[i] = distancesV.get(i);
			sortedIndices[i] = i;
		}

		oddEvenMergeSort_8(distances, sortedIndices);

		for (int i = 0; i < 8; i++)
		{

			int octant = sortedIndices[i];
			if (node.children[octant] != NULL)
			{
				//if (!(octant == 2 || octant ==3) ) continue; //test
				childBoxs[octant].worldMin.set(0, childrenWorldMinX.get(octant));
				childBoxs[octant].worldMin.set(1, childrenWorldMinY.get(octant));
				childBoxs[octant].worldMin.set(2, childrenWorldMinZ.get(octant));
				childBoxs[octant].worldMin.set(3, 1.0f);				
				childBoxs[octant].worldMax.set(0, childrenWorldMaxX.get(octant));
				childBoxs[octant].worldMax.set(1, childrenWorldMaxY.get(octant));
				childBoxs[octant].worldMax.set(2, childrenWorldMaxZ.get(octant));
				childBoxs[octant].worldMax.set(3, 1.0f);
//				childBoxs[octant].clipSpaceMin.set(0, childrenClipSpaceMinX.get(octant));
//				childBoxs[octant].clipSpaceMin.set(1, childrenClipSpaceMinY.get(octant));
//				childBoxs[octant].clipSpaceMin.set(2, childrenClipSpaceMinZ.get(octant));
//				childBoxs[octant].clipSpaceMin.set(3, childrenClipSpaceMinW.get(octant));
				if(curRecurseData.threeFaceOptimization)
				{
					childBoxs[octant].posOrNegFaceSeenBits = renderBox.posOrNegFaceSeenBits;	
					childBoxs[octant].isFaceSeenOnAxisBits = renderBox.isFaceSeenOnAxisBits;	
				}
				FindVisibleVoxelsRecursive(genData, curRecurseData, *node.children[octant], childBoxs[octant]);
			}
		}
	}

	inline static bool isBoxOutsideFrustum(GenData &genData, RecurseData &recurseData, bool possibleScreenIntersection, bareVec8f &worldCornersX, bareVec8f &worldCornersY, bareVec8f &worldCornersZ)
	{
		for (int a = 0; a < 29; a++) //0-6 are axii derived from near face, 28th being the near face normal
		{
			bool doFrustumCulling = (recurseData.seperatingAxiiCheckMask >> a) & 1;
			bool doScreenIntersectionTest = possibleScreenIntersection && ((recurseData.sepAxiiCheckMaskNearFace >> a) & 1) && (a < 7);
			if (doFrustumCulling || (doScreenIntersectionTest)) //1s in check mask mean to check the axxi
			{
				bareVec8f partialDotX, partialDotY, partialDotZ; float boxMin, boxMax;
				partialDotX = _mm256_mul_ps(_mm256_set1_ps(genData.cullFrustum->sepAxiiX[a]), worldCornersX);
				partialDotY = _mm256_mul_ps(_mm256_set1_ps(genData.cullFrustum->sepAxiiY[a]), worldCornersY);
				partialDotZ = _mm256_mul_ps(_mm256_set1_ps(genData.cullFrustum->sepAxiiZ[a]), worldCornersZ);
				bareVec8f dot = _mm256_add_ps(_mm256_add_ps(partialDotZ, partialDotY), partialDotX);
				bareVec8f temp = _mm256_permute_ps(dot, 0b10001110);
				bareVec8f temp2 = _mm256_max_ps(dot, temp);
				temp = _mm256_permute_ps(temp2, 0b01);
				temp2 = _mm256_max_ps(temp2, temp);
				if(a == 2)
				{
					boxMax = 0;
				}
				boxMax = temp2[0] > temp2[4] ? temp2[0] : temp2[4];
				temp = _mm256_permute_ps(dot, 0b10001110);
				temp2 = _mm256_min_ps(dot, temp);
				temp = _mm256_permute_ps(temp2, 0b01);
				temp2 = _mm256_min_ps(temp2, temp);
				boxMin = temp2[0] < temp2[4] ? temp2[0] : temp2[4];
				if (doFrustumCulling)
				{
					if (genData.cullFrustum->frustumOnAxiiMin[a] < boxMin)
					{
						if (genData.cullFrustum->frustumOnAxiiMax[a] > boxMax)
						{
							recurseData.seperatingAxiiCheckMask &= ~(1 << a); //toggle checking the plane off if the box fully enclosed cause the children ain't gonna escape
						}
						else if (boxMin >= genData.cullFrustum->frustumOnAxiiMax[a]) //outside on positive side of frustum projection
						{
							return true;
						}
					}
					else if (boxMax <= genData.cullFrustum->frustumOnAxiiMin[a]) //outside on neg side
					{
						return true;
					}
				}
				if (doScreenIntersectionTest)
				{
					if (genData.cullFrustum->nearFaceOnAxiiMin[a] < boxMin)
					{
						if (genData.cullFrustum->nearFaceOnAxiiMax[a] > boxMax)
						{
							recurseData.sepAxiiCheckMaskNearFace &= ~(1 << a); //toggle checking the plane off if the box fully enclosed cause the children ain't gonna escape
						}
						else if (boxMin >= genData.cullFrustum->nearFaceOnAxiiMax[a]) //outside on positive side of frustum projection
						{
							recurseData.intersectingScreen = 0;

						}
					}
					else if (boxMax <= genData.cullFrustum->nearFaceOnAxiiMin[a]) //outside on neg side
					{
						recurseData.intersectingScreen = 0;
					}
				}

			}
		}
		return false;
	}
	inline static void CMP_SWAP(int i, int j, float* distances, int* indices) {
		if (distances[i] > distances[j])
		{
			float tmpf = distances[i]; distances[i] = distances[j]; distances[j] = tmpf;
			int tmp = indices[i]; indices[i] = indices[j]; indices[j] = tmp;
		}
	}

	inline static void oddEvenMergeSort_8(float* distances, int* indices) //should be increasing order
	{
		CMP_SWAP(0, 1, distances, indices); CMP_SWAP(2, 3, distances, indices); CMP_SWAP(4, 5, distances, indices); CMP_SWAP(6, 7, distances, indices);
		CMP_SWAP(0, 2, distances, indices); CMP_SWAP(1, 3, distances, indices); CMP_SWAP(4, 6, distances, indices); CMP_SWAP(5, 7, distances, indices);
		CMP_SWAP(1, 2, distances, indices); CMP_SWAP(5, 6, distances, indices); CMP_SWAP(0, 4, distances, indices); CMP_SWAP(1, 5, distances, indices);
		CMP_SWAP(2, 6, distances, indices); CMP_SWAP(3, 7, distances, indices); CMP_SWAP(2, 4, distances, indices); CMP_SWAP(3, 5, distances, indices);
		CMP_SWAP(1, 2, distances, indices); CMP_SWAP(3, 4, distances, indices); CMP_SWAP(5, 6, distances, indices);
	}


};

