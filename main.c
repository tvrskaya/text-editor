#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include "la.h"
#include "font.h"
#include <SDL.h>

void scc(int code)
{
	if (code < 0)
	{
		fprintf(stderr, "SDL ERROR: %s\n", SDL_GetError());
		exit(1);
	}
}

void *scp(void *ptr)
{
	if (ptr==NULL)
	{
		fprintf(stderr, "SDL ERROR: %s\n", SDL_GetError());
		exit(1);
	}
	return ptr;
}

void render_char(SDL_Renderer *renderer, SDL_Texture *font, Vec2f pos, char c, Uint32 color, float scale)
{
	const size_t index = c - 32;
	const size_t col = index % FONT_COLS;
	const size_t row = index / FONT_COLS;

	SDL_Rect src = {
		.x = col * FONT_CHAR_WIDTH,
		.y = row * FONT_CHAR_HEIGHT,
		.w = FONT_CHAR_WIDTH,
		.h = FONT_CHAR_HEIGHT,
	};

	SDL_Rect dst = {
		.x = (int) floorf(pos.x),
		.y = (int) floorf(pos.y),
		.w = (int) floorf(FONT_CHAR_WIDTH * scale),
		.h = (int) floorf(FONT_CHAR_HEIGHT * scale),
	};

	scc(SDL_SetTextureColorMod(font, 
				  (color >> (8 * 0)) & 0xff,
				  (color >> (8 * 1)) & 0xff,
				  (color >> (8 * 2)) & 0xff));
	
	scc(SDL_SetTextureAlphaMod(font, (color >> (8 * 3)) & 0xff));

	scc(SDL_RenderCopy(renderer, font, &src, &dst));
}

void render_text(SDL_Renderer *renderer, SDL_Texture *font, Vec2f pos, const char *text, Uint32 color, float scale)
{
	size_t n = strlen(text);
	Vec2f pen = pos;
	for (size_t i = 0; i < n; i++)
	{
		render_char(renderer, font, pen, text[i], color, scale);
		pen.x += FONT_CHAR_WIDTH * scale;
	}
}

int main(int argc, char* argv[])
{
	scc(SDL_Init(SDL_INIT_VIDEO));

	SDL_Window *window = 
		scp(SDL_CreateWindow("Text Editor", 100, 160, 800, 600, SDL_WINDOW_RESIZABLE));
	
	SDL_Renderer *renderer = 
		scp(SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED));		
	SDL_Surface *font_surface =
		scp(SDL_CreateRGBSurfaceFrom((void*)FONT, FONT_WIDTH, FONT_HEIGHT, 
					 24, 384, 0xff0000, 0x00ff00, 0x0000ff, 0));
        SDL_Texture *font_texture = 
		scp(SDL_CreateTextureFromSurface(renderer, font_surface));	
	
	SDL_Rect font_rect = {
		.x = 0,
		.y = 0,
		.w = font_surface->w,
		.h = font_surface->h, 
	};

	bool quit = false;
	while(!quit)
	{
		SDL_Event event = {0};
		while (SDL_PollEvent(&event))
		{
			switch(event.type)
			{
			case SDL_QUIT: {
				quit = true;
				}break;
			}
		}

		scc(SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0));
		scc(SDL_RenderClear(renderer));
		
		render_text(renderer, font_texture, vec2f(0.0, 0.0), 
				"Hello, world!", 0xff0000ff, 4);

		SDL_RenderPresent(renderer);
	}

	SDL_Quit();
	return 0;
}
