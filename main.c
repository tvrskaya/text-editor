#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <assert.h>

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

#define ASCII_LOW 32
#define ASCII_HIGH 126

typedef struct {
	SDL_Texture *fontsheet;
	SDL_Rect char_table[ASCII_HIGH - ASCII_LOW + 1];
} Font;

#define BUFFER_CAPACITY 1024
char buffer[BUFFER_CAPACITY];
size_t buffer_reserve = 0;

Font load_font(SDL_Renderer *renderer)
{
	Font font = {0};

	SDL_Surface *font_surface =
		scp(SDL_CreateRGBSurfaceFrom((void*)FONT, FONT_WIDTH, FONT_HEIGHT, 
					 3*8, 3*FONT_WIDTH, 0xff0000, 0x00ff00, 0x0000ff, 0));
        font.fontsheet = 
		scp(SDL_CreateTextureFromSurface(renderer, font_surface));	
	
	SDL_FreeSurface(font_surface);
	
	for (size_t ascii = ASCII_LOW; ascii < ASCII_HIGH; ++ascii)
	{
		const size_t index = ascii - ASCII_LOW;
		const size_t col = index % FONT_COLS;
		const size_t row = index / FONT_COLS;
		font.char_table[index] = (SDL_Rect) {
			.x = col*FONT_CHAR_WIDTH,
			.y = row*FONT_CHAR_HEIGHT,
			.h = FONT_CHAR_HEIGHT,
			.w = FONT_CHAR_WIDTH,
		};
	}

	return font;
}

void render_char(SDL_Renderer *renderer, Font *font, Vec2f pos, char c, float scale)
{
	const size_t index = c - 32;

	SDL_Rect dst = {
		.x = (int) floorf(pos.x),
		.y = (int) floorf(pos.y),
		.w = (int) floorf(FONT_CHAR_WIDTH * scale),
		.h = (int) floorf(FONT_CHAR_HEIGHT * scale),
	};

	assert(c >= ASCII_LOW);	
	assert(c <= ASCII_HIGH);

	scc(SDL_RenderCopy(renderer, font->fontsheet, &font->char_table[index], &dst));
}

void render_text(SDL_Renderer *renderer, Font *font, Vec2f pos, const char *text, Uint32 color, float scale)
{
	size_t text_size = strlen(text);
	Vec2f pen = pos;
	scc(SDL_SetTextureAlphaMod(font->fontsheet, (color >> (8 * 3)) & 0xff));
	scc(SDL_SetTextureColorMod(font->fontsheet, 
				  (color >> (8 * 0)) & 0xff,
				  (color >> (8 * 1)) & 0xff,
				  (color >> (8 * 2)) & 0xff));

	for (size_t i = 0; i < buffer_reserve; i++)
	{
		render_char(renderer, font, pen, text[i], scale);
		pen.x += FONT_CHAR_WIDTH * scale;
	}
}

int main(int argc, char *argv[])
{
	scc(SDL_Init(SDL_INIT_VIDEO));

	SDL_Window *window = 
		scp(SDL_CreateWindow("Text Editor", 100, 160, 800, 600, SDL_WINDOW_RESIZABLE));
	
	SDL_Renderer *renderer = 
		scp(SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED));

	Font font = load_font(renderer);

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
				} break;
			case SDL_TEXTINPUT: {
				size_t text_size = strlen(event.text.text);
				const size_t free_buffer = BUFFER_CAPACITY - buffer_reserve;
				if (text_size > free_buffer)
					text_size = free_buffer;
				memcpy(buffer+buffer_reserve, event.text.text, text_size);
				buffer_reserve += text_size;
				}break;
			}		    
		}

		scc(SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0));
		scc(SDL_RenderClear(renderer));
		
		render_text(renderer, &font, vec2f(0.0, 0.0),
				buffer, 0xffffffff, 4);

		SDL_RenderPresent(renderer);
	}

	SDL_Quit();
	return 0;
}
