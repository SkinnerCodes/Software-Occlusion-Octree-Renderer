#pragma once
#include "VoxelRenderBase.h"
namespace Octree {
	__declspec(align(32)) class Node
	{
	public:
		Node *children[8];
		uint32_t color; //sdl defines int32 in a portability macro, ignore warning

		Node();

		~Node();
		bool isLeaf();
	};
}
