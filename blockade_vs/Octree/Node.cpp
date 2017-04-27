#include "Node.h"

namespace Octree {
	Node::Node()
	{
		color = ~0;
		for (int i = 0; i < 8; i++)
		{
			children[i] = NULL;
		}
	}
	bool Node::isLeaf()
	{
		__m256 result = _mm256_cmp_ps(_mm256_load_ps((float*)(children)), _mm256_load_ps((float*)(children + 4)), 0);
		return _mm256_movemask_ps(result) == 0b11111111;
	}

	Node::~Node()
	{
	}
}