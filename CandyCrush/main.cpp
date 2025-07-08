#include <iostream>
#include <Windows.h>
#include <conio.h>
#include <random>
#include "Player.h"

using namespace std;

constexpr int WIN_WIDTH = 70;
constexpr int WIN_HEIGHT = 60;

constexpr int MAP_WIDTH = 11;
constexpr int MAP_HEIGHT = 11;
constexpr int START_POS_X = 4;
constexpr int START_POS_Y = 1;

// Origin Map
const int ORIGIN_MAP[MAP_HEIGHT][MAP_WIDTH] =
{
	{1,1,1,1,1,1,1,1,1,1,1,},
	{1,0,0,0,0,0,0,0,0,0,1,},
	{1,0,0,0,0,0,0,0,0,0,1,},
	{1,0,0,0,0,0,0,0,0,0,1,},
	{1,0,0,0,0,0,0,0,0,0,1,},
	{1,0,0,0,0,0,0,0,0,0,1,},
	{1,0,0,0,0,0,0,0,0,0,1,},
	{1,0,0,0,0,0,0,0,0,0,1,},
	{1,0,0,0,0,0,0,0,0,0,1,},
	{1,0,0,0,0,0,0,0,0,0,1,},
	{1,1,1,1,1,1,1,1,1,1,1,},
};

// Block Data
const wchar_t* BLOCKS[] = { L"☆", L"♧", L"♤", L"♡" };
const wchar_t* CHECKBLOCKS[] = { L"★", L"♣", L"♠", L"♥" };
constexpr int BLOCK_COUNT = sizeof(BLOCKS) / sizeof(BLOCKS[0]);

// Key Code
enum eKeyCode
{
	KEY_UP = 72,
	KEY_DOWN = 80,
	KEY_LEFT = 75,
	KEY_RIGHT = 77,
	KEY_SPACE = 32,
	KEY_R = 114,
};

struct stRect
{
	int nWidth;
	int nHeight;
};

struct stConsole
{
	HANDLE hConsole;
	stRect rtConsole;
	HANDLE hBuffer[2];
	int nCurBuffer;

	// Random Seed
	random_device rdDevice;
	// Random Generation
	mt19937 rdGen;
	// Random Distribution (Block)
	uniform_int_distribution<> rdBlockDist;

	stConsole()
		: hConsole(nullptr), hBuffer{ nullptr, }, nCurBuffer(0)
		, rdGen(rdDevice()), rdBlockDist(0, BLOCK_COUNT - 1)
	{
	}
};

// Map Data
int g_nArrMap[MAP_HEIGHT][MAP_WIDTH] = {0,};
// Block Data
int* g_pCurBlock;
// Selected Block Data
int* g_pSelBlock;
// Console Data
stConsole g_console;
// Player Data
CPlayer g_player;
// Previous Player Data
CPlayer g_prevPlayerData;

void InitGame(bool bInitConsole = true)
{
	// Initialize Player Data
	{
		g_player.SetPosition(START_POS_X, START_POS_Y);
		g_player.SetXPositionRange(-1, MAP_WIDTH);
		g_player.SetYPositionRange(0, MAP_HEIGHT);
		g_player.SetBlock(0);
		g_player.SetGameScore(0);
		g_player.SetGameOver(false);

		g_prevPlayerData = g_player;
	}

	if (bInitConsole)
	{
		g_console.hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
		g_console.nCurBuffer = 0;

		CONSOLE_CURSOR_INFO consoleCursor{ 1, FALSE };
		CONSOLE_SCREEN_BUFFER_INFO consoleInfo{ 0, };
		GetConsoleScreenBufferInfo(g_console.hConsole, &consoleInfo);
		consoleInfo.dwSize.X = MAP_WIDTH;
		consoleInfo.dwSize.Y = MAP_HEIGHT;

		g_console.rtConsole.nWidth = consoleInfo.srWindow.Right - consoleInfo.srWindow.Left;
		g_console.rtConsole.nHeight = consoleInfo.srWindow.Bottom - consoleInfo.srWindow.Top;

		g_console.hBuffer[0] = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
		SetConsoleScreenBufferSize(g_console.hBuffer[0], consoleInfo.dwSize);
		SetConsoleWindowInfo(g_console.hBuffer[0], TRUE, &consoleInfo.srWindow);
		SetConsoleCursorInfo(g_console.hBuffer[0], &consoleCursor);

		g_console.hBuffer[1] = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
		SetConsoleScreenBufferSize(g_console.hBuffer[1], consoleInfo.dwSize);
		SetConsoleWindowInfo(g_console.hBuffer[1], TRUE, &consoleInfo.srWindow);
		SetConsoleCursorInfo(g_console.hBuffer[1], &consoleCursor);
	}

	// Map
	{
		int nMapSize = sizeof(int) * MAP_WIDTH * MAP_HEIGHT;
		memcpy_s(g_nArrMap, nMapSize, ORIGIN_MAP, nMapSize);

		for (int nY = 0; nY < MAP_HEIGHT; ++nY)
		{
			for (int nX = 0; nX < MAP_WIDTH; ++nX)
			{
				if (ORIGIN_MAP[nY][nX] == 0)
				{
					g_nArrMap[nY][nX] = g_console.rdBlockDist(g_console.rdGen);
				}
				else
				{
					g_nArrMap[nY][nX] = -1;
				}
			}
		}
	}
}

