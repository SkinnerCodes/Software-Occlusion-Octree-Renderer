#pragma once
template<unsigned int NUM_OF_LINES>
inline void writeMask_stamp4(bareVec4f distanceFromLine[], int64_t& output, int& outputOffset) { //assuming barevec.x is 0-31 in the register
	__m128 result = distanceFromLine[0];
	for (int i = 1; i < NUM_OF_LINES; i++)
	{
		result = _mm_and_ps(result, distanceFromLine[i]);
	}
	int64_t mask = _mm_movemask_ps(result);
	output |= mask << outputOffset;
}
template<unsigned int NUM_OF_LINES>
inline void RasterBox_stamp4(bareVec4f distFromLine[], bareVec4f line2A[], bareVec4f line2B[], int64_t *outputBlock, bareVec4i &minmax) {
	//assumptions made, rasterization starts aligned with block, there min.x() is aligned to a byte, outputOffset = 0 initially, this allows simple block picking, pointer outputBlock +=1
	//all arguments are correct for first stamp
	int columns = minmax.get(2) - minmax.get(0);//in column units to fill, including end and start
	int stampsForRow = (columns >> 1) + (columns & 1); //if it's odd we add 1 so it stamps everything
	int rows = minmax.get(3) - minmax.get(1);//max.y()-min.y();
	int stampsForColumnRemaining = (rows >> 1) + (rows & 1);
	int outputOffset = 0;
	int64_t* firstBlock = outputBlock;
	bareVec4f firstInRow[NUM_OF_LINES];
	for (int i = 0; i < NUM_OF_LINES; i++) { firstInRow[i] = distFromLine[i]; }

	stampsForColumnRemaining--;
	goto skipFirst;

	while (stampsForColumnRemaining > 0)
	{
		stampsForColumnRemaining--;
		firstBlock += (BlocksInRow);
		outputBlock = firstBlock;
		for (int i = 0; i < NUM_OF_LINES; i++) { distFromLine[i] = firstInRow[i]; }
		for (int i = 0; i < NUM_OF_LINES; i++) { distFromLine[i] = _mm_add_ps(distFromLine[i], line2B[i]); }
		for (int i = 0; i < NUM_OF_LINES; i++) { firstInRow[i] = distFromLine[i]; }
		outputOffset = 0;

		//-4995072469926809587
		//-4995072469926809587
		//-4995072469926809585
		//
	skipFirst:

		int stampsForRowRemaining = stampsForRow;
		while (stampsForRowRemaining > 16) {
			for (int i = 0; i < 16; i++) { //only 15 because the first of the block is written in the line above
				writeMask_stamp4<NUM_OF_LINES>(distFromLine, *outputBlock, outputOffset);
				outputOffset += 4;
				for (int i = 0; i < NUM_OF_LINES; i++) { distFromLine[i] = _mm_add_ps(distFromLine[i], line2A[i]); }
			}
			stampsForRowRemaining -= 16;
			outputBlock += 1;
			outputOffset = 0;
		}
		switch (stampsForRowRemaining) {
		case 16:
			writeMask_stamp4<NUM_OF_LINES>(distFromLine, *outputBlock, outputOffset);
			for (int i = 0; i < NUM_OF_LINES; i++) { distFromLine[i] = _mm_add_ps(distFromLine[i], line2A[i]); }
			outputOffset += 4;
		case 15:
			writeMask_stamp4<NUM_OF_LINES>(distFromLine, *outputBlock, outputOffset);
			for (int i = 0; i < NUM_OF_LINES; i++) { distFromLine[i] = _mm_add_ps(distFromLine[i], line2A[i]); }
			outputOffset += 4;
		case 14:
			writeMask_stamp4<NUM_OF_LINES>(distFromLine, *outputBlock, outputOffset);
			for (int i = 0; i < NUM_OF_LINES; i++) { distFromLine[i] = _mm_add_ps(distFromLine[i], line2A[i]); }
			outputOffset += 4;
		case 13:
			writeMask_stamp4<NUM_OF_LINES>(distFromLine, *outputBlock, outputOffset);
			for (int i = 0; i < NUM_OF_LINES; i++) { distFromLine[i] = _mm_add_ps(distFromLine[i], line2A[i]); }
			outputOffset += 4;
		case 12:
			writeMask_stamp4<NUM_OF_LINES>(distFromLine, *outputBlock, outputOffset);
			for (int i = 0; i < NUM_OF_LINES; i++) { distFromLine[i] = _mm_add_ps(distFromLine[i], line2A[i]); }
			outputOffset += 4;
		case 11:
			writeMask_stamp4<NUM_OF_LINES>(distFromLine, *outputBlock, outputOffset);
			for (int i = 0; i < NUM_OF_LINES; i++) { distFromLine[i] = _mm_add_ps(distFromLine[i], line2A[i]); }
			outputOffset += 4;
		case 10:
			writeMask_stamp4<NUM_OF_LINES>(distFromLine, *outputBlock, outputOffset);
			for (int i = 0; i < NUM_OF_LINES; i++) { distFromLine[i] = _mm_add_ps(distFromLine[i], line2A[i]); }
			outputOffset += 4;
		case 9:
			writeMask_stamp4<NUM_OF_LINES>(distFromLine, *outputBlock, outputOffset);
			for (int i = 0; i < NUM_OF_LINES; i++) { distFromLine[i] = _mm_add_ps(distFromLine[i], line2A[i]); }
			outputOffset += 4;
		case 8:
			writeMask_stamp4<NUM_OF_LINES>(distFromLine, *outputBlock, outputOffset);
			for (int i = 0; i < NUM_OF_LINES; i++) { distFromLine[i] = _mm_add_ps(distFromLine[i], line2A[i]); }
			outputOffset += 4;
		case 7:
			writeMask_stamp4<NUM_OF_LINES>(distFromLine, *outputBlock, outputOffset);
			for (int i = 0; i < NUM_OF_LINES; i++) { distFromLine[i] = _mm_add_ps(distFromLine[i], line2A[i]); }
			outputOffset += 4;
		case 6:
			writeMask_stamp4<NUM_OF_LINES>(distFromLine, *outputBlock, outputOffset);
			for (int i = 0; i < NUM_OF_LINES; i++) { distFromLine[i] = _mm_add_ps(distFromLine[i], line2A[i]); }
			outputOffset += 4;
		case 5:
			writeMask_stamp4<NUM_OF_LINES>(distFromLine, *outputBlock, outputOffset);
			for (int i = 0; i < NUM_OF_LINES; i++) { distFromLine[i] = _mm_add_ps(distFromLine[i], line2A[i]); }
			outputOffset += 4;
		case 4:
			writeMask_stamp4<NUM_OF_LINES>(distFromLine, *outputBlock, outputOffset);
			for (int i = 0; i < NUM_OF_LINES; i++) { distFromLine[i] = _mm_add_ps(distFromLine[i], line2A[i]); }
			outputOffset += 4;
		case 3:
			writeMask_stamp4<NUM_OF_LINES>(distFromLine, *outputBlock, outputOffset);
			for (int i = 0; i < NUM_OF_LINES; i++) { distFromLine[i] = _mm_add_ps(distFromLine[i], line2A[i]); }
			outputOffset += 4;
		case 2:
			writeMask_stamp4<NUM_OF_LINES>(distFromLine, *outputBlock, outputOffset);
			for (int i = 0; i < NUM_OF_LINES; i++) { distFromLine[i] = _mm_add_ps(distFromLine[i], line2A[i]); }
			outputOffset += 4;
		case 1:
			writeMask_stamp4<NUM_OF_LINES>(distFromLine, *outputBlock, outputOffset);
			for (int i = 0; i < NUM_OF_LINES; i++) { distFromLine[i] = _mm_add_ps(distFromLine[i], line2A[i]); }
			outputOffset += 4;
		}

	}

}
template<unsigned int NUM_OF_LINES>
inline void DrawQuad_stamp4(BorderEightLines &border) {
	bareVec4i minmax = _mm_cvtps_epi32(_mm_add_ps(border.minMaxBounds, _mm_set1_ps(0.5f)));//minmax should be stored minx, miny, maxx,maxy // consequently max is an open bound
	minmax = _mm_and_si128(minmax, _mm_set_epi32(~0, ~0, ~1, ~3)); //this does the following//get nearest even, stamp is now aligned for algorithm// get nearest multiple of 4 that is below number
																   //now do clipping on min / max, based on the screen region
	minmax = _mm_max_epi32(minmax, _mm_set_epi32(ScreenRegionStartY, ScreenRegionStartX, ScreenRegionStartY, ScreenRegionStartX));
	minmax = _mm_min_epi32(minmax, _mm_set_epi32(ScreenRegionStartY + ScreenRegionHeight, ScreenRegionStartX + ScreenRegionWidth, ScreenRegionStartY + ScreenRegionHeight, ScreenRegionStartX + ScreenRegionWidth));
	//get the window space coordinates for the first pixel to be checked for raster / bounds inclusion
	//actually, the first 4 pixels, the locations of the first "stamp", in order bottom left, top left, bottom right, top right
	bareVec4f pixCoordX = _mm_add_ps(_mm_cvtepi32_ps(_mm_shuffle_epi32(minmax, 0b0)), _mm_set_ps(1.5f, 1.5f, 0.5f, 0.5f)); //pixCoordX.x = min.x() +0.5f; pixCoordX.y = min.x()+0.5f; pixCoordX.z = min.x()+1.5f; pixCoordX.w = min.x()+1.5f;
	bareVec4f pixCoordY = _mm_add_ps(_mm_cvtepi32_ps(_mm_shuffle_epi32(minmax, 0b01010101)), _mm_set_ps(1.5f, 0.5f, 1.5f, 0.5f)); //pixCoordY.x = min.y()+0.5f; pixCoordY.y = min.y() + 1.5f; pixCoordY.z = min.y()+0.5f; pixCoordY.w = min.y()+1.5f;
	bareVec4f lineA[NUM_OF_LINES], lineB[NUM_OF_LINES], lineC[NUM_OF_LINES], line2A[NUM_OF_LINES], line2B[NUM_OF_LINES];
	__m128 two = _mm_set1_ps(2);
	bareVec4f distFromLine[NUM_OF_LINES];
	for (int i = 0; i < NUM_OF_LINES; i++) {

		lineB[i] = _mm_set1_ps(border.B.get(i));
		lineC[i] = _mm_set1_ps(border.C.get(i));
		lineA[i] = _mm_set1_ps(border.A.get(i));
		distFromLine[i].m128 = getDistances(lineA[i], lineB[i], lineC[i], pixCoordX, pixCoordY); //remember, after the these first determinations are made for the first pixel, the other distances are made just by adding constants

		line2A[i] = _mm_mul_ps(lineA[i], two);
		line2B[i] = _mm_mul_ps(lineB[i], two);
	}
	//convert the screen box to a box that is relative to the buffer origin located at the screen regions min corner
	minmax = _mm_sub_epi32(minmax, _mm_set_epi32(ScreenRegionStartY, ScreenRegionStartX, ScreenRegionStartY, ScreenRegionStartX));
	int byteIndex = (minmax.get(0) / 4) + (minmax.get(1) / 2)*(BlocksInRow * 8); // these divisions should be optimized to shifts by compiler
	int64_t* outputBlock = (int64_t*)(MaskBuffer.get() + byteIndex);
	return RasterBox_stamp4<NUM_OF_LINES>(distFromLine, line2A, line2B, outputBlock, minmax);
}
template<unsigned int NUM_OF_LINES>
inline void DrawQuad_stamp4(QuadBorder &border) {
	bareVec4i minmax = _mm_cvtps_epi32(_mm_add_ps(border.minMaxBounds, _mm_set1_ps(0.5f)));//minmax should be stored minx, miny, maxx,maxy // consequently max is an open bound
	minmax = _mm_and_si128(minmax, _mm_set_epi32(~0, ~0, ~1, ~3)); //this does the following//get nearest even, stamp is now aligned for algorithm// get nearest multiple of 4 that is below number
																   //now do clipping on min / max
	minmax = _mm_max_epi32(minmax, _mm_set_epi32(ScreenRegionStartY, ScreenRegionStartX, ScreenRegionStartY, ScreenRegionStartX));
	minmax = _mm_min_epi32(minmax, _mm_set_epi32(ScreenRegionStartY + ScreenRegionHeight, ScreenRegionStartX + ScreenRegionWidth, ScreenRegionStartY + ScreenRegionHeight, ScreenRegionStartX + ScreenRegionWidth));
	//get the window space coordinates for the first pixel to be checked for raster / bounds inclusion
	//actually, the first 4 pixels, the locations of the first "stamp", in order bottom left, top left, bottom right, top right
	bareVec4f pixCoordX = _mm_add_ps(_mm_cvtepi32_ps(_mm_shuffle_epi32(minmax, 0b0)), _mm_set_ps(1.5f, 1.5f, 0.5f, 0.5f)); //pixCoordX.x = min.x() +0.5f; pixCoordX.y = min.x()+0.5f; pixCoordX.z = min.x()+1.5f; pixCoordX.w = min.x()+1.5f;
	bareVec4f pixCoordY = _mm_add_ps(_mm_cvtepi32_ps(_mm_shuffle_epi32(minmax, 0b01010101)), _mm_set_ps(1.5f, 0.5f, 1.5f, 0.5f)); //pixCoordY.x = min.y()+0.5f; pixCoordY.y = min.y() + 1.5f; pixCoordY.z = min.y()+0.5f; pixCoordY.w = min.y()+1.5f;
	bareVec4f lineA[NUM_OF_LINES], lineB[NUM_OF_LINES], lineC[NUM_OF_LINES], line2A[NUM_OF_LINES], line2B[NUM_OF_LINES];
	__m128 two = _mm_set1_ps(2);
	bareVec4f distFromLine[NUM_OF_LINES];
	for (int i = 0; i < NUM_OF_LINES; i++) {

		lineB[i] = _mm_set1_ps(border.B.get(i));
		lineC[i] = _mm_set1_ps(border.C.get(i));
		lineA[i] = _mm_set1_ps(border.A.get(i));
		distFromLine[i].m128 = getDistances(lineA[i], lineB[i], lineC[i], pixCoordX, pixCoordY); //remember, after the these first determinations are made for the first pixel, the other distances are made just by adding constants

		line2A[i] = _mm_mul_ps(lineA[i], two);
		line2B[i] = _mm_mul_ps(lineB[i], two);
	}
	//convert the screen box to a box that is relative to the buffer origin located at the screen regions min corner
	minmax = _mm_sub_epi32(minmax, _mm_set_epi32(ScreenRegionStartY, ScreenRegionStartX, ScreenRegionStartY, ScreenRegionStartX));
	int byteIndex = (minmax.get(0) / 4) + (minmax.get(1) / 2)*(BlocksInRow * 8); // these divisions should be optimized to shifts by compiler
	int64_t* outputBlock = (int64_t*)(MaskBuffer.get() + byteIndex);
	return RasterBox_stamp4<NUM_OF_LINES>(distFromLine, line2A, line2B, outputBlock, minmax);
}

