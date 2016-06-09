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
#define MILLISEC_IN_TICK 500

using namespace std::chrono;

enum Cell { EMPTY, RED, GREEN, YELLOW, BLUE };
enum Status { NOT_STATIONARY, STATIONARY };

Cell game_field[FIELD_Y][FIELD_X];
Status status_map[FIELD_Y][FIELD_X];
int mark_map[FIELD_Y][FIELD_X];

WINDOW *field_win;
WINDOW *status_win;
WINDOW *mark_win;

int act1_x, act1_y, act2_x, act2_y;

bool isActive;
bool isGameOver;
bool spawnPaused = false;

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
			else if (status_map[j][i] == NOT_STATIONARY)
				mvwaddch(status_win, j + 1, i + 1, '0');
		}

	for (int j = 0; j < FIELD_Y; ++j)
		for (int i = 0; i < FIELD_X; ++i)
		{
			mvwaddch(mark_win, j + 1, i + 1, mark_map[j][i] + 48);
		}
}

void CreateCells()
{
	Cell cell1, cell2;

	act1_y = 0;      // Spawn point
	act1_x = 3;
	act2_y = 1;
	act2_x = 3;

	if ((game_field[act1_y][act1_x] == EMPTY) && (game_field[act2_y][act2_x] == EMPTY))
	{
		srand((int)time(0));
		cell1 = (Cell)(1 + rand() % NUM_OF_COLOURS);
		cell2 = (Cell)(1 + rand() % NUM_OF_COLOURS);

		game_field[act1_y][act1_x] = cell1;
		game_field[act2_y][act2_x] = cell2;

		isActive = true;
	}
	else
		isGameOver = true;
}

