// ----------------------------------------------------------------
// From Game Programming in C++ by Sanjay Madhav
// Copyright (C) 2017 Sanjay Madhav. All rights reserved.
// 
// Released under the BSD License
// See LICENSE in root directory for full details.
// ----------------------------------------------------------------

#pragma once
#include <cmath>
#include <list>
#include <random>
#include <vector>

#include "SDL/SDL.h"
#include "SDL/SDL_ttf.h"

// Vector2 struct just stores x/y coordinates
// (for now)
struct Vector2
{
	float x;
	float y;
};

class Ball {
public:
	Vector2 pos;
	Vector2 vel;
	Vector2 acc;
	float height;
	float width;
	bool onScreen;

	Ball(float x, float y, float vx, float vy, float h, float w)
		:pos({x, y}), vel({vx, vy}), height(h), width(w)
	{
		onScreen = true;
		acc = { 0.05f * fabsf(vx), 0.05f * fabsf(vy) };
	}

	bool collide(Vector2 o_pos, float o_height, float o_width){
		return ((pos.x < o_pos.x + o_width && o_pos.x < pos.x + width)
			&& (pos.y < o_pos.y + o_height && o_pos.y < pos.y + height));
	}

};

class Paddle {
public:
	Vector2 pos;
	int dir;
	float vel;
	float height;
	SDL_Scancode up;
	SDL_Scancode down;
//	int goals_taken;

	bool onScreen; 

	Paddle(float x, float y, float v, float h, SDL_Scancode u, SDL_Scancode d, bool show = true)
		:pos({x, y}), vel(v), height(h), dir(0), up(u), down(d), onScreen(show)
	{
		//constructor
	}

//	bool collide()

};

//Block class
class Block {
public:
	Vector2 pos;
	float height;
	float width;
	bool onScreen;

	Block(float x, float y, float w, float h, bool show = true)
		:pos({ x, y }), height(h), width(w), onScreen(show)
	{
		//constructor
	}

};

//Blockmap class
class BlockMap {
//private:
//	std::vector<std::vector<int>> matrix;

public:
	std::vector<std::vector<int>> matrix;
	float windowHeight;
	float windowWidth;
	float matrixHeight;
	float matrixWidth;
	BlockMap(): windowHeight(0), windowWidth(0), matrixHeight(0), matrixWidth(0) 
	{
	
	}
	BlockMap(float w_height, float w_width, int m_height, int m_width)
		: windowHeight(w_height), windowWidth(w_width), matrixHeight(m_height), matrixWidth(m_width) 
	{
		matrix.resize(m_height);
		matrix[0].resize(m_width, 0);
		matrix[1] = { 0,1,1,0,1,1,0 };
		matrix[2].resize(m_width, 0);
		matrix[3] = { 0,0,1,1,1,0,0 };
		matrix[4].resize(m_width, 0);
		/*for (auto& row : matrix) {
			row.resize(m_width);
		}*/
	}

	void Initialize(){
	}

};


// Game class
class Game
{
public:
	Game();
	// Initialize the game
	bool Initialize();
	// Runs the game loop until the game is over
	void RunLoop();
	// Shutdown the game
	void Shutdown();
private:
	// Helper functions for the game loop
	void ProcessInput();
	void UpdateGame();

	void DrawText(const char*, ...);

	void GenerateOutput();

	// Window created by SDL
	SDL_Window* mWindow;
	// Renderer for 2D drawing
	SDL_Renderer* mRenderer;

	TTF_Font* font;


	// Number of ticks since start of game
	Uint32 mTicksCount;
	// Game should continue to run

	bool mIsRunning;
	
	// Pong specific
	std::list<Ball> vBall;

	std::vector<Paddle> vPaddle;

	BlockMap map;

	std::list<Block> vBlock;

	int taps;

	//int goals_left;
	std::vector<int> goals;

	// 2021-08-28 - multiple balls
	// Position of ball2
	//Vector2 mBallPos2;
	//// Velocity of ball2
	//Vector2 mBallVel2;
};
