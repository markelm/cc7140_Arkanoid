// ----------------------------------------------------------------
// From Game Programming in C++ by Sanjay Madhav
// Copyright (C) 2017 Sanjay Madhav. All rights reserved.
// 
// Released under the BSD License
// See LICENSE in root directory for full details.
// ----------------------------------------------------------------

#include "Game.h"
#include <cmath>
#include <cstdio>
#include <cstdarg>

#define BUFFER_LENGTH 1024;

const int min_taps = 3;

const int max_balls = 15;

//sera usado para setar a altura de alguns objetos
const int thickness = 15;

float get_sign(float n)
{
	return n / fabsf(n);
}


Game::Game()
//para criar uma janela
:mWindow(nullptr)
//para fins de renderiza��o na tela
,mRenderer(nullptr)
//para guardar o tempo decorrido no jogo
,mTicksCount(0)
//verificar se o jogo ainda deve continuar sendo executado
,mIsRunning(true)
{
	
}

bool Game::Initialize()
{
	// Initialize SDL
	int sdlResult = SDL_Init(SDL_INIT_VIDEO);
	if (sdlResult != 0)
	{
		SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
		return false;
	}

	TTF_Init();
	
	// Create an SDL Window
	mWindow = SDL_CreateWindow(
		"Game Programming in C++ (Chapter 1)", // Window title
		100,	// Top left x-coordinate of window
		100,	// Top left y-coordinate of window
		1024,	// Width of window
		768,	// Height of window
		0		// Flags (0 for no flags set)
	);

	if (!mWindow)
	{
		SDL_Log("Failed to create window: %s", SDL_GetError());
		return false;
	}
	
	//// Create SDL renderer
	mRenderer = SDL_CreateRenderer(
		mWindow, // Window to create renderer for
		-1,		 // Usually -1
		SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
	);

	if (!mRenderer)
	{
		SDL_Log("Failed to create renderer: %s", SDL_GetError());
		return false;
	}

	font = TTF_OpenFont("VT323-Regular.ttf", 24);

	vPaddle = std::vector<Paddle>();
	vPaddle.push_back(
		Paddle(10.0f, 
			768.0f / 2.0f, 
			300.0f, 
			100.0f, 
			SDL_SCANCODE_W, 
			SDL_SCANCODE_S)
	);
	vPaddle.push_back(
		Paddle(1024.0f - (10.0f + thickness), 
			768.0f / 2.0f, 
			300.0f, 
			100.0f, 
			SDL_SCANCODE_I, 
			SDL_SCANCODE_K, 
			false)
	);
	
	vBall = std::list<Ball>();

	vBall.push_back(
		Ball(1024.0f / 2.0f,
			768.0f / 2.0f,
			-200.0f,
			235.0f, thickness, thickness)
	);

	taps = 0;

	//goals_left = 0;

	goals = std::vector<int>((size_t)2);

	//for (Ball &b : vBall)
	//{
	//	// posição inicial da bola
	//	b.pos.x = ;
	//	b.pos.y = ;
	//	// velocidade inicial da bola
	//	b.vel.x = ;
	//	b.vel.y = ;
	//}

	map = BlockMap(1024.0 / 2.0, 768.0, 5, 7);


	int i = 0;
	for (auto row : map.matrix) {
		for (int j = 0; j < map.matrixWidth; j++) {
			if (row[j] == 1) {
				float width = map.windowWidth / map.matrixWidth;
				float height = map.windowHeight / map.matrixHeight;
				float x = j * width;
				float y = i * height;
				vBlock.push_back(Block(x, y, width, height));
			}
		}
		i++;
	}

	return true;
}

void Game::RunLoop()
{
	while (mIsRunning)
	{
		ProcessInput();
		UpdateGame();
		GenerateOutput();
	}
}

void Game::ProcessInput()
{
	//evento, inputs do jogador s�o armazenados aqui
	SDL_Event event;
	while (SDL_PollEvent(&event))
	{
		switch (event.type)
		{
			// If we get an SDL_QUIT event, end loop
			case SDL_QUIT:
				mIsRunning = false;
				break;
		}
	}
	
	// Get state of keyboard - 
	// podemos buscar por alguma tecla espec�fica pressionada 
	// no teclado, nesse caso, Escape
	const Uint8* state = SDL_GetKeyboardState(NULL);
	// If escape is pressed, also end loop
	if (state[SDL_SCANCODE_ESCAPE])
	{
		mIsRunning = false;
	}
	
	// Update paddle direction based on W/S keys - 
	// atualize a dire��o da raquete com base na entrada do 
	// jogador
	// W -> move a raquete para cima, 
	// S -> move a raquete para baixo

	for(auto& paddle:vPaddle){
		bool was_shown = paddle.onScreen;
		paddle.dir = 0;
		if (state[paddle.up])
		{
			paddle.dir -= 1;
			paddle.onScreen = true;
			if (!was_shown) goals = { 0, 0 };
		}
		if (state[paddle.down])
		{
			paddle.dir += 1;
			paddle.onScreen = true;
			if (!was_shown) goals = { 0, 0 };
		}
	}	
}

