#include "stdafx.h"
#include "curses.h"
#include <iostream>

const int FIELD_X = 6;
const int FIELD_Y = 12;
const int FIELD_POS_X = 12;
const int FIELD_POS_Y = 10;

enum Cell { EMPTY, RED, GREEN, YELLOW, BLUE };

Cell game_field[FIELD_Y][FIELD_X];

WINDOW *field_win;

void DrawCell(Cell cell, int y, int x)
{
	int ch;
	int color;

	switch (cell)
	{
	case EMPTY:
		ch = L'\u0020';
		color = COLOR_PAIR(0);
		break;
	case RED:
		ch = L'\u0398';
		color = COLOR_PAIR(1);
		break;
	case GREEN:
		ch = L'\u0398';
		color = COLOR_PAIR(2);
		break;
	case YELLOW:
		ch = L'\u0398';
		color = COLOR_PAIR(3);
		break;
	case BLUE:
		ch = L'\u0398';
		color = COLOR_PAIR(4);
		break;
	default:
		ch = L'\u263A';
		color = COLOR_PAIR(1);
		break;
	}
	mvwaddch(field_win, y, x, ch | color);
}

void DrawField()
{
	for (int j = 0; j < FIELD_Y; ++j)
	{
		for (int i = 0; i < FIELD_X; ++i)
			DrawCell(game_field[j][i], j + 1, i + 1);
	}
}

void Redraw()
{
	DrawField();
	
	wrefresh(field_win);          // Print it on to the real screen
	wgetch(field_win);            // Wait for user input
}

int main()
{
	initscr();                    // Start curses mode
	raw();                        // Считывание без буферизации
	noecho();                     // Не показывать ввод
	start_color();                // Цветной вывод
	curs_set(0);                  // Спрятать курсор

	init_pair(0, COLOR_BLACK, COLOR_BLACK);
	init_pair(1, COLOR_RED, COLOR_BLACK);
	init_pair(2, COLOR_GREEN, COLOR_BLACK);
	init_pair(3, COLOR_YELLOW, COLOR_BLACK);
	init_pair(4, COLOR_BLUE, COLOR_BLACK);

	field_win = newwin(FIELD_Y+2, FIELD_X+2, FIELD_POS_Y, FIELD_POS_X);
	wborder(field_win, L'\u2551', L'\u2551', L'\u2550', L'\u2550', L'\u2554', L'\u2557', L'\u255A', L'\u255D');
	wrefresh(field_win);

	for (int j = 0; j < FIELD_Y; ++j)
		for (int i = 0; i < FIELD_X; ++i)
			game_field[j][i] = (Cell)(rand() % 5);

	Redraw();

	endwin();                     // End curses mode
	return 0;

	
}