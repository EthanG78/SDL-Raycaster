#include "Raycaster.h"

/// <summary>
/// Constructor for Raycaster class. Initializes SDL and class variables. Lets the user set the window width and height.
/// </summary>
/// <param name="windowWidth">Width of the window</param>
/// <param name="windowHeight">Width of the window</param>
Raycaster::Raycaster(int windowWidth, int windowHeight)
{
	window = NULL;
	renderer = NULL;
	player = new SDL_Rect;

	wWidth = windowWidth;
	wHeight = windowHeight;

	isRunning = false;
	playerAngle = 0.0;
	playerDeltaX = 0.0;
	playerDeltaY = 0.0;
}

/// <summary>
/// Destructor
/// </summary>
Raycaster::~Raycaster()
{}

/// <summary>
/// Initializes SDL in the class. Creates the window with the previously passed width and height. Creates the SDL renderer
/// that will be used to render everything later on.
/// </summary>
/// <param name="title"></param>
void Raycaster::init(const char* title)
{
	// We can modify these flags in the future
	int flags = 0;
	if (SDL_Init(SDL_INIT_EVERYTHING) == 0)
	{
		window = SDL_CreateWindow(title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, wWidth, wHeight, flags);
		if (window == NULL)
		{
			std::cout << "There was an error creating the window: " << SDL_GetError() << std::endl;
		}

		renderer = SDL_CreateRenderer(window, -1, 0);
		if (renderer == NULL)
		{
			std::cout << "There was an error creating the renderer: " << SDL_GetError() << std::endl;
		}

		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

		isRunning = true;
	}
	else
	{
		isRunning = false;
	}
}

/// <summary>
/// Continuously polls for any keyboard events or if the window has been quit out of. 
/// Handles all of the user input to move the player.
/// </summary>
void Raycaster::handleEvents()
{
	SDL_Event event;
	SDL_PollEvent(&event);

	switch (event.type)
	{
	case SDL_QUIT:
		isRunning = false;
		break;
	default:
		break;
	}

	// Use this to check which keys have changed state
	// This movement is weirdly bugged, i believe it is due to SDL_Rect x any y
	// being a reference to the top left corner of the rectangle not the center
	// so we are technically moving the corner
	const Uint8* kb = SDL_GetKeyboardState(NULL);

	// Rotates counter clockwise
	if (kb[SDL_SCANCODE_LEFT])
	{
		playerAngle -= 0.1;
		if (playerAngle < 0.0) playerAngle += 2 * PI;
		playerDeltaX = (player->w / 2.0) * cos(playerAngle);
		playerDeltaY = (player->h / 2.0) * sin(playerAngle);
	}

	// Rotates clockwise
	if (kb[SDL_SCANCODE_RIGHT])
	{
		playerAngle += 0.1;
		if (playerAngle > 2 * PI) playerAngle -= 2 * PI;
		playerDeltaX = (player->w / 2.0) * cos(playerAngle);
		playerDeltaY = (player->h / 2.0) * sin(playerAngle);
	}

	if (kb[SDL_SCANCODE_UP])
	{
		player->x += playerDeltaX;
		player->y += playerDeltaY;
	}

	if (kb[SDL_SCANCODE_DOWN])
	{
		player->x -= playerDeltaX;
		player->y -= playerDeltaY;
	}
}

/// <summary>
/// Spawns player at the supplied coordinates with a default width and height of 10.
/// </summary>
/// <param name="x">Player starting x coordinate</param>
/// <param name="y">Player starting y coordinate</param>
void Raycaster::spawnPlayer(int x, int y)
{
	// Give the player initial coordinates supplied by user
	player->x = x;
	player->y = y;
	player->w = 10;
	player->h = 10;
}