void Render(int nXOffset = 0, int nYOffset = 0)
{
	COORD coord{ 0, };
	DWORD dw = 0;

	for (int nY = 0; nY < MAP_HEIGHT; ++nY)
	{
		for (int nX = 0; nX < MAP_WIDTH; ++nX)
		{
			coord.X = nX * 2 + nXOffset;
			coord.Y = nY + nYOffset;
			SetConsoleCursorPosition(g_console.hBuffer[g_console.nCurBuffer], coord);

			int blockIdx = g_nArrMap[nY][nX];
			if (blockIdx >= 0 && blockIdx < BLOCK_COUNT)
			{
				COORD playerCursor = g_player.GetCursor();
				if (playerCursor.X == nX && playerCursor.Y == nY)
				{
					WriteConsoleW(g_console.hBuffer[g_console.nCurBuffer], CHECKBLOCKS[blockIdx], 1, &dw, NULL);
				}
				else
				{
					WriteConsoleW(g_console.hBuffer[g_console.nCurBuffer], BLOCKS[blockIdx], 1, &dw, NULL);
				}
			}
			else
			{
				WriteConsoleW(g_console.hBuffer[g_console.nCurBuffer], L"▣", 1, &dw, NULL);
			}
		}
	}

}

bool CheckBorder(const COORD& coordPlayer)
{
	// 맵 범위를 벗어났으면 충돌
	if (coordPlayer.X < 0 || coordPlayer.X >= MAP_WIDTH ||
		coordPlayer.Y < 0 || coordPlayer.Y >= MAP_HEIGHT)
	{
		return true;
	}

	// 벽이라면 충돌
	if (g_nArrMap[coordPlayer.Y][coordPlayer.X] == -1)
	{
		return true;
	}

	return false;
}

bool IsMoveAvailable(int nXAdder, int nYAdder)
{
	COORD coorNext = g_player.GetCursor();
	coorNext.X += nXAdder;
	coorNext.Y += nYAdder;

	return !CheckBorder(coorNext);
}

void CurrentPlayer(int x, int y) // 현재 블록 위치 표시 함수
{
	COORD playerCursor = g_player.GetCursor();
	int blockIndex = g_nArrMap[playerCursor.Y][playerCursor.X];

	if (blockIndex >= 0 && blockIndex < BLOCK_COUNT)
	{
		if (!IsMoveAvailable(x, y)) return;

		g_player.AddPosition(x, y);
		g_pCurBlock = &g_nArrMap[playerCursor.Y + y][playerCursor.X + x]; 
	}
}

void SelectBlock()
{
	if (!g_pCurBlock) return;

	g_pSelBlock = g_pCurBlock;
}

