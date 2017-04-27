#pragma once

template<unsigned int NUM_OF_LINES>
inline void RasterBox_stamp8(bareVec8f distFromLine[], bareVec8f line4A[], bareVec8f line2B[], int64_t *currentBlock, bareVec4i &minmax) {
	//assumptions made, rasterization starts aligned with block, the min.x() is aligned to a byte, outputOffset = 0 initially, this allows simple block picking, pointer outputBlock +=1
	//all arguments are correct for first stamp
	int columns = minmax.get(2) - minmax.get(0);//in column units to fill, since max is open-bound, the difference amount will include start pixel and end pixel
	int stampsForRow = (columns >> 2) + ((columns & 3) != 0); //if there is a remainder of division by 4, we add 1 more stamp to makre sure it stamps everything
	int rows = minmax.get(3) - minmax.get(1);
	int stampsForColumnRemaining = (rows >> 1) + (rows & 1);
	int outputOffset = 0;
	int64_t* firstBlock = currentBlock;
	bareVec8f firstDistancesFromLines[NUM_OF_LINES]; //each vector in array contains distances for each line
	for (int i = 0; i < NUM_OF_LINES; i++) { firstDistancesFromLines[i] = distFromLine[i]; }

	stampsForColumnRemaining--;
	goto skipFirst;

	while (stampsForColumnRemaining > 0)
	{
		stampsForColumnRemaining--;
		firstBlock += (BlocksInRow);
		currentBlock = firstBlock;
		for (int i = 0; i < NUM_OF_LINES; i++) { distFromLine[i] = firstDistancesFromLines[i]; }
		for (int i = 0; i < NUM_OF_LINES; i++) { distFromLine[i] = _mm256_add_ps(distFromLine[i], line2B[i]); }
		for (int i = 0; i < NUM_OF_LINES; i++) { firstDistancesFromLines[i] = distFromLine[i]; }
		outputOffset = 0;

	skipFirst:

		int stampsForRowRemaining = stampsForRow;
		while (stampsForRowRemaining > 8) {
			for (int i = 0; i < 8; i++) {
				writeMask_stamp8<NUM_OF_LINES>(distFromLine, *currentBlock, outputOffset);
				outputOffset += 8;
				for (int i = 0; i < NUM_OF_LINES; i++)
				{
					distFromLine[i] = _mm256_add_ps(distFromLine[i], line4A[i]);
				}
			}
			outputOffset = 0;
			stampsForRowRemaining -= 8;
			currentBlock += 1;
		}
		switch (stampsForRowRemaining) {
		case 8:
			writeMask_stamp8<NUM_OF_LINES>(distFromLine, *currentBlock, outputOffset);
			for (int i = 0; i < NUM_OF_LINES; i++) { distFromLine[i] = _mm256_add_ps(distFromLine[i], line4A[i]); }
			outputOffset += 8;
		case 7:
			writeMask_stamp8<NUM_OF_LINES>(distFromLine, *currentBlock, outputOffset);
			for (int i = 0; i < NUM_OF_LINES; i++) { distFromLine[i] = _mm256_add_ps(distFromLine[i], line4A[i]); }
			outputOffset += 8;
		case 6:
			writeMask_stamp8<NUM_OF_LINES>(distFromLine, *currentBlock, outputOffset);
			for (int i = 0; i < NUM_OF_LINES; i++) { distFromLine[i] = _mm256_add_ps(distFromLine[i], line4A[i]); }
			outputOffset += 8;
		case 5:
			writeMask_stamp8<NUM_OF_LINES>(distFromLine, *currentBlock, outputOffset);
			for (int i = 0; i < NUM_OF_LINES; i++) { distFromLine[i] = _mm256_add_ps(distFromLine[i], line4A[i]); }
			outputOffset += 8;
		case 4:
			writeMask_stamp8<NUM_OF_LINES>(distFromLine, *currentBlock, outputOffset);
			for (int i = 0; i < NUM_OF_LINES; i++) { distFromLine[i] = _mm256_add_ps(distFromLine[i], line4A[i]); }
			outputOffset += 8;
		case 3:
			writeMask_stamp8<NUM_OF_LINES>(distFromLine, *currentBlock, outputOffset);
			for (int i = 0; i < NUM_OF_LINES; i++) { distFromLine[i] = _mm256_add_ps(distFromLine[i], line4A[i]); }
			outputOffset += 8;
		case 2:
			writeMask_stamp8<NUM_OF_LINES>(distFromLine, *currentBlock, outputOffset);
			for (int i = 0; i < NUM_OF_LINES; i++) { distFromLine[i] = _mm256_add_ps(distFromLine[i], line4A[i]); }
			outputOffset += 8;
		case 1:
			writeMask_stamp8<NUM_OF_LINES>(distFromLine, *currentBlock, outputOffset);
			for (int i = 0; i < NUM_OF_LINES; i++) { distFromLine[i] = _mm256_add_ps(distFromLine[i], line4A[i]); }
			outputOffset += 8;
		}
	}
}
template<unsigned int NUM_OF_LINES>
inline void DrawQuad_stamp8(BorderEightLines &border) {
	bareVec4i minmax = _mm_cvtps_epi32(_mm_add_ps(border.minMaxBounds, _mm_set1_ps(0.5f)));//minmax should be stored minx, miny, maxx,maxy // consequently cause of rounding, max is an open bound
	minmax = _mm_and_si128(minmax, _mm_set_epi32(~0, ~0, ~1, ~3)); //this does the following
																   //get nearest even, stamp is now aligned for algorithm in y
																   // get nearest multiple of 4 that is below number, it will now be byte-aligned in x (every byte is 4 width because of the shape of stamp which has a height of 2)
																   //now do clipping on min / max
	minmax = _mm_max_epi32(minmax, _mm_set_epi32(0, 0, 0, 0));
	minmax = _mm_min_epi32(minmax, _mm_set_epi32(height - 1, width - 1, height - 1, width - 1)); //should it not be -1 cause it's open bound?

																								 //get the window space coordinates for the first pixel to be checked for raster / bounds inclusion
																								 //actually, the first 8 pixels, the locations of the first "stamp", in order bottom left, top left, bottom right, top right, etc

	bareVec4f minYToFloats = _mm_cvtepi32_ps(_mm_shuffle_epi32(minmax, 0b01010101));
	bareVec4f minXToFloats = _mm_cvtepi32_ps(_mm_shuffle_epi32(minmax, 0b0));
	bareVec8f pixCoordX = _mm256_set_m128(minXToFloats, minXToFloats);
	bareVec8f pixCoordY = _mm256_set_m128(minYToFloats, minYToFloats);
	pixCoordX = _mm256_add_ps(pixCoordX, _mm256_set_ps(3.5f, 3.5f, 2.5f, 2.5f, 1.5f, 1.5f, 0.5f, 0.5f));
	pixCoordY = _mm256_add_ps(pixCoordY, _mm256_set_ps(1.5f, 0.5f, 1.5f, 0.5f, 1.5f, 0.5f, 1.5f, 0.5f));

	bareVec8f lineA[NUM_OF_LINES], lineB[NUM_OF_LINES], lineC[NUM_OF_LINES], line4A[NUM_OF_LINES], line2B[NUM_OF_LINES];
	__m256 four = _mm256_set1_ps(4);
	__m256 two = _mm256_set1_ps(2);
	bareVec8f distFromLine[NUM_OF_LINES];

	for (int i = 0; i <NUM_OF_LINES; i++) {

		lineB[i] = _mm256_set1_ps(border.B.get(i));
		lineC[i] = _mm256_set1_ps(border.C.get(i));
		lineA[i] = _mm256_set1_ps(border.A.get(i));
		distFromLine[i] = getDistances_stamp8(lineA[i], lineB[i], lineC[i], pixCoordX, pixCoordY); //remember, after the these first determinations are made for the first pixels, the other distances are made just by adding constants

		line4A[i] = _mm256_mul_ps(lineA[i], four);
		line2B[i] = _mm256_mul_ps(lineB[i], two);

	}
	//beter name for byteIndex is byteOffset from start of the buffer
	int byteIndex = (minmax.get(0) / 4) + (minmax.get(1) / 2)*(BlocksInRow * 8); // these divisions should be optimized to shifts by compiler
	int64_t* outputBlock = (int64_t*)(MaskBuffer.get() + byteIndex);
	RasterBox_stamp8<NUM_OF_LINES>(distFromLine, line4A, line2B, outputBlock, minmax);
}
template<unsigned int NUM_OF_LINES>
inline void DrawQuad_stamp8(QuadBorder &border) {
	bareVec4i minmax = _mm_cvtps_epi32(_mm_add_ps(border.minMaxBounds, _mm_set1_ps(0.5f)));//minmax should be stored minx, miny, maxx,maxy // consequently cause of rounding, max is an open bound
	minmax = _mm_and_si128(minmax, _mm_set_epi32(~0, ~0, ~1, ~3)); //this does the following
																   //get nearest even, stamp is now aligned for algorithm in y
																   // get nearest multiple of 4 that is below number, it will now be byte-aligned in x (every byte is 4 width because of the shape of stamp which has a height of 2)
																   //now do clipping on min / max
	minmax = _mm_max_epi32(minmax, _mm_set_epi32(0, 0, 0, 0));
	minmax = _mm_min_epi32(minmax, _mm_set_epi32(height - 1, width - 1, height - 1, width - 1)); //should it not be -1 cause it's open bound?

																								 //get the window space coordinates for the first pixel to be checked for raster / bounds inclusion
																								 //actually, the first 8 pixels, the locations of the first "stamp", in order bottom left, top left, bottom right, top right, etc

	bareVec4f minYToFloats = _mm_cvtepi32_ps(_mm_shuffle_epi32(minmax, 0b01010101));
	bareVec4f minXToFloats = _mm_cvtepi32_ps(_mm_shuffle_epi32(minmax, 0b0));
	bareVec8f pixCoordX = _mm256_set_m128(minXToFloats, minXToFloats);
	bareVec8f pixCoordY = _mm256_set_m128(minYToFloats, minYToFloats);
	pixCoordX = _mm256_add_ps(pixCoordX, _mm256_set_ps(3.5f, 3.5f, 2.5f, 2.5f, 1.5f, 1.5f, 0.5f, 0.5f));
	pixCoordY = _mm256_add_ps(pixCoordY, _mm256_set_ps(1.5f, 0.5f, 1.5f, 0.5f, 1.5f, 0.5f, 1.5f, 0.5f));

	bareVec8f lineA[NUM_OF_LINES], lineB[NUM_OF_LINES], lineC[NUM_OF_LINES], line4A[NUM_OF_LINES], line2B[NUM_OF_LINES];
	__m256 four = _mm256_set1_ps(4);
	__m256 two = _mm256_set1_ps(2);
	bareVec8f distFromLine[NUM_OF_LINES];

	for (int i = 0; i <NUM_OF_LINES; i++) {

		lineB[i] = _mm256_set1_ps(border.B.get(i));
		lineC[i] = _mm256_set1_ps(border.C.get(i));
		lineA[i] = _mm256_set1_ps(border.A.get(i));
		distFromLine[i] = getDistances_stamp8(lineA[i], lineB[i], lineC[i], pixCoordX, pixCoordY); //remember, after the these first determinations are made for the first pixels, the other distances are made just by adding constants

		line4A[i] = _mm256_mul_ps(lineA[i], four);
		line2B[i] = _mm256_mul_ps(lineB[i], two);

	}
	//beter name for byteIndex is byteOffset from start of the buffer
	int byteIndex = (minmax.get(0) / 4) + (minmax.get(1) / 2)*(BlocksInRow * 8); // these divisions should be optimized to shifts by compiler
	int64_t* outputBlock = (int64_t*)(MaskBuffer.get() + byteIndex);
	RasterBox_stamp8<NUM_OF_LINES>(distFromLine, line4A, line2B, outputBlock, minmax);
}

