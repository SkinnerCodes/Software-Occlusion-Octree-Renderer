#pragma once
//#include "BlockGameBase.h"
#include "Render Sys/camera.h"
#include "Render Sys/OctreeRenderer.h"

#include "Octree/Node.h"

using namespace SoftRendering;
using namespace Octree;
#include "SDL/sdl.h"
class GameBase
{
public:
	SDL_Window* mainWindow;
	SDL_GLContext mainGLContext;
	bool quiting;
	SDL_Texture* texture;
	static SDL_Renderer* renderer;
	SDL_Rect texture_rect;
	Camera camera;
	int width, height;
	Node root;
	Node leaf;
	Node nodes[20];
	OctreeRenderer octreeRenderer;
public:
	// Constructor and destructor
	GameBase(void);
	virtual ~GameBase(void);

	// Initialization functions
	void InitApp(void);
	void InitializeSDL(Uint32 width, Uint32 height, Uint32 flags);
	// Event-related functions
	void PollEvents(Uint32 timeLastFrame);
	void HandleUserEvents(SDL_Event* event);

	// Game related functions
	void GameLoop(void);
	void Update(Uint32 timeLastFrame);
	void Draw(Uint32 timeLastFrame);

	//debug
	inline void fpsCounterUpdate() //call this every frame
	{
		static uint32_t lastFrame;
		static int start = 1;
		if (start) { start = 0; lastFrame = SDL_GetTicks(); return; }
		uint32_t thisFrame = SDL_GetTicks();
		if (thisFrame % 34 != 0) {
			lastFrame = thisFrame;
			return;
		}
		fprintf(stdout, "last frame: %f\n", 1.0f / ((thisFrame - lastFrame) / 1000.0f));

		lastFrame = thisFrame;

	}

};
