#include "stdafx.h"
#include "curses.h"
#include <iostream>
#include <chrono>
#include <cstdlib>

#define FIELD_X 7
#define FIELD_Y 12
#define FIELD_POS_X 12
#define FIELD_POS_Y 10
#define GROUP_THRESHOLD 4
#define NUM_OF_COLOURS 4  // see enum Cell

using namespace std::chrono;

enum Cell { EMPTY, RED, GREEN, YELLOW, BLUE };
enum Status { NOT_STATIONARY, STATIONARY };

Cell game_field[FIELD_Y][FIELD_X];
Status status_map[FIELD_Y][FIELD_X];
int mark_map[FIELD_Y][FIELD_X];

WINDOW *field_win;
WINDOW *status_win;

int act1_x, act1_y, act2_x, act2_y;

bool isActive;
bool isVertical;

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
		for (int i = 0; i < FIELD_X; ++i)
			DrawCell(game_field[j][i], j + 1, i + 1);

	for (int j = 0; j < FIELD_Y; ++j)
		for (int i = 0; i < FIELD_X; ++i)
		{
			if (status_map[j][i] == STATIONARY)
				mvwaddch(status_win, j + 1, i + 1, '+');
			if (status_map[j][i] == NOT_STATIONARY)
				mvwaddch(status_win, j + 1, i + 1, '0');
		}
}

void CreateCells()
{
	int x;
	Cell cell1, cell2;

	srand((int)time(0));
	x = rand() % FIELD_X;
	cell1 = (Cell)(1 + rand() % NUM_OF_COLOURS);
	cell2 = (Cell)(1 + rand() % NUM_OF_COLOURS);

	act1_y = 0;
	act1_x = x;
	act2_y = 1;
	act2_x = x;

	game_field[act1_y][act1_x] = cell1;
	game_field[act2_y][act2_x] = cell2;

	isActive = true;
	isVertical = true;
}