/// <summary>
/// Renders __everything__ in the window. 
///		-First renders in the map supplied by the user, 1's represent walls and 0's represent 
///			open spaces for the player to move through. 
/// 
///		-Then renders in the player.
/// 
///		-Calulates all of the rays in a 60 degree field of view around the player, finds these ray lengths
///			and then uses these values to render in the 3D walls of the window.
/// 
///		TODO: Should we pass the map vector as a param?
/// 
/// </summary>
/// <param name="map">Vector of vectors of integers that represent the playable map layout</param>
void Raycaster::render(const std::vector<std::vector<int>>& map)
{
	SDL_RenderClear(renderer);

	// This will render in the game board at the top right of the screen
	// A 50 x 50 square
	SDL_Rect square{ 0, 0, 50, 50 };
	for (int i = 0; i < map.size(); i++)
	{
		square.y = 0 + (i * 50);
		for (int j = 0; j < map[i].size(); j++)
		{
			square.x = 0 + (j * 50);

			if (map[i][j] == 1)
			{
				// Draw a white wall
				SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
			}
			else
			{
				// Draw a grey square
				SDL_SetRenderDrawColor(renderer, 128, 128, 128, 255);
			}

			SDL_RenderFillRect(renderer, &square);
		}
	}

	// Render player
	SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
	SDL_RenderFillRect(renderer, player);

	// Calculate the center of the player rectangle for future use
	double playerXCenter = player->x + (player->w / 2.0);
	double playerYCenter = player->y + (player->h / 2.0);

	// Now we render in the rays
	// TODO: Use the method OLC does here https://www.youtube.com/watch?v=NbSee-XM7WA&t=1448s
	// We will be using a field of view of 60. So start the rays 30 degrees back, adding one degree each iteration until we 
	// are 30 degrees ahead of the angle of the player
	double rayAngle = playerAngle - 30 * (PI / 180);
	if (rayAngle < 0.0) rayAngle += 2 * PI;
	if (rayAngle > 2 * PI) rayAngle -= 2 * PI;

	bool wallFound = false;
	double rayX = playerXCenter;
	double rayY = playerYCenter;
	// Iterate 600 times because each degree represents ten pixels
	for (int i = 0; i < 600; i+=10)
	{
		wallFound = false;
		rayX = playerXCenter;
		rayY = playerYCenter;

		do
		{
			rayX += cos(rayAngle);
			rayY += sin(rayAngle);

			// Divide the current x and y of the ray by the size of the squares in the map
			// This integer division will round down to what index of the map we are currently in
			int col = rayX / 50;
			int row = rayY / 50;

			// Right now if you go outside the map the program just destroys itself
			if (row < map.size() && col < map[0].size() && map[row][col] == 1) wallFound = true;

		} while (!wallFound);

		// Set color of the ray to red
		SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
		SDL_RenderDrawLine(renderer, playerXCenter, playerYCenter, rayX, rayY);

		// Calculate the length of the ray
		double rayLength = sqrt((rayX - playerXCenter) * (rayX - playerXCenter) + (rayY - playerYCenter) * (rayY - playerYCenter));
		
		// Render in the 3D walls 

		// To get rid of the fisheye effect we multiply the length of the ray by the 
		// cosine of the difference between player angle and ray angle
		double angleDiff = playerAngle - rayAngle;
		if (angleDiff < 0.0) angleDiff += 2 * PI;
		if (angleDiff > 2 * PI) angleDiff -= 2 * PI;

		rayLength = rayLength * cos(angleDiff);

		double lineHeight = (50 * wHeight) / rayLength;
		if (lineHeight > wHeight) lineHeight = wHeight;
		

		// Offsets that help us calculate where to draw the vertical lines used for 3D walls
		// These ensure that the lines have a large enough x to clear the renderer map,
		// but also are in the middle of the screen and don't have parts that go off screen
		int yOffset = (wHeight / 2) - lineHeight / 2;
		int xOffset = map[0].size() * 50 + 100;

		// Render each line ten time, one pixel to the right each time so that each line has a width of 10 pixels
		for (int j = 0; j < 10; j++)
		{
			SDL_RenderDrawLine(renderer, i + j + xOffset, yOffset, i + j + xOffset, lineHeight + yOffset);
		}

		// Add one degree each iteration (1 degree = pi / 180 rads)
		rayAngle += PI / 180;
	}

	// Set the render color back to black
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderPresent(renderer);
}

/// <summary>
/// Deallocates the created SDL variables and tells the program to quit.
/// </summary>
void Raycaster::quit()
{
	isRunning = false;
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
}