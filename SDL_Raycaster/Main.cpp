/// Author: Ethan Garnier
#include "Raycaster.h"

constexpr int WIDTH = 1400;
constexpr int HEIGHT = 800;
constexpr int FRAME_CAP = 60;
constexpr int FRAME_DELAY = 1000 / FRAME_CAP;

Raycaster *caster = nullptr;

// Ones represent the walls in the game map
const std::vector<std::vector<int>> map = {
	{1,1,1,1,1,1,1,1},
	{1,0,0,0,0,0,0,1},
	{1,0,1,1,0,0,0,1},
	{1,0,0,0,0,0,0,1},
	{1,0,0,0,0,1,0,1},
	{1,0,1,0,0,1,0,1},
	{1,0,0,0,0,1,0,1},
	{1,1,1,1,1,1,1,1}
};

int main(int argc, char *argv[])
{
	caster = new Raycaster(WIDTH, HEIGHT);

	caster->init("Raytracing Application");

	caster->spawnPlayer(75, 75);

	Uint32 frameStart;
	int frameTime;

	// Main game loop
	while (caster->running())
	{
		frameStart = SDL_GetTicks();

		caster->handleEvents();
		caster->render(map);

		frameTime = SDL_GetTicks() - frameStart;

		// Restric the loop to run only 60 times a second
		if (FRAME_DELAY > frameTime)
		{
			SDL_Delay(FRAME_DELAY - frameTime);
		}
	}

	caster->quit();

	return 0;
}