void Game::UpdateGame()
{
	std::random_device rd;  //Will be used to obtain a seed for the random number engine
	std::mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()

	// Espere que 16ms tenham passado desde o �ltimo frame - 
	// limitando os frames
	while (!SDL_TICKS_PASSED(SDL_GetTicks(), mTicksCount + 16))
		;

	// Delta time � a diferen�a em ticks do �ltimo frame
	// (convertido pra segundos) - 
	// calcula o delta time para atualiza��o do jogo
	float deltaTime = (SDL_GetTicks() - mTicksCount) / 1000.0f;
	
	// "Clamp" (lima/limita) valor m�ximo de delta time
	if (deltaTime > 0.05f)
	{
		deltaTime = 0.05f;
	}

	// atualize a contagem de ticks par ao pr�ximo frame
	mTicksCount = SDL_GetTicks();
	
	// atualiza a posição da raquete
	for(auto& paddle:vPaddle){
		if (paddle.dir != 0)
		{
			// velocidade de 300 pixels por segundo
			paddle.pos.y += paddle.dir * paddle.vel * deltaTime;
			// verifique que a raquete n�o se move para fora da tela
			// usamos "thickness", 
			// que definimos como a altura dos elementos

			if (paddle.pos.y < (paddle.height / 2.0f + thickness))
			{
				paddle.pos.y = paddle.height / 2.0f + thickness;
			}
			else if (
				paddle.pos.y > (768.0f - paddle.height / 2.0f - thickness))
			{
				paddle.pos.y = 768.0f - paddle.height / 2.0f - thickness;
			}
		}
	}

	//Update Map
	
	// atualiza a posição da bola com base na sua velocidade
	for (Ball& b : vBall)
	{
		std::uniform_real_distribution<> dis(-0.5 * b.acc.x, 0.5 * b.acc.x);

		float var_x = dis(gen);
		float var_y = dis(gen);

		printf("var x: %.2f, var y: %.2f\n", var_x, var_y);

		b.pos.x += b.vel.x * deltaTime;
		b.pos.y += b.vel.y * deltaTime;

		float b_top = b.pos.y;
		float b_bottom = b.pos.y + thickness;

		float b_left = b.pos.x;
		float b_right = b.pos.x + thickness;

		// atualiza a posição da bola se ela colidiu com a raquete
		for(auto const& paddle : vPaddle) {
			bool left_p = paddle.pos.x < 1024.0f / 2.0f;

			float p_top = paddle.pos.y;
			float p_bottom = paddle.pos.y + paddle.height;

			float p_left = paddle.pos.x;
			float p_right = paddle.pos.x + thickness;

			if( paddle.onScreen &&
				// bolinha dentro do espaço da raquete
				(b_bottom >= p_top && b_top <= p_bottom)
				&& (b_left <= p_right && b_right >= p_left)
				// e bolinha ainda vai em direção à raquete
				&& ( (left_p && (b_left <= p_right && b.vel.x < 0.0f))
					|| (!left_p && (b_right >= p_left && b.vel.x > 0.0f)) )
			)
			{
				b.vel.x *= -1.0f;

				// acelera a cada colis�o
				// depois adicionar condi��o de apertar barra de espa�o
				b.vel.x += get_sign(b.vel.x) * b.acc.x;
				printf("vel_x: %.2f\n", b.vel.x);
				//b.vel.y += b.accel.y * deltaTime;*/
			}
			//Verifica se a bola saiu da tela (no lado esquerdo, onde � permitido)
		//Se sim, encerra o jogo
			else if (b_left <= 0.0f)
			{
				// marcada para dele��o
				b.onScreen = false;
				goals[0] += 1;
			}
			// Atualize (negative) a velocidade da bola se ela 
			// colidir com a parede da direita
			else if (b_right >= 1024.0f
				&& b.vel.x > 0.0f)
			{
				if (vPaddle[1].onScreen) {
					// marcada para dele��o
					b.onScreen = false;
					goals[1] += 1;
				}
				else {
					b.vel.x *= -1.0f;

					taps++;
					if (taps > min_taps && vBall.size() < max_balls) {
						vBall.push_back(Ball(b.pos.x, b.pos.y, b.vel.x + var_x, -b.vel.y + var_y, thickness, thickness));
						taps = 0;
					}
				}
			}
		}

		// Atualize (negative) a velocidade da bola se ela 
		// colidir com a parede de cima
		if (b_top <= 0.0f && b.vel.y < 0.0f)
		{
			b.vel.y *= -1.0f;

			taps++;
			if (taps > min_taps && vBall.size() < max_balls) {
				vBall.push_back(Ball(b.pos.x, b.pos.y, -b.vel.x + var_x, b.vel.y + var_y, thickness, thickness));
				taps = 0;
			}
		}
		// Atualize (negative) a velocidade da bola se ela 
		// colidir com a parede de baixo
		else if (b_bottom >= 768.0f
			&& b.vel.y > 0.0f)
		{
			b.vel.y *= -1.0f;

			taps++;
			if (taps > min_taps && vBall.size() < max_balls) {
				vBall.push_back(Ball(b.pos.x, b.pos.y, -b.vel.x + var_x, b.vel.y + var_y, thickness, thickness));
				taps = 0;
			}
		}
	}

	auto iter = vBall.begin();
	while (iter != vBall.end())
	{
		if (!iter->onScreen)
		{
			auto erase = iter;
			iter++;

			vBall.erase(erase);
		}
		else iter++;
	}
	
	if (vBall.size() == 0) mIsRunning = false;
}

