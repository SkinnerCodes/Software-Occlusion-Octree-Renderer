#include "OctreeRenderer.h"
#include "RenderBox.h"
#include "QuadMask/QuadMask.h"
#include "QuadVAO.h"
#include "VisibilitySys.h"
#include "CullFrustum.h"
#include <thread>
#include <stopwatch.h>

using namespace SoftRendering;
struct OctreeRenderer::OpaquePart{
	QuadVAO quadVAO[8];
	QuadMask quadMask[8];
};
OctreeRenderer::OctreeRenderer(int screenWidth, int screenHeight, int maxNodesToRenderAtOnce)  //1000000
{
	this->screenWidth = screenWidth;
	this->screenHeight = screenHeight;
	opaquePart.reset(new OpaquePart());
	for (int i = 0; i < 4; i++)
	{
		opaquePart->quadVAO[i].initialize(maxNodesToRenderAtOnce /8);
		//opaquePart->quadMask[i].Initialize(screenWidth, screenHeight);
		int halfWidth = screenWidth / 2;
		int lastWidth = screenWidth - (halfWidth * 1);
		int halfHeight = screenHeight / 2;
		int lastHeight = screenHeight - (halfHeight * 1);
		opaquePart->quadMask[i].Initialize((i % 2) * halfWidth, (i / 2) * halfHeight, (i % 2) == 1 ? lastWidth : halfWidth, (i / 2) == 1 ? lastHeight : halfHeight);
		//		int quaterWidth = screenWidth / 4;
		//		int lastWidth = screenWidth - (quaterWidth * 3);
		//		int halfHeight = screenHeight / 2;
		//		int lastHeight = screenHeight - (halfHeight * 1);
		//		opaquePart->quadMask[i].Initialize((i % 4) * quaterWidth, (i / 2) * halfHeight, (i % 4) == 3 ? lastWidth : quaterWidth, (i / 2) == 1 ? lastHeight : halfHeight);
	}
	
}
void OctreeRenderer::Initialize(int screenWidth, int screenHeight, int maxNodesToRenderAtOnce)
{
	this->screenWidth = screenWidth;
	this->screenHeight = screenHeight;
	opaquePart.reset(new OpaquePart());
	for (int i = 0; i < 4; i++)
	{
		opaquePart->quadVAO[i].initialize(maxNodesToRenderAtOnce /8);
		//opaquePart->quadMask[i].Initialize(screenWidth, screenHeight);
		int halfWidth = screenWidth / 2;
		int lastWidth = screenWidth - (halfWidth * 1);
		int halfHeight = screenHeight / 2;
		int lastHeight = screenHeight - (halfHeight * 1);
		opaquePart->quadMask[i].Initialize( (i % 2) * halfWidth, (i /2) * halfHeight, (i%2) == 1 ? lastWidth : halfWidth , (i / 2) == 1 ? lastHeight : halfHeight);
//		int quaterWidth = screenWidth / 4;
//		int lastWidth = screenWidth - (quaterWidth * 3);
//		int halfHeight = screenHeight / 2;
//		int lastHeight = screenHeight - (halfHeight * 1);
//		opaquePart->quadMask[i].Initialize((i % 4) * quaterWidth, (i / 2) * halfHeight, (i % 4) == 3 ? lastWidth : quaterWidth, (i / 2) == 1 ? lastHeight : halfHeight);
	}
}

OctreeRenderer::~OctreeRenderer()
{

}
static void GenerateSubLayerForVAO(VisibilitySys::GenData *layerGenData, Octree::Node &root, glm::vec3 rootMinCorner, glm::vec3 rootMaxCorner)
{
	layerGenData->quadMask->Clear();

	VisibilitySys::FindVisibileVoxels(*layerGenData, root, bareVec4f(rootMinCorner), bareVec4f(rootMaxCorner));
}
#include "GameBase.h"
void OctreeRenderer::Draw(glm::mat4 view, glm::mat4 projection, Octree::Node &root, glm::vec3 rootMinCorner, glm::vec3 rootMaxCorner, int levelDepth)
{
	static int start = 0;

	const Uint8* keystate = SDL_GetKeyboardState(NULL);
	static bool drawOld = false;
	static bool pressB = false;
	static bool releaseB = false;
	static bool oldB = false;

	pressB = false; releaseB = false;
	if (keystate[SDL_SCANCODE_B] && !oldB) pressB = true;
	if (!keystate[SDL_SCANCODE_B] && oldB) releaseB = true;
	oldB = keystate[SDL_SCANCODE_B];
	if (pressB) drawOld = !drawOld;

	ProjectionData projData(view, projection, screenWidth, screenHeight);
	CullFrustum cullFrustum(projData);

	glUniformMatrix4fv(QuadVAO::uniformVP, 1, false, glm::value_ptr(projData.glmViewProjection)); //send vp to gpu incase the reset function doesn't do it
	for (int i = 0; i < 4; i++)
	{
		if (drawOld) {
			break;
		}
		//must be done on main thread because opengl is not thread safe normally
		if (true) {
			opaquePart->quadVAO[i].reset(projData.glmViewProjection); //todo, decouple shaderprogram and vao
		}
		else
		{
			//opaquePart->quadVAO[i].dummyReset(startingDummy);
		}
	}


	VisibilitySys::GenData layerGenData[8];
	CullFrustum subCullFrustums[8];
	cullFrustum.Split_4_way(subCullFrustums);
	thread threads[8];
	for (int i = 0; i < 4; i++)
	{
		if (drawOld) {
			break;
		}
		layerGenData[i].maxTreeLevel = levelDepth;
		layerGenData[i].quadMask = &opaquePart->quadMask[i];
		layerGenData[i].quadVAO = &opaquePart->quadVAO[i];
		layerGenData[i].projData = &projData;
		layerGenData[i].cullFrustum = &subCullFrustums[i];
		threads[i] = thread(GenerateSubLayerForVAO, &layerGenData[i], root, rootMinCorner, rootMaxCorner);
	}
//	layerGenData[0].maxTreeLevel = levelDepth;
//	layerGenData[0].quadMask = &opaquePart->quadMask[0];
//	layerGenData[0].quadVAO = &opaquePart->quadVAO[0];
//	layerGenData[0].projData = &projData;
//	layerGenData[0].cullFrustum = &cullFrustum;
//	GenerateSubLayerForVAO(&layerGenData[0], root, rootMinCorner, rootMaxCorner);
	StopWatch sw[8];
	if (!drawOld)
	{
		for (int i = 0; i < 4; i++)
		{
			sw[i].Start();
		}
		for (int i = 0; i < 4; i++)
		{
			threads[i].join();
			sw[i].Stop();
		}
		for (int i = 0; i < 4; i++)
		{

			fprintf(stdout, " %d: %lld ", i, sw[i].MilliSeconds());
		}
		fprintf(stdout, "\n");
	}
	for (int i = 0; i < 4; i++)
	{
		opaquePart->quadVAO[i].draw(0); //dicks
	}

	if (keystate[SDL_SCANCODE_SPACE]) opaquePart->quadMask[0].drawWithSDL(GameBase::renderer);

}
//void populateSubLayer(LayerGen::GenData *genData);