void MoveCells()
{
	/*bool hasChanged = false;

	if (isActive)
	{
		if (isVertical)
		{
			if (act1_y > act2_y)                                             // Vertical, act1 lower than act2
			{
				game_field[act1_y + 1][act1_x] = game_field[act1_y][act1_x];
				game_field[act1_y][act1_x] = game_field[act1_y - 1][act1_x];
				if ((game_field[act1_y + 2][act1_x] != EMPTY) || ((act1_y + 1) == FIELD_Y - 1))
					isActive = false;
			}
			else                                                             // Vertical, act2 lower than act1
			{
				game_field[act2_y + 1][act1_x] = game_field[act2_y][act1_x];
				game_field[act2_y][act2_x] = game_field[act2_y - 1][act2_x];
				if ((game_field[act2_y + 2][act2_x] != EMPTY) || ((act2_y + 1) == FIELD_Y - 1))
					isActive = false;
			}
			if (!isActive)
			{
				status_map[act1_y][act1_x] = NOT_STATIONARY;
				status_map[act2_y][act2_x] = NOT_STATIONARY;
				status_map[act1_y + 1][act1_x] = STATIONARY;
				status_map[act2_y + 1][act2_x] = STATIONARY;
			}
		}
		else                                                                 // Horizontal
		{
			game_field[act1_y + 1][act1_x] = game_field[act1_y][act1_x];
			game_field[act2_y + 1][act1_x] = game_field[act2_y][act1_x];
			if ((game_field[act1_y + 2][act1_x] != EMPTY) || ((act1_y + 1) == FIELD_Y - 1))
			{
				isActive = false;
				status_map[act1_y][act1_x] = NOT_STATIONARY;
				status_map[act1_y + 1][act1_x] = STATIONARY;
			}
			if ((game_field[act2_y + 2][act2_x] != EMPTY) || ((act2_y + 1) == FIELD_Y - 1))
			{
				isActive = false;
				status_map[act2_y][act2_x] = NOT_STATIONARY;
				status_map[act2_y + 1][act2_x] = STATIONARY;
			}
		}

		act1_y++;
		act2_y++;
	}
	else
	{
		for (int j = FIELD_Y - 1; j > 0; --j)
			for (int i = FIELD_X - 1; i > -1; --i)
				if ((game_field[j][i] == EMPTY) && (game_field[j - 1][i] != EMPTY))
				{
					game_field[j][i] = game_field[j - 1][i];
					if ((status_map[j + 1][i] == STATIONARY) || (j == (FIELD_Y - 1)))
						status_map[j][i] = STATIONARY;

					game_field[j - 1][i] = EMPTY;
					status_map[j - 1][i] = NOT_STATIONARY;

					hasChanged = true;
				}
		if (!hasChanged)
			CreateCells();
	}*/

	bool hasMoved = false;
	Cell cell1, cell2;

	if (isActive)
	{
		cell1 = game_field[act1_y][act1_x];
		cell2 = game_field[act2_y][act2_x];
		game_field[act1_y][act1_x] = EMPTY;
		game_field[act2_y][act2_x] = EMPTY;
		game_field[++act1_y][act1_x] = cell1;
		game_field[++act2_y][act2_x] = cell2;

		if ((status_map[act1_y + 1][act1_x] == STATIONARY) || (act1_y == FIELD_Y - 1))
		{
			status_map[act1_y][act1_x] = STATIONARY;
			if (status_map[act2_y + 1][act2_x] == STATIONARY)
				status_map[act2_y][act2_x] = STATIONARY;
			isActive = false;
		}
		if ((status_map[act2_y + 1][act2_x] == STATIONARY) || (act2_y == FIELD_Y - 1))
		{
			status_map[act2_y][act2_x] = STATIONARY;
			if (status_map[act1_y + 1][act1_x] == STATIONARY)
				status_map[act1_y][act1_x] = STATIONARY;
			isActive = false;
		}
	}
	else
	{
		for (int j = FIELD_Y - 2; j > 0; --j)
			for (int i = 0; i < FIELD_X - 1; ++i)
				if ((game_field[j][i] != EMPTY) && (game_field[j + 1][i] == EMPTY))
				{
					game_field[j + 1][i] = game_field[j][i];
					game_field[j][i] = EMPTY;

					if ((status_map[j + 2][i] == STATIONARY) || (j  == (FIELD_Y - 2)))
						status_map[j + 1][i] = STATIONARY;

					hasMoved = true;
				}
		if (!hasMoved)
			CreateCells();
	}
}

int CheckAdj(int j, int i, int group_num)
{
	int num_of_cells = 1;

	if ((mark_map[j + 1][i] == 0) && (j != FIELD_Y))
	{
		num_of_cells += CheckAdj(j + 1, i, group_num);
	}
	if ((mark_map[j - 1][i] == 0) && (j != 0))
	{
		num_of_cells += CheckAdj(j - 1, i, group_num);
	}
	if ((mark_map[j][i + 1] == 0) && (i != FIELD_X))
	{
		num_of_cells += CheckAdj(j, i + 1, group_num);
	}
	if ((mark_map[j][i - 1] == 0) && (i != 0))
	{
		num_of_cells += CheckAdj(j, i - 1, group_num);
	}
	return num_of_cells;
}

void PopCells()
{
	int group_num;
	int num_of_cells;

	if (!isActive)
	{
		group_num = 0;
		for (int j = FIELD_Y - 1; j > -1; --j)
			for (int i = FIELD_X - 1; i > -1; --i)
				mark_map[j][i] = 0;

		for (int j = FIELD_Y - 1; j > -1; --j)
			for (int i = FIELD_X - 1; i > -1; --i)
				if ((status_map[j][i] == STATIONARY) && (mark_map[j][i] = 0))
				{
					group_num++;
					num_of_cells = CheckAdj(j, i, group_num);

					if (num_of_cells >= GROUP_THRESHOLD)
						for (int j = FIELD_Y - 1; j > -1; --j)
							for (int i = FIELD_X - 1; i > -1; --i)
								if (mark_map[j][i] == group_num)
								{
									game_field[j][i] = EMPTY;
									status_map[j][i] = NOT_STATIONARY;
									status_map[j - 1][i] = NOT_STATIONARY;
								}
				}
	}
}