void InputKey()
{
	int nKey = 0;

	if (_kbhit() > 0)
	{
		nKey = _getch();

		switch (nKey)
		{
		case eKeyCode::KEY_UP:
		{
			if(!g_pSelBlock) CurrentPlayer(0, -1);
			else
			{
				if (!IsMoveAvailable(0, -1)) break;

				int index = g_pSelBlock - &g_nArrMap[0][0];

				int y = index / MAP_WIDTH;
				int x = index % MAP_WIDTH;

				if (y > 0)
				{
					std::swap(g_nArrMap[y][x], g_nArrMap[y - 1][x]);
					g_pCurBlock = &g_nArrMap[y][x];
					g_pSelBlock = nullptr;
				}
			}
			break;
		}
		case eKeyCode::KEY_DOWN:
		{
			if (!g_pSelBlock) CurrentPlayer(0, 1);
			else
			{
				if (!IsMoveAvailable(0, 1)) break;

				int index = g_pSelBlock - &g_nArrMap[0][0];

				int y = index / MAP_WIDTH;
				int x = index % MAP_WIDTH;

				if (y > 0 && y < MAP_HEIGHT - 1)
				{
					std::swap(g_nArrMap[y][x], g_nArrMap[y + 1][x]);
					g_pCurBlock = &g_nArrMap[y][x];
					g_pSelBlock = nullptr;
				}
			}
			break;
		}
		case eKeyCode::KEY_LEFT:
		{
			if (!g_pSelBlock) CurrentPlayer(-1, 0);
			else
			{
				if (!IsMoveAvailable(-1, 0)) break;

				int index = g_pSelBlock - &g_nArrMap[0][0];

				int y = index / MAP_WIDTH;
				int x = index % MAP_WIDTH;

				if (x > 0)
				{
					std::swap(g_nArrMap[y][x], g_nArrMap[y][x - 1]);
					g_pCurBlock = &g_nArrMap[y][x];
					g_pSelBlock = nullptr;
				}
			}
			break;
		}
		case eKeyCode::KEY_RIGHT:
		{
			if (!g_pSelBlock) CurrentPlayer(1, 0); 
			else
			{
				if (!IsMoveAvailable(1, 0)) break;

				int index = g_pSelBlock - &g_nArrMap[0][0];

				int y = index / MAP_WIDTH;
				int x = index % MAP_WIDTH;

				if (x > 0 && x < MAP_WIDTH - 1)
				{
					std::swap(g_nArrMap[y][x], g_nArrMap[y][x + 1]);
					g_pCurBlock = &g_nArrMap[y][x];
					g_pSelBlock = nullptr;
				}

			}
			break;
		}
		case eKeyCode::KEY_SPACE:
		{
			SelectBlock();
			break;
		}
		case eKeyCode::KEY_R:
		{
			InitGame(false);
			break;
		}
		default:
			break;
		}
	}
}

bool CheckThreeMatch()
{
	// 3개 이상 같은 문자가 세로/가로로 나열되었을 때
	// 세로: 2차원 배열에서 같은 열에 같은 문자가 3개 이상으로 나열되었을 때
	// 가로: 2차원 배열에서 같은 행에 같은 문자가 3개 이상으로 나열되었을 때
	// 세줄이 되더라도 선택된 블록이 코드가 다를 거라 어떻게 처리할 건지 고민해보기

	return false;
}

void ClearScreen()
{
	COORD pos = { 0, };
	DWORD dwWritten = 0;
	unsigned size = g_console.rtConsole.nWidth * g_console.rtConsole.nHeight;

	FillConsoleOutputCharacter(g_console.hConsole, L' ', size, pos, &dwWritten);
	SetConsoleCursorPosition(g_console.hConsole, pos);
}

void BufferFlip()
{
	SetConsoleActiveScreenBuffer(g_console.hBuffer[g_console.nCurBuffer]);
	g_console.nCurBuffer = g_console.nCurBuffer ? 0 : 1;
}

void DestroyGame()
{
	if (g_console.hBuffer[0] != nullptr)
	{
		CloseHandle(g_console.hBuffer[0]);
	}

	if (g_console.hBuffer[1] != nullptr)
	{
		CloseHandle(g_console.hBuffer[1]);
	}
}

int main()
{
	InitGame();

	while (true)
	{
		InputKey();
		
		Render(30, 5);

		ClearScreen();
		BufferFlip();
		Sleep(1);
	}

	DestroyGame();

	return 0;
}