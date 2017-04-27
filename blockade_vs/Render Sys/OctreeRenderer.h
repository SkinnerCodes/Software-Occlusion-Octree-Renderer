#pragma once
#include "glm/glm.hpp"
#include <memory>
#include "Octree/Node.h"
class OctreeRenderer
{
public:
	int screenWidth, screenHeight;
	struct OpaquePart;
	std::shared_ptr<OpaquePart> opaquePart;
	inline OctreeRenderer() {}
	~OctreeRenderer();
	inline void Initialize(int screenWidth, int screenHeight, int maxLeafNodesToRenderAtOnce);
	OctreeRenderer(int screenWidth, int screenHeight, int maxLeafNodesToRenderAtOnce);
	//levelDepth is the amount of levels to consider for rendering after the root level
	void Draw(glm::mat4 view, glm::mat4 projection, Octree::Node &root, glm::vec3 rootMinCorner, glm::vec3 rootMaxCorner, int levelDepth);


};