template<unsigned int NUM_OF_LINES>
inline int isQuadFullyOccludedRaster_stamp4(bareVec4f distFromLine[], bareVec4f line2A[], bareVec4f line2B[], int64_t *currentBlock, bareVec4i &minmax) {
	//assumptions made, rasterization starts aligned with block, there min.x() is aligned to a byte, outputOffset = 0 initially, this allows simple block picking, pointer outputBlock +=1
	//all arguments are correct for first stamp
	int columns = minmax.get(2) - minmax.get(0);//in column units to fill, including end and start
	int stampsForRow = (columns >> 1) + (columns & 1); //if it's odd we add 1 so it stamps everything
	int rows = minmax.get(3) - minmax.get(1);//max.y()-min.y();
	int stampsForColumnRemaining = (rows >> 1) + (rows & 1);
	int outputOffset = 0;
	bool modifiedBits = false;
	int64_t* firstBlock = currentBlock;
	int64_t unmodifiedBlock = *firstBlock;
	int64_t writeBlock = *firstBlock;
	bareVec4f firstInRow[NUM_OF_LINES];
	for (int i = 0; i < NUM_OF_LINES; i++) { firstInRow[i] = distFromLine[i]; }
	stampsForColumnRemaining--;

	goto skipFirst;
	while (stampsForColumnRemaining > 0 && !modifiedBits)
	{
		stampsForColumnRemaining--;
		firstBlock += (BlocksInRow);
		currentBlock = firstBlock;
		unmodifiedBlock = *currentBlock;
		writeBlock = *currentBlock;
		for (int i = 0; i < NUM_OF_LINES; i++) { distFromLine[i] = firstInRow[i]; }
		for (int i = 0; i < NUM_OF_LINES; i++) { distFromLine[i] = _mm_add_ps(distFromLine[i], line2B[i]); }
		for (int i = 0; i < NUM_OF_LINES; i++) { firstInRow[i] = distFromLine[i]; }
		outputOffset = 0;

	skipFirst:

		int stampsForRowRemaining = stampsForRow;
		while (stampsForRowRemaining > 16 && !modifiedBits) {
			for (int i = 0; i < 16; i++) {
				writeMask_stamp4<NUM_OF_LINES>(distFromLine, writeBlock, outputOffset);
				outputOffset += 4;
				for (int i = 0; i < NUM_OF_LINES; i++) { distFromLine[i] = _mm_add_ps(distFromLine[i], line2A[i]); }
			}
			//if (writeBlock ^ unmodifiedBlock) return 0; //if it adds coverage, it's not fully occuluded, return false
			modifiedBits |= (writeBlock ^ unmodifiedBlock);

			stampsForRowRemaining -= 16;
			currentBlock += 1;
			unmodifiedBlock = *currentBlock;
			writeBlock = *currentBlock;
			outputOffset = 0;
		}
		switch (stampsForRowRemaining & ((unmodifiedBlock == ~0) - 1)) {
		case 16:
			writeMask_stamp4<NUM_OF_LINES>(distFromLine, writeBlock, outputOffset);
			for (int i = 0; i < NUM_OF_LINES; i++) { distFromLine[i] = _mm_add_ps(distFromLine[i], line2A[i]); }
			outputOffset += 4;
		case 15:
			writeMask_stamp4<NUM_OF_LINES>(distFromLine, writeBlock, outputOffset);
			for (int i = 0; i < NUM_OF_LINES; i++) { distFromLine[i] = _mm_add_ps(distFromLine[i], line2A[i]); }
			outputOffset += 4;
		case 14:
			writeMask_stamp4<NUM_OF_LINES>(distFromLine, writeBlock, outputOffset);
			for (int i = 0; i < NUM_OF_LINES; i++) { distFromLine[i] = _mm_add_ps(distFromLine[i], line2A[i]); }
			outputOffset += 4;
		case 13:
			writeMask_stamp4<NUM_OF_LINES>(distFromLine, writeBlock, outputOffset);
			for (int i = 0; i < NUM_OF_LINES; i++) { distFromLine[i] = _mm_add_ps(distFromLine[i], line2A[i]); }
			outputOffset += 4;
		case 12:
			writeMask_stamp4<NUM_OF_LINES>(distFromLine, *currentBlock, outputOffset);
			for (int i = 0; i < NUM_OF_LINES; i++) { distFromLine[i] = _mm_add_ps(distFromLine[i], line2A[i]); }
			outputOffset += 4;
		case 11:
			writeMask_stamp4<NUM_OF_LINES>(distFromLine, writeBlock, outputOffset);
			for (int i = 0; i < NUM_OF_LINES; i++) { distFromLine[i] = _mm_add_ps(distFromLine[i], line2A[i]); }
			outputOffset += 4;
		case 10:
			writeMask_stamp4<NUM_OF_LINES>(distFromLine, writeBlock, outputOffset);
			for (int i = 0; i < NUM_OF_LINES; i++) { distFromLine[i] = _mm_add_ps(distFromLine[i], line2A[i]); }
			outputOffset += 4;
		case 9:
			writeMask_stamp4<NUM_OF_LINES>(distFromLine, writeBlock, outputOffset);
			for (int i = 0; i < NUM_OF_LINES; i++) { distFromLine[i] = _mm_add_ps(distFromLine[i], line2A[i]); }
			outputOffset += 4;
		case 8:
			writeMask_stamp4<NUM_OF_LINES>(distFromLine, writeBlock, outputOffset);
			for (int i = 0; i < NUM_OF_LINES; i++) { distFromLine[i] = _mm_add_ps(distFromLine[i], line2A[i]); }
			outputOffset += 4;
		case 7:
			writeMask_stamp4<NUM_OF_LINES>(distFromLine, writeBlock, outputOffset);
			for (int i = 0; i < NUM_OF_LINES; i++) { distFromLine[i] = _mm_add_ps(distFromLine[i], line2A[i]); }
			outputOffset += 4;
		case 6:
			writeMask_stamp4<NUM_OF_LINES>(distFromLine, writeBlock, outputOffset);
			for (int i = 0; i < NUM_OF_LINES; i++) { distFromLine[i] = _mm_add_ps(distFromLine[i], line2A[i]); }
			outputOffset += 4;
		case 5:
			writeMask_stamp4<NUM_OF_LINES>(distFromLine, writeBlock, outputOffset);
			for (int i = 0; i < NUM_OF_LINES; i++) { distFromLine[i] = _mm_add_ps(distFromLine[i], line2A[i]); }
			outputOffset += 4;
		case 4:
			writeMask_stamp4<NUM_OF_LINES>(distFromLine, writeBlock, outputOffset);
			for (int i = 0; i < NUM_OF_LINES; i++) { distFromLine[i] = _mm_add_ps(distFromLine[i], line2A[i]); }
			outputOffset += 4;
		case 3:
			writeMask_stamp4<NUM_OF_LINES>(distFromLine, writeBlock, outputOffset);
			for (int i = 0; i < NUM_OF_LINES; i++) { distFromLine[i] = _mm_add_ps(distFromLine[i], line2A[i]); }
			outputOffset += 4;
		case 2:
			writeMask_stamp4<NUM_OF_LINES>(distFromLine, writeBlock, outputOffset);
			for (int i = 0; i < NUM_OF_LINES; i++) { distFromLine[i] = _mm_add_ps(distFromLine[i], line2A[i]); }
			outputOffset += 4;
		case 1:
			writeMask_stamp4<NUM_OF_LINES>(distFromLine, writeBlock, outputOffset);
			for (int i = 0; i < NUM_OF_LINES; i++) { distFromLine[i] = _mm_add_ps(distFromLine[i], line2A[i]); }
			outputOffset += 4;
		}
		modifiedBits |= (writeBlock ^ unmodifiedBlock);
	}
	return !modifiedBits;
}