void Input()
{
	int ch;
	Cell cell1, cell2;

	ch = wgetch(field_win);

	switch (ch)
	{
	case KEY_LEFT:
		if ((act1_x > 0) && (act2_x > 0) && isActive)
		{
			cell1 = game_field[act1_y][act1_x];
			cell2 = game_field[act2_y][act2_x];
			game_field[act1_y][act1_x] = EMPTY;
			game_field[act2_y][act2_x] = EMPTY;
			game_field[act1_y][--act1_x] = cell1;
			game_field[act2_y][--act2_x] = cell2;
		}
		break;
	case KEY_RIGHT:
		if ((act1_x < (FIELD_X - 1)) && (act2_x < (FIELD_X - 1)) && isActive)
		{
			cell1 = game_field[act1_y][act1_x];
			cell2 = game_field[act2_y][act2_x];
			game_field[act1_y][act1_x] = EMPTY;
			game_field[act2_y][act2_x] = EMPTY;
			game_field[act1_y][++act1_x] = cell1;
			game_field[act2_y][++act2_x] = cell2;
		}
		break;
	/*case KEY_DOWN:
		cell1 = game_field[act1_y][act1_x];
		cell2 = game_field[act2_y][act2_x];
		game_field[act1_y][act1_x] = EMPTY;
		game_field[act2_y][act2_x] = EMPTY;
		for (int j = 0; j < FIELD_Y - 1; ++j)
			if (game_field[j][act])
		break;*/
	case KEY_UP:
		break;
	case 113:  // 'q'
		exit(0);
		break;
	default:
		break;
	}

}

void GameCycle()
{
	const milliseconds tick_dur { 500 };

	auto t1 = high_resolution_clock::now();
	auto t2 = t1;

	CreateCells();
	DrawField();
	wrefresh(field_win);

	while (true)
	{
		if (duration_cast<milliseconds>(t2 - t1) >= tick_dur)
		{
			MoveCells();

			//PopCells();

			DrawField();

			wrefresh(field_win);
			wrefresh(status_win);

			t1 = high_resolution_clock::now();
			t2 = t1;
		}
		else
			while (duration_cast<milliseconds>(t2 - t1) < tick_dur)
			{			
				Input();

				DrawField();

				wrefresh(field_win);
				wrefresh(status_win);

				t2 = high_resolution_clock::now();
			}
	}
}

int main()
{
	initscr();                    // Start curses mode
	cbreak();                     // Считывание без буферизации
	noecho();                     // Не показывать ввод
	start_color();                // Цветной вывод
	curs_set(0);                  // Спрятать курсор

	init_pair(0, COLOR_BLACK, COLOR_BLACK);
	init_pair(1, COLOR_RED, COLOR_BLACK);
	init_pair(2, COLOR_GREEN, COLOR_BLACK);
	init_pair(3, COLOR_YELLOW, COLOR_BLACK);
	init_pair(4, COLOR_BLUE, COLOR_BLACK);

	field_win = newwin(FIELD_Y + 2, FIELD_X + 2, FIELD_POS_Y, FIELD_POS_X);
	wborder(field_win, L'\u2551', L'\u2551', L'\u2550', L'\u2550', L'\u2554', L'\u2557', L'\u255A', L'\u255D');
	wrefresh(field_win);
	nodelay(field_win, true);
	keypad(field_win, true);

	status_win = newwin(FIELD_Y + 2, FIELD_X + 2, FIELD_POS_Y, FIELD_POS_X + FIELD_X + 10);
	wborder(status_win, L'\u2551', L'\u2551', L'\u2550', L'\u2550', L'\u2554', L'\u2557', L'\u255A', L'\u255D');
	wrefresh(status_win);
	for (int j = FIELD_Y - 1; j > -1; --j)
		for (int i = FIELD_X - 1; i > -1; --i)
			status_map[j][i] = NOT_STATIONARY;

	GameCycle();

	wgetch(field_win);            // Wait for user input
	endwin();                     // End curses mode
	return 0;

	
}