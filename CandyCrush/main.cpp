#include <iostream>
#include <Windows.h>
#include <conio.h>

using namespace std;

constexpr int WIN_WIDTH = 70;
constexpr int WIN_HEIGHT = 60;

constexpr int MAP_WIDTH = 12;
constexpr int MAP_HEIGHT = 24;
constexpr int START_POSITION_X = 4;
constexpr int START_POSITION_Y = 1;

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

// Map Data
int g_nArrMap[MAP_HEIGHT][MAP_WIDTH] = {0,};

stConsole g_Console;

void InitGame(bool bInitConsole = true)
{
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

void InputKey()
{
	int nKey = 0;

	if (_kbhit() > 0)
	{
		nKey = _getch();

		switch (nKey)
		{
		case eKeyCode::KEY_UP:
			break;
		case eKeyCode::KEY_DOWN:
			break;
		case eKeyCode::KEY_LEFT:
			break;
		case eKeyCode::KEY_RIGHT:
			break;
		case eKeyCode::KEY_SPACE:
			break;
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
		//CalcPlayer();

		//CheckBottom();
		//Render(3, 1);

		ClearScreen();
		BufferFlip();
		Sleep(1);
	}

	DestroyGame();

	return 0;
}