template<unsigned int NUM_OF_LINES>
inline int isQuadFullyOccluded_stamp4(BorderEightLines &border) {
	bareVec4i minmax = _mm_cvtps_epi32(_mm_add_ps(border.minMaxBounds, _mm_set1_ps(0.5f)));//minmax should be stored minx, miny, maxx,maxy // consequently max is an open bound
	minmax = _mm_and_si128(minmax, _mm_set_epi32(~0, ~0, ~1, ~3)); //this does the following//get nearest even, stamp is now aligned for algorithm// get nearest multiple of 4 that is below number
																   //now do clipping on min / max
	minmax = _mm_max_epi32(minmax, _mm_set_epi32(ScreenRegionStartY, ScreenRegionStartX, ScreenRegionStartY, ScreenRegionStartX));
	minmax = _mm_min_epi32(minmax, _mm_set_epi32(ScreenRegionStartY + ScreenRegionHeight, ScreenRegionStartX + ScreenRegionWidth, ScreenRegionStartY + ScreenRegionHeight, ScreenRegionStartX + ScreenRegionWidth));
	//get the window space coordinates for the first pixel to be checked for raster / bounds inclusion
	//actually, the first 4 pixels, the locations of the first "stamp", in order bottom left, top left, bottom right, top right
	bareVec4f pixCoordX = _mm_add_ps(_mm_cvtepi32_ps(_mm_shuffle_epi32(minmax, 0b0)), _mm_set_ps(1.5f, 1.5f, 0.5f, 0.5f)); //pixCoordX.x = min.x() +0.5f; pixCoordX.y = min.x()+0.5f; pixCoordX.z = min.x()+1.5f; pixCoordX.w = min.x()+1.5f;
	bareVec4f pixCoordY = _mm_add_ps(_mm_cvtepi32_ps(_mm_shuffle_epi32(minmax, 0b01010101)), _mm_set_ps(1.5f, 0.5f, 1.5f, 0.5f)); //pixCoordY.x = min.y()+0.5f; pixCoordY.y = min.y() + 1.5f; pixCoordY.z = min.y()+0.5f; pixCoordY.w = min.y()+1.5f;
	bareVec4f lineA[NUM_OF_LINES], lineB[NUM_OF_LINES], lineC[NUM_OF_LINES], line2A[NUM_OF_LINES], line2B[NUM_OF_LINES];
	__m128 two = _mm_set1_ps(2);
	bareVec4f distFromLine[NUM_OF_LINES];
	for (int i = 0; i < NUM_OF_LINES; i++) {

		lineB[i] = _mm_set1_ps(border.B.get(i));
		lineC[i] = _mm_set1_ps(border.C.get(i));
		lineA[i] = _mm_set1_ps(border.A.get(i));
		distFromLine[i].m128 = getDistances(lineA[i], lineB[i], lineC[i], pixCoordX, pixCoordY); //remember, after the these first determinations are made for the first pixel, the other distances are made just by adding constants

		line2A[i] = _mm_mul_ps(lineA[i], two);
		line2B[i] = _mm_mul_ps(lineB[i], two);
	}
	//convert the screen box to a box that is relative to the buffer origin located at the screen regions min corner
	minmax = _mm_sub_epi32(minmax, _mm_set_epi32(ScreenRegionStartY, ScreenRegionStartX, ScreenRegionStartY, ScreenRegionStartX));
	int byteIndex = (minmax.get(0) / 4) + (minmax.get(1) / 2)*(BlocksInRow * 8); // these divisions should be optimized to shifts by compiler
	int64_t* outputBlock = (int64_t*)(MaskBuffer.get() + byteIndex);
	return isQuadFullyOccludedRaster_stamp4<NUM_OF_LINES>(distFromLine, line2A, line2B, outputBlock, minmax);
}
template<unsigned int NUM_OF_LINES>
inline int isQuadFullyOccluded_stamp4(QuadBorder &border) {
	bareVec4i minmax = _mm_cvtps_epi32(_mm_add_ps(border.minMaxBounds, _mm_set1_ps(0.5f)));//minmax should be stored minx, miny, maxx,maxy // consequently max is an open bound
	minmax = _mm_and_si128(minmax, _mm_set_epi32(~0, ~0, ~1, ~3)); //this does the following//get nearest even, stamp is now aligned for algorithm// get nearest multiple of 4 that is below number
																   //now do clipping on min / max
	minmax = _mm_max_epi32(minmax, _mm_set_epi32(ScreenRegionStartY, ScreenRegionStartX, ScreenRegionStartY, ScreenRegionStartX));
	minmax = _mm_min_epi32(minmax, _mm_set_epi32(ScreenRegionStartY + ScreenRegionHeight, ScreenRegionStartX + ScreenRegionWidth, ScreenRegionStartY + ScreenRegionHeight, ScreenRegionStartX + ScreenRegionWidth));
	//get the window space coordinates for the first pixel to be checked for raster / bounds inclusion
	//actually, the first 4 pixels, the locations of the first "stamp", in order bottom left, top left, bottom right, top right
	bareVec4f pixCoordX = _mm_add_ps(_mm_cvtepi32_ps(_mm_shuffle_epi32(minmax, 0b0)), _mm_set_ps(1.5f, 1.5f, 0.5f, 0.5f)); //pixCoordX.x = min.x() +0.5f; pixCoordX.y = min.x()+0.5f; pixCoordX.z = min.x()+1.5f; pixCoordX.w = min.x()+1.5f;
	bareVec4f pixCoordY = _mm_add_ps(_mm_cvtepi32_ps(_mm_shuffle_epi32(minmax, 0b01010101)), _mm_set_ps(1.5f, 0.5f, 1.5f, 0.5f)); //pixCoordY.x = min.y()+0.5f; pixCoordY.y = min.y() + 1.5f; pixCoordY.z = min.y()+0.5f; pixCoordY.w = min.y()+1.5f;
	bareVec4f lineA[NUM_OF_LINES], lineB[NUM_OF_LINES], lineC[NUM_OF_LINES], line2A[NUM_OF_LINES], line2B[NUM_OF_LINES];
	__m128 two = _mm_set1_ps(2);
	bareVec4f distFromLine[NUM_OF_LINES];
	for (int i = 0; i < NUM_OF_LINES; i++) {

		lineB[i] = _mm_set1_ps(border.B.get(i));
		lineC[i] = _mm_set1_ps(border.C.get(i));
		lineA[i] = _mm_set1_ps(border.A.get(i));
		distFromLine[i].m128 = getDistances(lineA[i], lineB[i], lineC[i], pixCoordX, pixCoordY); //remember, after the these first determinations are made for the first pixel, the other distances are made just by adding constants

		line2A[i] = _mm_mul_ps(lineA[i], two);
		line2B[i] = _mm_mul_ps(lineB[i], two);
	}
	//convert the screen box to a box that is relative to the buffer origin located at the screen regions min corner
	minmax = _mm_sub_epi32(minmax, _mm_set_epi32(ScreenRegionStartY, ScreenRegionStartX, ScreenRegionStartY, ScreenRegionStartX));
	int byteIndex = (minmax.get(0) / 4) + (minmax.get(1) / 2)*(BlocksInRow * 8); // these divisions should be optimized to shifts by compiler
	int64_t* outputBlock = (int64_t*)(MaskBuffer.get() + byteIndex);
	return isQuadFullyOccludedRaster_stamp4<NUM_OF_LINES>(distFromLine, line2A, line2B, outputBlock, minmax);
}