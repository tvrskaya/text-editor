#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <assert.h>

#include "la.h"
#include "font.h"
#include <SDL.h>

#define FONT_SCALE 5

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


#define ROW_CAPACITY 16
#define COLS_AMOUNT 16

typedef struct {
	char text[COLS_AMOUNT];
    size_t reserve; //Max lenght
	size_t amount;	//Current lenght
}Text;

typedef struct Row {
	struct Row* pPrev; 
	struct Row* pNext; 
	Text* text;
}Row_t;

typedef struct {
	Row_t* head;
	size_t lenght;
}Head;

Row_t* get_row_pos(size_t row_pos, Head *head)
{
	Row_t *r = head->head; //TODO: check about allocating to manumulating with data
	for (size_t itr = 0; itr < row_pos; itr++)
	{
		r = r->pNext;
	}
	return r;
}

Text* get_text(size_t row_pos, Head *head)
{
	Row_t *r = get_row_pos(row_pos, head); //TODO: check about allocating to manumulating with data
	Text *text = r->text;
	return text;
}

Row_t* new_row(Row_t* pPrev, Row_t* pNext)
{
	Row_t *r = malloc(sizeof(Row_t));
	r->pPrev = pPrev;
	r->pNext = pNext;
	r->text = malloc(sizeof(Text));
	r->text->reserve = COLS_AMOUNT;
	r->text->amount = 0;
	return r;
}

void init_head(Head* head)
{
	head->head = new_row(NULL, NULL);
	head->lenght = 1;
}

void insert_row(size_t pos, Head *head)
{
	if (head->lenght < 1)
	{
		init_head(head);
		return;
	}
	
	Row_t *r = head->head;
	for (size_t i = 0; i < pos - 1; i++)
	{
		r = r->pNext;
	}

	Row_t* newRow = new_row(r, r->pNext);
	r->pNext = newRow;
	//r = r->pNext;
	//r->pPrev = newRow;
	head->lenght += 1;
}

void del_row(Row_t *r, Head *head)
{
	if (r->pNext != NULL)
	{
		r->pPrev->pNext = r->pNext;
		r->pNext->pPrev = r->pPrev;
	}
	else
	{
		r->pPrev->pNext = NULL;
	}
	free((void*)r);
	head->lenght -= 1;
}

void delete_row_pos(size_t pos, Head *head)
{
	if (pos > 0)
	{
		Row_t *r = get_row_pos(pos, head);
		del_row(r, head);
	}
}

typedef struct {
	size_t y_pos;
	size_t x_pos;
}Cursor;

Cursor c = {
	.y_pos = 0,
	.x_pos = 0,
};

void set_cursor(const int x, const int y)
{
	c.x_pos = x;
	c.y_pos = y;
}

#define ASCII_LOW 32
#define ASCII_HIGH 126

typedef struct {
	SDL_Texture *fontsheet;
	SDL_Rect char_table[ASCII_HIGH - ASCII_LOW + 1];
} Font;


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

void render_text(SDL_Renderer *renderer, Font *font, Vec2f pos, Head head, Uint32 color, float scale)
{
	Vec2f pen = pos;
	scc(SDL_SetTextureAlphaMod(font->fontsheet, (color >> (8 * 3)) & 0xff));
	scc(SDL_SetTextureColorMod(font->fontsheet, 
				  (color >> (8 * 0)) & 0xff,
				  (color >> (8 * 1)) & 0xff,
				  (color >> (8 * 2)) & 0xff));

	Row_t *row = head.head;
	for (size_t i = 0; i < head.lenght; i++)
	{
		for (size_t j = 0; j < row->text->amount; j++)
		{
			render_char(renderer, font, pen, row->text->text[j], scale);
			pen.x += FONT_CHAR_WIDTH * scale;
		}
		row = row->pNext;
		pen.y += FONT_CHAR_HEIGHT * scale;
		pen.x = 0;
	}
}


#define UNHEX(color) \
	((color >> (8*0)) & 0xff), \
	((color >> (8*1)) & 0xff), \
	((color >> (8*2)) & 0xff), \
	((color >> (8*3)) & 0xff) 

void render_cursor(SDL_Renderer *renderer, Uint32 color)
{
	SDL_Rect cursor = {
		.x = (int)c.x_pos * FONT_CHAR_WIDTH * FONT_SCALE,
		.y = (int)c.y_pos * FONT_CHAR_HEIGHT * FONT_SCALE,
		.w = FONT_CHAR_WIDTH * FONT_SCALE,
		.h = FONT_CHAR_HEIGHT * FONT_SCALE,
	};
	scc(SDL_SetRenderDrawColor(renderer, UNHEX(color)));
	scc(SDL_RenderFillRect(renderer, &cursor));
}

int main(int argc, char *argv[])
{
	scc(SDL_Init(SDL_INIT_VIDEO));

	SDL_Window *window = 
		scp(SDL_CreateWindow("Text Editor", 100, 160, 800, 600, SDL_WINDOW_RESIZABLE));
	
	SDL_Renderer *renderer = 
		scp(SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED));

	Font font = load_font(renderer);

	Head head;
	init_head(&head);

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
			case SDL_KEYDOWN: {
				switch(event.key.keysym.sym) {
				case SDLK_BACKSPACE: { //TODO: handle all cases(cursor pos is 0, deleting empty row)
					if (!(c.y_pos == 0 && c.x_pos == 0))
					{
						Row_t *row = get_row_pos(c.y_pos, &head);
						Text *txt = row->text;
						if (txt->amount == 0)
						{
							set_cursor(row->pPrev->text->amount, c.y_pos - 1);
							del_row(row, &head);
						}
						else
						{
							for(size_t i = c.x_pos; i < txt->amount - 1; i++)
							{
								txt->text[i] = txt->text[i - 1]; 
							}
							txt->amount -= 1;
							set_cursor(c.x_pos - 1, c.y_pos);
						}
					}		
					}break;
				case SDLK_RETURN: {
					set_cursor(0, c.y_pos + 1);
					insert_row(c.y_pos, &head);
				}break;
				}
				break;
			}
			case SDL_TEXTINPUT: {
				size_t text_size = strlen(event.text.text);
				const size_t free_row = COLS_AMOUNT - get_text(c.y_pos, &head)->amount;
				if (free_row > text_size)
				{
					memcpy(get_text(c.y_pos, &head)->text + c.x_pos , event.text.text, text_size);
				}
				else
				{
					set_cursor(0, c.y_pos + 1);
					insert_row(c.y_pos, &head);
					memcpy(get_text(c.y_pos, &head)->text + c.x_pos, event.text.text, text_size);
				}
				get_text(c.y_pos, &head)->amount += text_size;
				c.x_pos += text_size;
				}break;
			}		    
		}

		scc(SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0));
		scc(SDL_RenderClear(renderer));
		
		render_text(renderer, &font, vec2f(0.0, 0.0),
				head, 0xffffffff, FONT_SCALE);
		render_cursor(renderer, 0xffffffff);
		SDL_RenderPresent(renderer);
	}

	SDL_Quit();
	return 0;
}