void CheckStationary()
{
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

void MoveCells()
{
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

		CheckStationary();
	}
	else
	{
		for (int j = FIELD_Y - 2; j > 0; --j)
			for (int i = 0; i < FIELD_X; ++i)
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

	mark_map[j][i] = group_num;

	if ((game_field[j + 1][i] == game_field[j][i]) && (mark_map[j + 1][i] == 0) && (j != FIELD_Y))
	{
		num_of_cells += CheckAdj(j + 1, i, group_num);
	}
	if ((game_field[j - 1][i] == game_field[j][i]) && (mark_map[j - 1][i] == 0) && (j != 0))
	{
		num_of_cells += CheckAdj(j - 1, i, group_num);
	}
	if ((game_field[j][i + 1] == game_field[j][i]) && (mark_map[j][i + 1] == 0) && (i != FIELD_X))
	{
		num_of_cells += CheckAdj(j, i + 1, group_num);
	}
	if ((game_field[j][i - 1] == game_field[j][i]) && (mark_map[j][i - 1] == 0) && (i != 0))
	{
		num_of_cells += CheckAdj(j, i - 1, group_num);
	}
	return num_of_cells;
}

void PopCells()
{
	int group_num;
	int num_of_cells;

	group_num = 0;
	for (int j = FIELD_Y - 1; j > -1; --j)
		for (int i = FIELD_X - 1; i > -1; --i)
			mark_map[j][i] = 0;

	for (int j = FIELD_Y - 1; j > -1; --j)
		for (int i = FIELD_X - 1; i > -1; --i)
			if ((status_map[j][i] == STATIONARY) && (game_field[j][i] != EMPTY) && (mark_map[j][i] == 0))
			{
				group_num++;
				mark_map[j][i] = group_num;
				num_of_cells = CheckAdj(j, i, group_num);

				if (num_of_cells >= GROUP_THRESHOLD)
					for (int y = FIELD_Y - 1; y > -1; --y)
						for (int x = FIELD_X - 1; x > -1; --x)
							if (mark_map[y][x] == group_num)
							{
								game_field[y][x] = EMPTY;
								for (int k = y; k > -1; --k)
								{
									status_map[k][x] = NOT_STATIONARY;
								}
								isActive = false;
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
		if ((act1_x > 0) && (act2_x > 0) && (status_map[act1_y][act1_x - 1] == EMPTY) && (status_map[act2_y][act2_x - 1] == EMPTY) && isActive)
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
		if ((act1_x < (FIELD_X - 1)) && (act2_x < (FIELD_X - 1)) && (status_map[act1_y][act1_x + 1] == EMPTY) && (status_map[act1_y][act1_x + 1] == EMPTY) && isActive)
		{
			cell1 = game_field[act1_y][act1_x];
			cell2 = game_field[act2_y][act2_x];
			game_field[act1_y][act1_x] = EMPTY;
			game_field[act2_y][act2_x] = EMPTY;
			game_field[act1_y][++act1_x] = cell1;
			game_field[act2_y][++act2_x] = cell2;
		}
		break;
	case KEY_DOWN:
		while ((status_map[act1_y + 1][act1_x] != STATIONARY) && (status_map[act2_y + 1][act2_x] != STATIONARY) &&
			(act1_y  != FIELD_Y - 1) && (act2_y != FIELD_Y - 1))
		{
			cell1 = game_field[act1_y][act1_x];
			cell2 = game_field[act2_y][act2_x];
			game_field[act1_y][act1_x] = EMPTY;
			game_field[act2_y][act2_x] = EMPTY;
			game_field[++act1_y][act1_x] = cell1;
			game_field[++act2_y][act2_x] = cell2;

			DrawCell(cell1, act1_y + 1, act1_x + 1);
			DrawCell(cell2, act2_y + 1, act2_x + 1);

			wrefresh(field_win);
		}
		break;
	case KEY_UP:
		if (isActive)
		{
			cell1 = game_field[act1_y][act1_x];
			cell2 = game_field[act2_y][act2_x];
			game_field[act1_y][act1_x] = EMPTY;
			game_field[act2_y][act2_x] = EMPTY;

			if (act1_y < act2_y)
			{
				if ((game_field[act2_y][act2_x + 1] == EMPTY) && (game_field[act2_y - 1][act2_x + 1] == EMPTY) && (act2_x != (FIELD_X - 1)))
				{
					act1_y = act2_y;
					act1_x = act2_x + 1;
				}
				else if (game_field[act2_y][act2_x - 1] == EMPTY)
				{
					act2_x--;
					act1_y++;
				}
			}
			else if (act1_y > act2_y)
			{
				if ((game_field[act2_y][act2_x - 1] == EMPTY) && (game_field[act2_y + 1][act2_x - 1] == EMPTY) && (act2_x != 0))
				{
					act1_y = act2_y;
					act1_x = act2_x - 1;
				}
				else if (game_field[act2_y][act2_x + 1] == EMPTY)
				{
					act2_x++;
					act1_y--;
				}
			}
			else if (act1_x < act2_x)
			{
				if ((game_field[act2_y - 1][act2_x] == EMPTY) && (game_field[act2_y - 1][act2_x - 1] == EMPTY) && (act2_y != 0))
				{
					act1_x = act2_x;
					act1_y = act2_y - 1;
				}
				else if (game_field[act2_y + 1][act2_x] == EMPTY)
				{
					act2_y++;
					act1_x++;
				}
			}
			else if (act1_x > act2_x)
			{
				if ((game_field[act2_y + 1][act2_x] == EMPTY) && (game_field[act2_y + 1][act2_x + 1] == EMPTY) && (act2_y != (FIELD_Y - 1)))
				{
					act1_x = act2_x;
					act1_y = act2_y + 1;
				}
				else if (game_field[act2_y - 1][act2_x] == EMPTY)
				{
					act2_y--;
					act1_x--;
				}
			}

			game_field[act1_y][act1_x] = cell1;
			game_field[act2_y][act2_x] = cell2;
		}
		break;
	case 113:  // 'q'
		exit(0);
		break;
	case 122:  // 'z'
		break;
	case 120:  // 'x'
		break;
	default:
		break;
	}
	if (isActive)
		CheckStationary();
}

void GameCycle()
{
	const milliseconds tick_dur { MILLISEC_IN_TICK };

	auto t1 = high_resolution_clock::now();
	auto t2 = t1;

	CreateCells();
	DrawField();
	wrefresh(field_win);

	while (true)
	{
		if (duration_cast<milliseconds>(t2 - t1) >= tick_dur)
		{
			PopCells();

			MoveCells();

			DrawField();

			wrefresh(field_win);
			wrefresh(status_win);
			wrefresh(mark_win);

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
				wrefresh(mark_win);

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

	mark_win = newwin(FIELD_Y + 2, FIELD_X + 2, FIELD_POS_Y, FIELD_POS_X + 2 * FIELD_X+ 20);
	wborder(mark_win, L'\u2551', L'\u2551', L'\u2550', L'\u2550', L'\u2554', L'\u2557', L'\u255A', L'\u255D');
	wrefresh(mark_win);

	GameCycle();

	wgetch(field_win);            // Wait for user input
	endwin();                     // End curses mode
	return 0;
}