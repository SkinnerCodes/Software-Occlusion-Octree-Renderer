
#include "GameBase.h"
#include "stopwatch.h"


SDL_Renderer* GameBase::renderer;
// Constructor
GameBase::GameBase(void)
{
	quiting = false;

}

// Destructor
GameBase::~GameBase(void)
{
	SDL_GL_DeleteContext(mainGLContext);
	SDL_DestroyWindow(mainWindow);
	SDL_Quit();
}


// Initialization functions
void GameBase::InitApp(void)
{
	//assert(_CrtCheckMemory()); //use this check for heap corruption but may not work
	width = 1080;  height = 720;

	Uint32 contextFlags;
	contextFlags = SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL;

	// Create a 640 by 480 window.

	InitializeSDL(width, height, contextFlags);

	glewExperimental = GL_TRUE;
	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		/* Problem: glewInit failed, something is seriously wrong. */
		fprintf(stderr, "Error: %p\n", glewGetErrorString(err));

	}
	fprintf(stdout, "Status: Using GLEW %p\n", glewGetString(GLEW_VERSION));


	renderer = SDL_CreateRenderer(mainWindow, -1, SDL_RENDERER_ACCELERATED);

	//texture = IMG_LoadTexture(renderer, "C:\\Users\\Thomas\\Pictures\\gabe.jpg");
	texture_rect.x = 0;  //the x coordinate
	texture_rect.y = 0; // the y coordinate
	texture_rect.w = width; //the width of the texture
	texture_rect.h = height; //the height of the texture

	camera.SetMode(FREE);	//Two Modes FREE and ORTHO
	camera.SetViewport(0, 0, width, height);
	//camera.SetPosition(glm::vec3(1.86585140, 1.67026091, 10.3906651));
	//camera.SetPosition(glm::vec3(9.62623787, 1.45289886, 8.18322182));
	
//	camera.SetPosition(glm::vec3(37.0496864, 1.93207490, -7.55248642));
//	camera.SetLookAt(glm::vec3(36.0659561, 2.03216910, -7.40330791));	
	
	camera.SetPosition(glm::vec3(-10.0, 0.0, 0.0));
	camera.SetLookAt(glm::vec3(0.0, 0.0, 0.0));
	//camera.SetLookAt(glm::vec3(9.68458557, 1.44019890, 7.18500614));
	camera.SetClipping(1, 500);
	camera.SetFOV(45);

	camera.Update();

	//test code
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glDisable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	root.color = 0xff0000ff;
	root.children[3] = &nodes[3];
	root.children[1] = &nodes[1];
	root.children[2] = &nodes[2];
	root.children[0] = &nodes[0];
	root.children[4] = &nodes[3];
	root.children[5] = &nodes[1];
	root.children[6] = &nodes[2];
	root.children[7] = &nodes[0];
	nodes[2].color = 0x00f00f00;
	nodes[0].color = 0x00ff0000;
	nodes[1].color = 0x0000ff00;
	nodes[3].color = 0x000000ff;

	for (int i = 0; i < 4; i++) {
		nodes[i].children[3] = &root;
		nodes[i].children[1] = &root;
		nodes[i].children[2] = &root;
		nodes[i].children[0] = &root;
	}
	octreeRenderer = OctreeRenderer(width, height, 1000000);
}

void GameBase::InitializeSDL(Uint32 width, Uint32 height, Uint32 flags)
{
	if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
	{
		printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
	}
	// Turn on double buffering.
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3); // TODO make this 4 later
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

	// Create the window

	mainWindow = SDL_CreateWindow("gabe", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, flags);
	if (mainWindow == NULL)
	{
		printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
	}
	mainGLContext = SDL_GL_CreateContext(mainWindow); //giant penis

	if (mainGLContext == NULL)
	{
		printf("GL context could not be created! SDL_Error: %s\n", SDL_GetError());
	}
	SDL_GL_MakeCurrent(mainWindow, mainGLContext);// todo: figure out if u need this
	SDL_GL_SetSwapInterval(0); //disable vertical sync, this is important for testing because other wise random opengl functions will block in order to sync with framerate


}


// Event-related functions
void GameBase::PollEvents(Uint32 timeLastFrame)
{
	SDL_Event event;

	while (SDL_PollEvent(&event) != 0) {
		switch (event.type) {
		case SDL_USEREVENT:
			HandleUserEvents(&event);
			break;

		case SDL_KEYDOWN:
			// Quit when user presses a key.
			switch (event.key.keysym.sym) {
			case SDLK_ESCAPE:
				quiting = true;
				break;
			}
			break;

		case SDL_QUIT:
			quiting = true;
			break;

		default:
			break;
		}   // End switch

	}   // End while
	const Uint8* keystate = SDL_GetKeyboardState(NULL);

	//continuous-response keys
	float scale = timeLastFrame / 50.0f;
	if (keystate[SDL_SCANCODE_LEFT])
	{
		camera.ChangeHeading(0.01f * scale);
	}
	if (keystate[SDL_SCANCODE_RIGHT])
	{
		camera.ChangeHeading(-0.01f * scale);
	}
	if (keystate[SDL_SCANCODE_UP])
	{
		camera.ChangePitch(0.01f *scale );
	}
	if (keystate[SDL_SCANCODE_DOWN])
	{
		camera.ChangePitch(-0.01f *scale);
	}
	camera.camera_scale = scale /10.0f;// keystate[SDL_SCANCODE_SPACE] ? 0.01f : 0.2f;
	if (keystate[SDL_SCANCODE_W]) camera.Move(FORWARD);
	if (keystate[SDL_SCANCODE_S]) camera.Move(BACK);
	if (keystate[SDL_SCANCODE_D]) camera.Move(RIGHT);
	if (keystate[SDL_SCANCODE_A]) camera.Move(LEFT);
	if (keystate[SDL_SCANCODE_F]) camera.SetFOV(camera.field_of_view - 0.01);
	if (keystate[SDL_SCANCODE_G]) camera.SetFOV(camera.field_of_view + 0.01);


}

void GameBase::HandleUserEvents(SDL_Event* event)
{
	// use this to handle custom events
	switch (event->user.code) {
	case 0: //need to make a constant or enum to put names to user values 

		break;

	default:
		break;
	}
}


// Game related functions
void GameBase::GameLoop(void)
{
	Uint32 startOfLastFrame = SDL_GetTicks();
	Uint32 timeLastFrame; //time of last frame
	while (!quiting) {
		fpsCounterUpdate();
		timeLastFrame = SDL_GetTicks() - startOfLastFrame;
		startOfLastFrame = SDL_GetTicks();
		PollEvents(timeLastFrame);
		Update(timeLastFrame);
		Draw(timeLastFrame);

		//glFinish(); //probably a bad idea to use this
		GLenum error = glGetError();
		if (error != GL_NO_ERROR)
		{
			//printf("openGL error somewhere: %d\n", error);
		}
		const Uint8* keystate = SDL_GetKeyboardState(NULL);
		if (!keystate[SDL_SCANCODE_SPACE]) {
			SDL_GL_SwapWindow(mainWindow); //used for just plain opengl
		}
		else {
			SDL_RenderPresent(renderer); //flushes SDL rendering calls, unclear if this will work with normal opengl
		}


	}

}

void GameBase::Update(Uint32 timeLastFrame)
{

}
void GameBase::Draw(Uint32 timeLastFrame)
{
	SDL_RenderClear(renderer);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	camera.Update();
	mat4 v, p; camera.GetMatricies(v, p);
	octreeRenderer.Draw(v, p, root, vec3(0, 0, 0), vec3(10, 10, 10), 7);

}