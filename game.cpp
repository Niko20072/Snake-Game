#include "game.h"
#include "surface.h"
#include <cstdio> //printf
#include <windows.h>
#include "template.h"
#define SnakeSize 100

namespace Tmpl8
{
	struct TGAHeader
	{
		unsigned char ID, colmapt;// set both to 0
		unsigned char type;// set to 2
		unsigned char colmap[5];// set all elements to 0
		unsigned short xorigin, yorigin;// set to 0
		unsigned short width, height;// put image size here
		unsigned char bpp;// set to 32
		unsigned char idesc;// set to 0
	};

	TGAHeader header;

	Sprite image(new Surface("assets/gameover.jpg"), 1);
	bool gameover = false;
	int x[SnakeSize],y[SnakeSize];
	int foodx = rand() % (800 / 8), foody = rand() % (512 / 8);
	int heading = 0;
	int delta[8] = { 1, 0, 0, 1, -1, 0, 0, -1 };
	int head = 7, tail = 0;
	int score = 0;

	void Boundary(Surface* screen)
	{
		screen->Line(0, 0, 0, 512, 0xFFFF00);
		screen->Line(799, 0, 799, 512, 0xFFFF00);
		screen->Line(0, 511, 799, 511, 0xFFFF00);
		screen->Line(0, 1, 800, 1, 0xFFFF00);
	}

	void DrawScore(Surface* screen, int score)
	{
		screen->Bar(10, 10, 90, 20, 0); // 0 = culoarea fundalului
		char text[32];
		sprintf(text, "Score: %d", score);
		screen->Print(text, 10, 10, 0xFFFFFF);
	}

	void Collision(Surface* screen, int headx, int heady)
	{
		//Verificare boundary inainte sa citim buffer
		if (headx < 0 || headx >= 800 / 8 || heady < 0 || heady >= 512 / 8)
		{
			printf("Game Over\n");
			gameover = true;
			return;
		}
		//Verificare coliziune cu corpul (pixel verde)
		int pixelX = headx * 8;
		int pixelY = heady * 8;
		Pixel* buffer = screen->GetBuffer();
		Pixel color = buffer[pixelY * 800 + pixelX]; // 800 = screenWidth
		if (color == 0x00ff00)
		{
			printf("Game Over\n");
			gameover = true;
		}
	}

	void RestartGame(Surface* screen)
	{
		screen->Clear(0); // curatam ecranul
		// Reseteaza variabilele globale
		head = 7;
		tail = 0;
		heading = 0;
		score = 0;
		gameover = false;

		// Reseteaza pozitia sarpelui
		int index = 5;
		for (int i = 0; i <= 7; i++)
		{
			x[i] = index++;
			y[i] = 8;
		}

		// Reseteaza mancarea
		foodx = rand() % (800 / 8);
		foody = rand() % (512 / 8);
	}
	
	void GameOverScreen(Surface* screen)
	{
		screen->Clear(0);
		image.Draw(screen, 20, 0);
		screen->Print("Press Space to restart", 340, 350, 0xFFFFFF);
	}

	// -----------------------------------------------------------
	// Initialize the application
	// -----------------------------------------------------------

	void Game::Init()
	{
		int index = 5;
		for (int i = 0; i <= 7; i++)
		{
			x[i] = index++;
			y[i] = 8;
		}
	}

	void Game::ScreenShot(Surface* screen)
	{
		header.ID = 0;
		header.colmapt = 0;
		header.type = 2;
		for (int i = 0; i < 5; i++) header.colmap[i] = 0;
		header.xorigin = 0;
		header.yorigin = 0;
		header.width = 800;
		header.height = 512;
		header.bpp = 32;
		header.idesc = 0;
		FILE* image = fopen("screenshot.tga", "wb");
		fwrite(&header, sizeof(TGAHeader), 1, image);
		for (int y = 0; y < header.height; y++)
		{
			for (int x = 0; x < header.width; x++)
			{
				//Indexarea in buffer incepe de la 0. Deci ultimul rand din buffer este header.height - 1
				Pixel p = screen->GetBuffer()[x + (header.height - 1 - y) * header.width]; //nu pun y normal pt ca imi stocheaza imaginea de jos in sus, asa ca incep de la inaltime-1 sa o iau invers
				fwrite(&p, sizeof(p), 1, image);
			}
		}
		fclose(image);
	}

	// -----------------------------------------------------------
	// Close down application
	// -----------------------------------------------------------
	void Game::Shutdown()
	{
	}

	// -----------------------------------------------------------
	// Main application tick function
	// -----------------------------------------------------------

	void Game::Tick(float deltaTime)
	{
		if (GetAsyncKeyState('E'))
			ScreenShot(screen);
		if (gameover==true)
		{
			GameOverScreen(screen);
			if (GetAsyncKeyState(VK_SPACE))
				RestartGame(screen); // apelam subprogramul
			return; // nu face altceva pana nu dai Enter
		}
		//Desenam granita
		DrawScore(screen, score);
		Boundary(screen);
		//Calculam noua pozitie a capului
		int headx = x[head] + delta[heading * 2];
		int heady = y[head] + delta[heading * 2 + 1];
		//Verificam coliziunea boundary si corp
		Collision(screen, headx, heady);
		//daca mananca, generam mancare noua
		if (headx == foodx && heady == foody)
		{
			// stergem zona scorului
			screen->Bar(foodx * 8, foody * 8, foodx * 8 + 6, foody * 8 + 6, 0);
			foodx = rand() % (800 / 8); // 0 - 99
			foody = rand() % (512 / 8); // 0 - 
			score++;
		}
		else //stergem coada daca nu mananca
		{
			screen->Box(x[tail] * 8, y[tail] * 8, x[tail] * 8 + 6, y[tail] * 8 + 6, 0);
			tail = (tail + 1) % SnakeSize;
		}
		//Mutam capul
		head = (head + 1) % SnakeSize;
		x[head] = headx;
		y[head] = heady;
		//Desenam capul si mancarea
		screen->Box(x[head] * 8, y[head] * 8, x[head] * 8 + 6, y[head] * 8 + 6, 0x00ff00);
		screen->Bar(foodx * 8, foody * 8, foodx * 8 + 6, foody * 8 + 6, 0x0000ff);
		//Control
		if (GetAsyncKeyState(VK_LEFT)) heading = (heading + 3) % 4;
		if (GetAsyncKeyState(VK_RIGHT)) heading = (heading + 1) % 4;

		Sleep(100);
	}

};