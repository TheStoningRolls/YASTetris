#include "stdafx.h"
#include "curses.h"
#include <iostream>
#include <chrono>
#include <cstdlib>

#define FIELD_X 6
#define FIELD_Y 12
#define FIELD_POS_X 12
#define FIELD_POS_Y 10
#define GROUP_THRESHOLD 4
#define NUM_OF_COLOURS 4  // see enum Cell

using namespace std::chrono;

enum Cell { EMPTY, RED, GREEN, YELLOW, BLUE };
enum Status { FALLEN, NOT_FALLEN };

Cell game_field[FIELD_Y][FIELD_X];
Status status_map[FIELD_Y][FIELD_X];
int mark_map[FIELD_Y][FIELD_X];

WINDOW *field_win;

int act1_x, act1_y, act2_x, act2_y;

bool isActive = false;
bool isVertical = true;

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

void CreateCells()
{
	int x;
	Cell cell1, cell2;

	srand((int)time(0));
	x = 1 + rand() % (FIELD_X - 1);
	cell1 = (Cell)(1 + rand() % (NUM_OF_COLOURS - 1));
	cell2 = (Cell)(1 + rand() % (NUM_OF_COLOURS - 1));

	game_field[0][x] = cell1;
	game_field[1][x] = cell2;

	isActive = true;
}

void MoveCells()
{
	bool hasChanged = false;

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
				status_map[act1_y][act1_x] = NOT_FALLEN;
				status_map[act2_y][act2_x] = NOT_FALLEN;
				status_map[act1_y + 1][act1_x] = FALLEN;
				status_map[act2_y + 1][act2_x] = FALLEN;
			}
		}
		else                                                                 // Horizontal
		{
			game_field[act1_y + 1][act1_x] = game_field[act1_y][act1_x];
			game_field[act2_y + 1][act1_x] = game_field[act2_y][act1_x];
			if ((game_field[act1_y + 2][act1_x] != EMPTY) || ((act1_y + 1) == FIELD_Y - 1))
			{
				isActive = false;
				status_map[act1_y][act1_x] = NOT_FALLEN;
				status_map[act1_y + 1][act1_x] = FALLEN;
			}
			if ((game_field[act2_y + 2][act2_x] != EMPTY) || ((act2_y + 1) == FIELD_Y - 1))
			{
				isActive = false;
				status_map[act2_y][act2_x] = NOT_FALLEN;
				status_map[act2_y + 1][act2_x] = FALLEN;
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
					if ((status_map[j + 1][i] == FALLEN) || (j == (FIELD_Y - 1)))
						status_map[j][i] = FALLEN;

					game_field[j - 1][i] = EMPTY;
					status_map[j - 1][i] = NOT_FALLEN;

					hasChanged = true;
				}
		if (!hasChanged)
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
				if ((status_map[j][i] == FALLEN) && (mark_map[j][i] = 0))
				{
					group_num++;
					num_of_cells = CheckAdj(j, i, group_num);

					if (num_of_cells >= GROUP_THRESHOLD)
						for (int j = FIELD_Y - 1; j > -1; --j)
							for (int i = FIELD_X - 1; i > -1; --i)
								if (mark_map[j][i] == group_num)
								{
									game_field[j][i] = EMPTY;
									status_map[j][i] = NOT_FALLEN;
									status_map[j - 1][i] = NOT_FALLEN;
								}
				}
	}
}

void GameCycle()
{
	int k = 0;

	const milliseconds tick_dur { 500 };

	auto t1 = high_resolution_clock::now();
	auto t2 = t1;
	int ch;

	CreateCells();
	DrawField();
	wrefresh(field_win);

	while (true)
	{
		if (duration_cast<milliseconds>(t2 - t1) >= tick_dur)
		{
			MoveCells();

			PopCells();

			DrawField();

			wrefresh(field_win);

			t1 = high_resolution_clock::now();
			t2 = t1;
		}
		else
			while (duration_cast<milliseconds>(t2 - t1) < tick_dur)
			{
				//k++;
				//mvwaddch(field_win, 1, 1, k);
				/*
				ch = wgetch(field_win);

				switch (ch)
				{
				case KEY_LEFT:
					break;
				case KEY_RIGHT:
					break;
				case KEY_DOWN:
					break;
				case KEY_UP: 
					break;
				case 113:  // 'q'
					exit(0);
					break;
				default:
					break;
				}
				*/

				t2 = high_resolution_clock::now();
			}
	}
}

int main()
{
	initscr();                    // Start curses mode
	cbreak();                     // ���������� ��� �����������
	noecho();                     // �� ���������� ����
	start_color();                // ������� �����
	curs_set(0);                  // �������� ������

	init_pair(0, COLOR_BLACK, COLOR_BLACK);
	init_pair(1, COLOR_RED, COLOR_BLACK);
	init_pair(2, COLOR_GREEN, COLOR_BLACK);
	init_pair(3, COLOR_YELLOW, COLOR_BLACK);
	init_pair(4, COLOR_BLUE, COLOR_BLACK);

	field_win = newwin(FIELD_Y+2, FIELD_X+2, FIELD_POS_Y, FIELD_POS_X);
	wborder(field_win, L'\u2551', L'\u2551', L'\u2550', L'\u2550', L'\u2554', L'\u2557', L'\u255A', L'\u255D');
	wrefresh(field_win);
	nodelay(field_win, true);

	GameCycle();

	wgetch(field_win);            // Wait for user input
	endwin();                     // End curses mode
	return 0;

	
}