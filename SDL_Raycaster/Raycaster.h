#pragma once

#include <iostream>
#include <vector>
#include <stdio.h>
#include <SDL.h>

constexpr double PI = 3.141592653589793238462643383279502884;

class Raycaster
{
private:
	SDL_Window* window;
	SDL_Renderer* renderer;
	SDL_Rect* player;

	int wWidth;
	int wHeight;

	bool isRunning;
	double playerAngle;
	double playerDeltaX;
	double playerDeltaY;

public:
	Raycaster(int windowWidth, int windowHeight);
	~Raycaster();

	void init(const char* title);
	void handleEvents();
	void spawnPlayer(int x, int y);
	void render(const std::vector<std::vector<int>>& board);
	void quit();

	bool running() { return isRunning; }

};
