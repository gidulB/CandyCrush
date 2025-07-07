#include <iostream>
#include <Windows.h>
#include <conio.h>
#include "Player.h"

using namespace std;

constexpr int WIN_WIDTH = 70;
constexpr int WIN_HEIGHT = 60;

constexpr int MAP_WIDTH = 12;
constexpr int MAP_HEIGHT = 24;
constexpr int START_POS_X = 4;
constexpr int START_POS_Y = 1;

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

	stConsole()
		: hConsole(nullptr), hBuffer{ nullptr, }, nCurBuffer(0)
	{}
};

// Origin Map
const int ORIGIN_MAP[MAP_HEIGHT][MAP_WIDTH] =
{
	{1,1,1,1,1,1,1,1,1,1,1,1,},
	{1,0,0,0,0,0,0,0,0,0,0,1,},
	{1,0,0,0,0,0,0,0,0,0,0,1,},
	{1,0,0,0,0,0,0,0,0,0,0,1,},
	{1,0,0,0,0,0,0,0,0,0,0,1,},
	{1,0,0,0,0,0,0,0,0,0,0,1,},
	{1,0,0,0,0,0,0,0,0,0,0,1,},
	{1,0,0,0,0,0,0,0,0,0,0,1,},
	{1,0,0,0,0,0,0,0,0,0,0,1,},
	{1,0,0,0,0,0,0,0,0,0,0,1,},
	{1,0,0,0,0,0,0,0,0,0,0,1,},
	{1,0,0,0,0,0,0,0,0,0,0,1,},
	{1,0,0,0,0,0,0,0,0,0,0,1,},
	{1,0,0,0,0,0,0,0,0,0,0,1,},
	{1,0,0,0,0,0,0,0,0,0,0,1,},
	{1,0,0,0,0,0,0,0,0,0,0,1,},
	{1,0,0,0,0,0,0,0,0,0,0,1,},
	{1,0,0,0,0,0,0,0,0,0,0,1,},
	{1,0,0,0,0,0,0,0,0,0,0,1,},
	{1,0,0,0,0,0,0,0,0,0,0,1,},
	{1,0,0,0,0,0,0,0,0,0,0,1,},
	{1,1,1,1,1,1,1,1,1,1,1,1,},
};

// Block Data
const int BLOCKS[]		= { '☆', '♧', '◇', '○', '△', '▽', '♤', '♡' };		
const int CHECKBLOCKS[] = { '★', '♣', '◆', '●', '▲', '▼', '♠', '♥' };		

// Block Type
const char BLOCK_TYPES[][4] =
{
	"  ",
	"▣",
};

// Map Data
int g_nArrMap[MAP_HEIGHT][MAP_WIDTH] = {0,};
// Block Data
int* g_pCurBlock;
// Selected Block Data
int* g_pSelBlock;
// Console Data
stConsole g_Console;
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
		//g_player.SetBlock(RandomBlock());
		g_player.SetCheckBlock(CPlayer::eCheckBlock::Check0);
		g_player.SetGameScore(0);
		g_player.SetGameOver(false);

		g_prevPlayerData = g_player;
	}

	if (bInitConsole)
	{
		g_Console.hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
		g_Console.nCurBuffer = 0;

		CONSOLE_CURSOR_INFO consoleCursor{ 0, FALSE };
		CONSOLE_SCREEN_BUFFER_INFO consoleInfo{ 0, };
		GetConsoleScreenBufferInfo(g_Console.hBuffer, &consoleInfo);
		consoleInfo.dwSize.X = 40;
		consoleInfo.dwSize.Y = 30;

		g_Console.rtConsole.nWidth = consoleInfo.srWindow.Right - consoleInfo.srWindow.Left;
		g_Console.rtConsole.nHeight = consoleInfo.srWindow.Bottom - consoleInfo.srWindow.Top;

		g_Console.hBuffer[0] = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
		SetConsoleScreenBufferSize(g_Console.hBuffer[0], consoleInfo.dwSize);
		SetConsoleWindowInfo(g_Console.hBuffer[0], TRUE, &consoleInfo.srWindow);
		SetConsoleCursorInfo(g_Console.hBuffer[0], &consoleCursor);

		g_Console.hBuffer[1] = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
		SetConsoleScreenBufferSize(g_Console.hBuffer[1], consoleInfo.dwSize);
		SetConsoleWindowInfo(g_Console.hBuffer[1], TRUE, &consoleInfo.srWindow);
		SetConsoleCursorInfo(g_Console.hBuffer[1], &consoleCursor);
	}
}