template<unsigned int NUM_OF_LINES>
inline void writeMask_stamp8(bareVec8f distanceFromLine[], int64_t& output, int& outputOffset) { //assuming barevec.x is 0-31 in the register
	__m256 result = distanceFromLine[0];
	for (int i = 1; i < NUM_OF_LINES; i++)
	{
		result = _mm256_and_ps(result, distanceFromLine[i]);
	}
	int64_t mask = _mm256_movemask_ps(result);
	output |= mask << outputOffset;
}
template<unsigned int NUM_OF_LINES>
inline int isQuadFullyOccludedRaster_stamp8(bareVec8f distFromLine[], bareVec8f line4A[], bareVec8f line2B[], int64_t *currentBlock, bareVec4i &minmax) {
	//assumptions made, rasterization starts aligned with block, the min.x() is aligned to a byte, outputOffset = 0 initially, this allows simple block picking, pointer outputBlock +=1
	//all arguments are correct for first stamp
	int columns = minmax.get(2) - minmax.get(0);//in column units to fill, since max is open-bound, the difference amount will include start pixel and end pixel
	int stampsForRow = (columns >> 2) + ((columns & 3) != 0); //if there is a remainder of division by 4, we add 1 more stamp to makre sure it stamps everything
	int rows = minmax.get(3) - minmax.get(1);
	int stampsForColumnRemaining = (rows >> 1) + (rows & 1);
	int outputOffset = 0;
	int64_t* firstBlock = currentBlock;
	bareVec8f firstDistancesFromLines[NUM_OF_LINES]; //each vector in array contains distances for each line
	for (int i = 0; i < NUM_OF_LINES; i++) { firstDistancesFromLines[i] = distFromLine[i]; }

	int64_t unmodifiedBlock = *firstBlock;
	int64_t writeBlock = *firstBlock;
	int wouldModifyOutput = 0; //really a bool
	stampsForColumnRemaining--;
	goto skipFirst;

	while (stampsForColumnRemaining > 0)
	{
		stampsForColumnRemaining--;
		firstBlock += (BlocksInRow);
		currentBlock = firstBlock;
		unmodifiedBlock = *currentBlock;
		writeBlock = *currentBlock;
		for (int i = 0; i < NUM_OF_LINES; i++) { distFromLine[i] = firstDistancesFromLines[i]; }
		for (int i = 0; i < NUM_OF_LINES; i++) { distFromLine[i] = _mm256_add_ps(distFromLine[i], line2B[i]); }
		for (int i = 0; i < NUM_OF_LINES; i++) { firstDistancesFromLines[i] = distFromLine[i]; }
		outputOffset = 0;

	skipFirst:

		int stampsForRowRemaining = stampsForRow;
		while (stampsForRowRemaining > 8) {
			for (int i = 0; i < 8; i++) {
				writeMask_stamp8<NUM_OF_LINES>(distFromLine, writeBlock, outputOffset);
				outputOffset += 8;
				for (int i = 0; i < NUM_OF_LINES; i++)
				{
					distFromLine[i] = _mm256_add_ps(distFromLine[i], line4A[i]);
				}
			}
			wouldModifyOutput |= (writeBlock ^ unmodifiedBlock); //if it adds coverage, it's not fully occuluded, return false

			outputOffset = 0;
			stampsForRowRemaining -= 8;
			currentBlock += 1;
			unmodifiedBlock = *currentBlock;
			writeBlock = *currentBlock;

		}
		switch (stampsForRowRemaining) {
		case 8:
			writeMask_stamp8<NUM_OF_LINES>(distFromLine, writeBlock, outputOffset);
			for (int i = 0; i < NUM_OF_LINES; i++) { distFromLine[i] = _mm256_add_ps(distFromLine[i], line4A[i]); }
			outputOffset += 8;
		case 7:
			writeMask_stamp8<NUM_OF_LINES>(distFromLine, writeBlock, outputOffset);
			for (int i = 0; i < NUM_OF_LINES; i++) { distFromLine[i] = _mm256_add_ps(distFromLine[i], line4A[i]); }
			outputOffset += 8;
		case 6:
			writeMask_stamp8<NUM_OF_LINES>(distFromLine, writeBlock, outputOffset);
			for (int i = 0; i < NUM_OF_LINES; i++) { distFromLine[i] = _mm256_add_ps(distFromLine[i], line4A[i]); }
			outputOffset += 8;
		case 5:
			writeMask_stamp8<NUM_OF_LINES>(distFromLine, writeBlock, outputOffset);
			for (int i = 0; i < NUM_OF_LINES; i++) { distFromLine[i] = _mm256_add_ps(distFromLine[i], line4A[i]); }
			outputOffset += 8;
		case 4:
			writeMask_stamp8<NUM_OF_LINES>(distFromLine, writeBlock, outputOffset);
			for (int i = 0; i < NUM_OF_LINES; i++) { distFromLine[i] = _mm256_add_ps(distFromLine[i], line4A[i]); }
			outputOffset += 8;
		case 3:
			writeMask_stamp8<NUM_OF_LINES>(distFromLine, writeBlock, outputOffset);
			for (int i = 0; i < NUM_OF_LINES; i++) { distFromLine[i] = _mm256_add_ps(distFromLine[i], line4A[i]); }
			outputOffset += 8;
		case 2:
			writeMask_stamp8<NUM_OF_LINES>(distFromLine, writeBlock, outputOffset);
			for (int i = 0; i < NUM_OF_LINES; i++) { distFromLine[i] = _mm256_add_ps(distFromLine[i], line4A[i]); }
			outputOffset += 8;
		case 1:
			writeMask_stamp8<NUM_OF_LINES>(distFromLine, writeBlock, outputOffset);
			for (int i = 0; i < NUM_OF_LINES; i++) { distFromLine[i] = _mm256_add_ps(distFromLine[i], line4A[i]); }
			outputOffset += 8;
		}
		wouldModifyOutput |= (writeBlock ^ unmodifiedBlock);
	}
	return wouldModifyOutput == 0;
}
template<unsigned int NUM_OF_LINES>
inline int isQuadFullyOccluded_stamp8(BorderEightLines &border) { //todo finish
	bareVec4i minmax = _mm_cvtps_epi32(_mm_add_ps(border.minMaxBounds, _mm_set1_ps(0.5f)));//minmax should be stored minx, miny, maxx,maxy // consequently cause of rounding, max is an open bound
	minmax = _mm_and_si128(minmax, _mm_set_epi32(~0, ~0, ~1, ~3)); //this does the following
																   //get nearest even, stamp is now aligned for algorithm in y
																   // get nearest multiple of 4 that is below number, it will now be byte-aligned in x (every byte is 4 width because of the shape of stamp which has a height of 2)
																   //now do clipping on min / max
	minmax = _mm_max_epi32(minmax, _mm_set_epi32(0, 0, 0, 0));
	minmax = _mm_min_epi32(minmax, _mm_set_epi32(height - 1, width - 1, height - 1, width - 1)); //should it not be -1 cause it's open bound?

																								 //get the window space coordinates for the first pixel to be checked for raster / bounds inclusion
																								 //actually, the first 8 pixels, the locations of the first "stamp", in order bottom left, top left, bottom right, top right, etc

	bareVec4f minYToFloats = _mm_cvtepi32_ps(_mm_shuffle_epi32(minmax, 0b01010101));
	bareVec4f minXToFloats = _mm_cvtepi32_ps(_mm_shuffle_epi32(minmax, 0b0));
	bareVec8f pixCoordX = _mm256_set_m128(minXToFloats, minXToFloats);
	bareVec8f pixCoordY = _mm256_set_m128(minYToFloats, minYToFloats);
	pixCoordX = _mm256_add_ps(pixCoordX, _mm256_set_ps(3.5f, 3.5f, 2.5f, 2.5f, 1.5f, 1.5f, 0.5f, 0.5f));
	pixCoordY = _mm256_add_ps(pixCoordY, _mm256_set_ps(1.5f, 0.5f, 1.5f, 0.5f, 1.5f, 0.5f, 1.5f, 0.5f));

	bareVec8f lineA[NUM_OF_LINES], lineB[NUM_OF_LINES], lineC[NUM_OF_LINES], line4A[NUM_OF_LINES], line2B[NUM_OF_LINES];
	__m256 four = _mm256_set1_ps(4);
	__m256 two = _mm256_set1_ps(2);
	bareVec8f distFromLine[NUM_OF_LINES];

	for (int i = 0; i <NUM_OF_LINES; i++) {

		lineB[i] = _mm256_set1_ps(border.B.get(i));
		lineC[i] = _mm256_set1_ps(border.C.get(i));
		lineA[i] = _mm256_set1_ps(border.A.get(i));
		distFromLine[i] = getDistances_stamp8(lineA[i], lineB[i], lineC[i], pixCoordX, pixCoordY); //remember, after the these first determinations are made for the first pixels, the other distances are made just by adding constants

		line4A[i] = _mm256_mul_ps(lineA[i], four);
		line2B[i] = _mm256_mul_ps(lineB[i], two);

	}
	//beter name for byteIndex is byteOffset from start of the buffer
	int byteIndex = (minmax.get(0) / 4) + (minmax.get(1) / 2)*(BlocksInRow * 8); // these divisions should be optimized to shifts by compiler
	int64_t* outputBlock = (int64_t*)(MaskBuffer.get() + byteIndex);
	return isQuadFullyOccludedRaster_stamp8<NUM_OF_LINES>(distFromLine, line4A, line2B, outputBlock, minmax);
}
template<unsigned int NUM_OF_LINES>
inline int isQuadFullyOccluded_stamp8(QuadBorder &border) { //todo finish
	bareVec4i minmax = _mm_cvtps_epi32(_mm_add_ps(border.minMaxBounds, _mm_set1_ps(0.5f)));//minmax should be stored minx, miny, maxx,maxy // consequently cause of rounding, max is an open bound
	minmax = _mm_and_si128(minmax, _mm_set_epi32(~0, ~0, ~1, ~3)); //this does the following
																   //get nearest even, stamp is now aligned for algorithm in y
																   // get nearest multiple of 4 that is below number, it will now be byte-aligned in x (every byte is 4 width because of the shape of stamp which has a height of 2)
																   //now do clipping on min / max
	minmax = _mm_max_epi32(minmax, _mm_set_epi32(0, 0, 0, 0));
	minmax = _mm_min_epi32(minmax, _mm_set_epi32(height - 1, width - 1, height - 1, width - 1)); //should it not be -1 cause it's open bound?

																								 //get the window space coordinates for the first pixel to be checked for raster / bounds inclusion
																								 //actually, the first 8 pixels, the locations of the first "stamp", in order bottom left, top left, bottom right, top right, etc

	bareVec4f minYToFloats = _mm_cvtepi32_ps(_mm_shuffle_epi32(minmax, 0b01010101));
	bareVec4f minXToFloats = _mm_cvtepi32_ps(_mm_shuffle_epi32(minmax, 0b0));
	bareVec8f pixCoordX = _mm256_set_m128(minXToFloats, minXToFloats);
	bareVec8f pixCoordY = _mm256_set_m128(minYToFloats, minYToFloats);
	pixCoordX = _mm256_add_ps(pixCoordX, _mm256_set_ps(3.5f, 3.5f, 2.5f, 2.5f, 1.5f, 1.5f, 0.5f, 0.5f));
	pixCoordY = _mm256_add_ps(pixCoordY, _mm256_set_ps(1.5f, 0.5f, 1.5f, 0.5f, 1.5f, 0.5f, 1.5f, 0.5f));

	bareVec8f lineA[NUM_OF_LINES], lineB[NUM_OF_LINES], lineC[NUM_OF_LINES], line4A[NUM_OF_LINES], line2B[NUM_OF_LINES];
	__m256 four = _mm256_set1_ps(4);
	__m256 two = _mm256_set1_ps(2);
	bareVec8f distFromLine[NUM_OF_LINES];

	for (int i = 0; i <NUM_OF_LINES; i++) {

		lineB[i] = _mm256_set1_ps(border.B.get(i));
		lineC[i] = _mm256_set1_ps(border.C.get(i));
		lineA[i] = _mm256_set1_ps(border.A.get(i));
		distFromLine[i] = getDistances_stamp8(lineA[i], lineB[i], lineC[i], pixCoordX, pixCoordY); //remember, after the these first determinations are made for the first pixels, the other distances are made just by adding constants

		line4A[i] = _mm256_mul_ps(lineA[i], four);
		line2B[i] = _mm256_mul_ps(lineB[i], two);

	}
	//beter name for byteIndex is byteOffset from start of the buffer
	int byteIndex = (minmax.get(0) / 4) + (minmax.get(1) / 2)*(BlocksInRow * 8); // these divisions should be optimized to shifts by compiler
	int64_t* outputBlock = (int64_t*)(MaskBuffer.get() + byteIndex);
	return isQuadFullyOccludedRaster_stamp8<NUM_OF_LINES>(distFromLine, line4A, line2B, outputBlock, minmax);
}