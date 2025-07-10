#include <iostream>
#include <Windows.h>
#include <conio.h>
#include <random>
#include "Player.h"

// @TODO
// 1. 3X3 이상이면 삭제 -> 해결
// 2. 삭제되면 점수 추가 -> 해결
// 3. 빈 공간 생기면 채워넣기 -> 해결이긴 한데 수정해야 함
// 4. 생성되는 시점부터 3X3이면 자동으로 삭제 -> 삭제가 이상함 수정해야 함
// 5. InputKey로 위치 바꾸면 잠시 CHECKBLOCK 해제
// 6. 맞으면 1,2,3 진행 후 다시 g_pCurBlock에 CHECKBLOCK 생성
// 7. 틀리면 다시 위치 원래대로 변경 및 g_pCurBlock에 CHECKBLOCK 생성 -> 해결이긴 한데 눈에 안 보임
// + 제한시간을 넣을까 말까 -> 넣음 해결

using namespace std;

constexpr int WIN_WIDTH = 70;
constexpr int WIN_HEIGHT = 60;

constexpr int MAP_WIDTH = 11;
constexpr int MAP_HEIGHT = 11;
constexpr int START_POS_X = 4;
constexpr int START_POS_Y = 1;

constexpr int GAME_TIME_LIMIT = 60;

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

// Block Type
const wchar_t* BLOCK_TYPES[] = { L"  ", L"▣", };

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

	// Clock
	clock_t timeStart;

	stConsole()
		: hConsole(nullptr), hBuffer{ nullptr, }, nCurBuffer(0)
		, rdGen(rdDevice()), rdBlockDist(0, BLOCK_COUNT - 1)
		, timeStart(clock())
	{
	}
};

// Map Data
int g_nArrMap[MAP_HEIGHT][MAP_WIDTH] = {0,};
// Match Flag
bool matchFlags[MAP_HEIGHT][MAP_WIDTH] = { false };
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
// Clock
clock_t g_gameStartTime;

bool IsBlockMatch()
{
	bool bMatched = false;

	// 가로 방향 검사
	for (int nY = 0; nY < MAP_HEIGHT; ++nY)
	{
		int nCount = 1;
		for (int nX = 1; nX < MAP_WIDTH; ++nX)
		{
			int nCurr = g_nArrMap[nY][nX];
			int nPrev = g_nArrMap[nY][nX - 1];

			if (nCurr >= 0 && nCurr == nPrev)
			{
				nCount++;
			}
			else
			{
				if (nCount >= 3)
				{
					for (int nK = 0; nK < nCount; ++nK)
					{
						matchFlags[nY][nX - 1 - nK] = true;
					}
						
					bMatched = true;
				}
				nCount = 1;
			}
		}
		if (nCount >= 3)
		{
			for (int nK = 0; nK < nCount; ++nK)
			{
				matchFlags[nY][MAP_WIDTH - 1 - nK] = true;
			}
				
			bMatched = true;
		}
	}

	// 세로 방향 검사
	for (int nX = 0; nX < MAP_WIDTH; ++nX)
	{
		int nCount = 1;
		for (int nY = 1; nY < MAP_HEIGHT; ++nY)
		{
			int nCurr = g_nArrMap[nY][nX];
			int nPrev = g_nArrMap[nY - 1][nX];

			if (nCurr >= 0 && nCurr == nPrev)
			{
				nCount++;
			}
			else
			{
				if (nCount >= 3)
				{
					for (int nK = 0; nK < nCount; ++nK)
					{
						matchFlags[nY - 1 - nK][nX] = true;
					}
						
					bMatched = true;
				}
				nCount = 1;
			}
		}
		if (nCount >= 3)
		{
			for (int nK = 0; nK < nCount; ++nK)
			{
				matchFlags[MAP_HEIGHT - 1 - nK][nX] = true;
			}
				
			bMatched = true;
		}
	}

	return bMatched;
}