void Render(int nXOffset = 0, int nYOffset = 0)
{
	COORD coord{ 0, };
	int nXAdd = 0;
	DWORD dw = 0;
	char chBuf[256] = { 0, };

	for (int nY = 0; nY < MAP_HEIGHT; ++nY)
	{
		nXAdd = 0;
		for (int nX = 0; nX < MAP_WIDTH; ++nX)
		{
			coord.X = nXAdd + nXOffset;
			coord.Y = nY + nYOffset;

			SetConsoleCursorPosition(g_Console.hBuffer[g_Console.nCurBuffer], coord);
			WriteFile(g_Console.hBuffer[g_Console.nCurBuffer], BLOCK_TYPES[g_nArrMap[nY][nX]], sizeof(BLOCK_TYPES[g_nArrMap[nY][nX]]), &dw, NULL);

			++nXAdd;
		}
	}
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
			if(!g_pSelBlock) CalcPlayer();
			
			int index = g_pSelBlock - &g_nArrMap[0][0];

			int y = index / MAP_WIDTH;
			int x = index % MAP_WIDTH;

			if (y > 0)
			{
				std::swap(g_nArrMap[y][x], g_nArrMap[y - 1][x]);				
				g_pSelBlock = &g_nArrMap[y - 1][x];
			}

			break;
		}
		case eKeyCode::KEY_DOWN:
		{
			if (!g_pSelBlock) CalcPlayer();

			int index = g_pSelBlock - &g_nArrMap[0][0];

			int y = index / MAP_WIDTH;
			int x = index % MAP_WIDTH;

			if (y > 0)
			{
				std::swap(g_nArrMap[y][x], g_nArrMap[y - 1][x]);
				g_pSelBlock = &g_nArrMap[y + 1][x];
			}

			break;
		}
		case eKeyCode::KEY_LEFT:
		{
			if (!g_pSelBlock) CalcPlayer();

			int index = g_pSelBlock - &g_nArrMap[0][0];

			int y = index / MAP_WIDTH;
			int x = index % MAP_WIDTH;

			if (x > 0)
			{
				std::swap(g_nArrMap[y][x], g_nArrMap[y][x - 1]);
				g_pSelBlock = &g_nArrMap[y][x - 1];
			}

			break;
		}
		case eKeyCode::KEY_RIGHT:
		{
			if (!g_pSelBlock) CalcPlayer();

			int index = g_pSelBlock - &g_nArrMap[0][0];

			int y = index / MAP_WIDTH;
			int x = index % MAP_WIDTH;

			if (x > 0)
			{
				std::swap(g_nArrMap[y][x], g_nArrMap[y][x + 1]);
				g_pSelBlock = &g_nArrMap[y][x + 1];
			}

			break;
		}
		case eKeyCode::KEY_SPACE:
		{
			*g_pSelBlock = *g_pCurBlock;
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

void CalcPlayer() // 현재 블록 위치 표시 함수
{
	COORD playerCursor = g_player.GetCursor();
	int BlockValue = g_nArrMap[playerCursor.Y][playerCursor.X];
	for (int i = 0; i < sizeof(BLOCKS); ++i)
	{
		if (BlockValue == BLOCKS[i])
		{
			*g_pCurBlock = CHECKBLOCKS[i];
			/*g_player.SetCheckBlock(CPlayer::eCheckBlock::Check1);
			g_prevPlayerData.SetCheckBlock(CPlayer::eCheckBlock::Check0);*/
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
	unsigned size = g_Console.rtConsole.nWidth * g_Console.rtConsole.nHeight;

	FillConsoleOutputCharacter(g_Console.hConsole, ' ', size, pos, &dwWritten);
	SetConsoleCursorPosition(g_Console.hConsole, pos);
}

void BufferFlip()
{
	SetConsoleActiveScreenBuffer(g_Console.hBuffer[g_Console.nCurBuffer]);
	g_Console.nCurBuffer = g_Console.nCurBuffer ? 0 : 1;
}

void DestroyGame()
{
	if (g_Console.hBuffer[0] != nullptr)
	{
		CloseHandle(g_Console.hBuffer[0]);
	}

	if (g_Console.hBuffer[1] != nullptr)
	{
		CloseHandle(g_Console.hBuffer[1]);
	}
}

int main()
{
	InitGame();

	while (true)
	{
		InputKey();
		CalcPlayer();

		//CheckBottom();
		//Render(3, 1);

		ClearScreen();
		BufferFlip();
		Sleep(1);
	}

	DestroyGame();

	return 0;
}