void Game::DrawText(const char* fmt, ...) {
	char buffer[1024];

	va_list rest;
	va_start(rest, fmt);

	SDL_vsnprintf(buffer, 1024, fmt, rest);

	va_end(rest);

	SDL_Color color = {
		255, 255, 255, 255
	};

	SDL_Surface* text = TTF_RenderText_Solid(font, buffer, color);

	SDL_Texture* textureText = SDL_CreateTextureFromSurface(mRenderer, text);

	SDL_Rect dest = { 0 };

	SDL_QueryTexture(textureText, NULL, NULL, &dest.w, &dest.h);

	SDL_FreeSurface(text);

	dest.x = thickness;
	dest.y = 2 * thickness;

	SDL_RenderCopy(mRenderer, textureText, NULL, &dest);

	SDL_DestroyTexture(textureText);
}

//Desenhando a tela do jogo
void Game::GenerateOutput()
{
	// Setamos a cor de fundo para azul
	SDL_SetRenderDrawColor(
		mRenderer,
		0,   // R
		0,   // G
		255, // B
		255  // A
	);
	
	// limpa o back buffer
	SDL_RenderClear(mRenderer);
	
	// paredes brancas
	SDL_SetRenderDrawColor(mRenderer, 255, 255, 255, 255);

	// parede de cima
	SDL_Rect wall{
		0,        // top left x
		0,        // top left y
		1024,     // width
		thickness // height
	};
	SDL_RenderFillRect(mRenderer, &wall);

	// desenhamos as outras paredes apenas mudando as 
	// coordenadas de wall
	// parede de baixo
	wall.y = 768 - thickness;
	SDL_RenderFillRect(mRenderer, &wall);

	// parede da direita
	if (!vPaddle[1].onScreen) {
		wall.x = 1024 - thickness;
		wall.y = 0;
		wall.w = thickness;
		wall.h = 1024;
		SDL_RenderFillRect(mRenderer, &wall);
	}
	
	// como as posi��es da raquete e da bola ser�o atualizadas 
	// a cada itera��o do game loop, criamos "membros" na classe
	// Game.h para tal

	//mudar a cor da raquete
	SDL_SetRenderDrawColor(mRenderer, 0, 255, 0, 255);

	// desenhando a raquete - 
	// usando mPaddlePos que � uma struct de coordenada 
	// definida em Game.h
	
	for(auto const& paddle : vPaddle){
		if (paddle.onScreen) {
			SDL_Rect rPaddle{
				// static_cast converte de float para inteiros, 
				// pois SDL_Rect trabalha com inteiros
				static_cast<int>(paddle.pos.x),
				static_cast<int>(paddle.pos.y - paddle.height / 2),
				thickness,
				static_cast<int>(paddle.height)
			};
			SDL_RenderFillRect(mRenderer, &rPaddle);
		}
	}
	
	// desenhando a bola - 
	// usando mBallPos que � uma struct de coordenadas 
	// definida como membro em Game.h
	
	//mudar a cor do renderizador para a bola
	SDL_SetRenderDrawColor(mRenderer, 255, 255, 0, 255);

	for (Ball& b : vBall)
	{
		// Draw ball
		
		//Revisar posição da Bola
		SDL_Rect ball{
			static_cast<int>(b.pos.x - thickness / 2),
			static_cast<int>(b.pos.y - thickness / 2),
			thickness,
			thickness
		};

		SDL_RenderFillRect(mRenderer, &ball);
	}

	for (Block const &block : vBlock) {
		if (block.onScreen == true) {
			SDL_Rect renderedBlock{
				static_cast<int>(block.pos.x),
				static_cast<int>(block.pos.y),
				block.width,
				block.height
			};

			SDL_RenderFillRect(mRenderer, &renderedBlock);
		}
	}

	SDL_SetRenderDrawColor(mRenderer, 0, 0, 0, 255);

	printf("gols sofridos:");
	if (vPaddle[1].onScreen) printf("\nesq: %3d\ndir : % 3d\n", goals[0], goals[1]);
	else printf(" %3d\n", goals[0]);

	if (vPaddle[1].onScreen) DrawText("\n\nGols sofridos:\nesq: %3d\ndir : %3d\n", goals[0], goals[1]);
	else DrawText("\n\nGols sofridos: %3d\n", goals[0]);

	SDL_SetRenderDrawColor(mRenderer, 255, 0, 0, 255);
	
	// Swap front buffer and back buffer
	SDL_RenderPresent(mRenderer);
}

//Para encerrar o jogo
void Game::Shutdown()
{
	SDL_DestroyRenderer(mRenderer);//encerra o renderizador
	SDL_DestroyWindow(mWindow);//encerra a janela aberta
	SDL_Quit();//encerra o jogo
}