void InitGame(bool bInitConsole = true)
{
	// Initialize Player Data
	{
		g_player.SetPosition(START_POS_X, START_POS_Y);
		g_player.SetXPositionRange(-1, MAP_WIDTH);
		g_player.SetYPositionRange(0, MAP_HEIGHT);
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
		// 맵 초기화
		while (true)
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
						g_nArrMap[nY][nX] = -2;
					}
				}
			}

			// 블록 매칭이 없으면 루프 탈출
			memset(matchFlags, false, sizeof(matchFlags));
			if (!IsBlockMatch()) break;
		}
	}

	// time
	{
		g_gameStartTime = clock();

		COORD coord = { 60, 10 };
		DWORD dw;
		for (int i = 0; i < 2; ++i)
		{
			FillConsoleOutputCharacterW(g_console.hBuffer[i], L' ', 10, coord, &dw); // GameOver 초기화
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
			else if (blockIdx == -1)
			{
				WriteConsoleW(g_console.hBuffer[g_console.nCurBuffer], BLOCK_TYPES[0], wcslen(BLOCK_TYPES[0]), &dw, NULL); // 공백
			}
			else if (blockIdx == -2)
			{
				WriteConsoleW(g_console.hBuffer[g_console.nCurBuffer], BLOCK_TYPES[1], wcslen(BLOCK_TYPES[1]), &dw, NULL); // 벽
			}

		}
	}

	// Score
	{
		coord.X = 30 + nXOffset;
		coord.Y = 0 + nYOffset;
		wchar_t wchBuf[256];
		swprintf_s(wchBuf, L"SCORE : %6d", g_player.GetGameScore());
		SetConsoleCursorPosition(g_console.hBuffer[g_console.nCurBuffer], coord);
		WriteConsoleW(g_console.hBuffer[g_console.nCurBuffer], wchBuf, wcslen(wchBuf), &dw, NULL);
	}

	// Time and GameOver
	{
		int elapsedTime = static_cast<int>((clock() - g_gameStartTime) / CLOCKS_PER_SEC);
		int remainingTime = GAME_TIME_LIMIT - elapsedTime;

		if (remainingTime < 0)
		{
			const wchar_t* msg = L"TIME OVER!";
			coord.X = 30 + nXOffset;
			coord.Y = 5 + nYOffset;
			DWORD dw = 0;

			SetConsoleCursorPosition(g_console.hBuffer[g_console.nCurBuffer], coord);
			WriteConsoleW(g_console.hBuffer[g_console.nCurBuffer], msg, wcslen(msg), &dw, NULL);

			g_player.SetGameOver(true);
		}
		else
		{
			wchar_t timerBuf[32];
			swprintf_s(timerBuf, L"TIME: %2d", remainingTime);

			coord.X = 30 + nXOffset;
			coord.Y = 10 + nYOffset;
			SetConsoleCursorPosition(g_console.hBuffer[g_console.nCurBuffer], coord);
			WriteConsoleW(g_console.hBuffer[g_console.nCurBuffer], timerBuf, wcslen(timerBuf), &dw, NULL);
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
	if (g_nArrMap[coordPlayer.Y][coordPlayer.X] == -1 ||
		g_nArrMap[coordPlayer.Y][coordPlayer.X] == -2)
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

void Swap(int x, int y)
{
	int index = g_pSelBlock - &g_nArrMap[0][0];

	int nY = index / MAP_WIDTH;
	int nX = index % MAP_WIDTH;

	if (nY > 0)
	{
		std::swap(g_nArrMap[nY][nX], g_nArrMap[nY + y][nX + x]);
		
		if (!IsBlockMatch())
		{
			std::swap(g_nArrMap[nY][nX], g_nArrMap[nY + y][nX + x]);
		}		

		g_pCurBlock = &g_nArrMap[nY][nX];
		g_pSelBlock = nullptr;
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
			if (g_player.GetGameOver()) break;

			if(!g_pSelBlock) CurrentPlayer(0, -1);
			else
			{
				if (!IsMoveAvailable(0, -1)) break;
				Swap(0, -1);
			}
			break;
		}
		case eKeyCode::KEY_DOWN:
		{
			if (g_player.GetGameOver()) break;

			if (!g_pSelBlock) CurrentPlayer(0, 1);
			else
			{
				if (!IsMoveAvailable(0, 1)) break;
				Swap(0, 1);
			}
			break;
		}
		case eKeyCode::KEY_LEFT:
		{
			if (g_player.GetGameOver()) break;

			if (!g_pSelBlock) CurrentPlayer(-1, 0);
			else
			{
				if (!IsMoveAvailable(-1, 0)) break;
				Swap(-1, 0);
			}
			break;
		}
		case eKeyCode::KEY_RIGHT:
		{
			if (g_player.GetGameOver()) break;

			if (!g_pSelBlock) CurrentPlayer(1, 0); 
			else
			{
				if (!IsMoveAvailable(1, 0)) break;
				Swap(1, 0);
			}
			break;
		}
		case eKeyCode::KEY_SPACE:
		{
			if (g_player.GetGameOver()) break;

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

void PushCharDown()
{
	for (int x = 0; x < MAP_WIDTH; ++x)
	{
		int writeY = MAP_HEIGHT - 1;

		while (writeY >= 0 && g_nArrMap[writeY][x] == -2)
			writeY--;

		for (int y = writeY; y >= 0; --y)
		{
			if (g_nArrMap[y][x] >= 0 && g_nArrMap[y][x] < BLOCK_COUNT)
			{
				if (writeY != y)
				{
					g_nArrMap[writeY][x] = g_nArrMap[y][x];
					g_nArrMap[y][x] = -1;
				}
				writeY--;
				
				while (writeY >= 0 && g_nArrMap[writeY][x] == -2)
					writeY--;
			}
		}
	}
}

void FillBlank()
{	
	for (int y = 0; y < MAP_HEIGHT; ++y)
	{
		for (int x = 0; x < MAP_WIDTH; ++x)
		{
			if (g_nArrMap[y][x] == -1)
			{
				// 빈칸을 랜덤 블록으로 채우기
				g_nArrMap[y][x] = g_console.rdBlockDist(g_console.rdGen);
			}
		}
	}
}

int DeleteLine()
{
	int deleteCount = 0;

	for (int nY = 0; nY < MAP_HEIGHT; ++nY)
	{
		for (int nX = 0; nX < MAP_WIDTH; ++nX)
		{
			if (matchFlags[nY][nX])
			{
				g_nArrMap[nY][nX] = -1;
				deleteCount++;
			}
		}
	}

	// 점수 추가
	if (deleteCount > 0)
	{
		g_player.AddGameScore(deleteCount);
	}

	return deleteCount;
}

void CheckBlockLine()
{	
	while (true)
	{
		memset(matchFlags, false, sizeof(matchFlags));

		if (!IsBlockMatch()) break;

		if (DeleteLine() > 0)
		{
			PushCharDown();
			FillBlank();
		}
	}
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
		Render(30, 5);

		InputKey();
		CheckBlockLine();
				
		ClearScreen();
		BufferFlip();
		Sleep(1);		
	}

	DestroyGame();

	return 0;
}