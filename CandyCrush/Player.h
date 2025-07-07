#pragma once

class CPlayer
{
public:
	enum eCheckBlock
	{
		Check0, // Not Corrected Block
		Check1, // Corrected Block
	};

private:
	// Current X Position
	int m_nXPos;
	// Minimum X Position
	int m_nMinXPos;
	// Maximum X Position
	int m_nMaxXPos;
	// Current Y Position
	int m_nYPos;
	// Minimum Y Position
	int m_nMinYPos;
	// Maximum Y Position
	int m_nMaxYPos;
	// Current Block Index
	int m_nCurBlock;
	// Current Block State
	eCheckBlock m_eCheckBlock;
	// Game Score
	int m_nGameScore;
	// Game Over
	bool m_bIsGameOver;

public:
	CPlayer(int nXPos = 0, int nYPos = 0, int nCurBlock = 0, eCheckBlock eCheck = eCheckBlock::Check0)
		: m_nXPos(nXPos), m_nYPos(nYPos), m_nCurBlock(nCurBlock), m_eCheckBlock(eCheck)
		, m_nGameScore(0), m_bIsGameOver(false)
	{ }

	inline void SetXPositionRange(int nMinXPos, int nMaxXPos)
	{
		m_nMinXPos = nMinXPos;
		m_nMaxXPos = nMaxXPos;
	}

	inline void SetYPositionRange(int nMinYPos, int nMaxYPos)
	{
		m_nMinYPos = nMinYPos;
		m_nMaxYPos = nMaxYPos;
	}

	inline void SetXPosition(int nXPos)
	{
		m_nXPos = nXPos;
	}

	inline void SetYPosition(int nYPos)
	{
		m_nYPos = nYPos;
	}

	inline void SetPosition(int nXPos, int nYPos)
	{
		m_nXPos = nXPos;
		m_nYPos = nYPos;
	}

	inline void SetBlock(int nBlock)
	{
		m_nCurBlock = nBlock;
	}

	inline void SetCheckBlock(eCheckBlock Check)
	{
		m_eCheckBlock = Check;
	}

	inline void SetGameScore(int nScore)
	{
		m_nGameScore = nScore;
	}

	inline void AddGameScore(int nScore)
	{
		m_nGameScore = m_nGameScore + nScore >= 0 ? m_nGameScore + nScore : 0;
	}

	inline void SetGameOver(bool bIsGameOver)
	{
		m_bIsGameOver = bIsGameOver;
	}

	inline int GetXPosition() const
	{
		return m_nXPos;
	}

	inline int GetYPosition() const
	{
		return m_nYPos;
	}

	inline int GetCurBlock() const
	{
		return m_nCurBlock;
	}

	inline eCheckBlock GetCheckBlock() const
	{
		return m_eCheckBlock;
	}

	inline COORD GetCursor() const
	{
		COORD cursor{ (short)m_nXPos, (short)m_nYPos };
		return cursor;
	}

	inline int GetGameScore() const
	{
		return m_nGameScore;
	}

	inline bool GetGameOver() const
	{
		return m_bIsGameOver;
	}

	CPlayer& operator=(CPlayer& player)
	{
		m_nXPos = player.m_nXPos;
		m_nYPos = player.m_nYPos;
		m_nCurBlock = player.m_nCurBlock;
		m_eCheckBlock = player.m_eCheckBlock;
		m_nGameScore = player.m_nGameScore;
		m_bIsGameOver = player.m_bIsGameOver;

		return *this;
	}

	friend bool operator==(const CPlayer& player1, const CPlayer& player2)
	{
		return (player1.m_nXPos == player2.m_nXPos) &&
			(player1.m_nYPos == player2.m_nYPos) &&
			(player1.m_nCurBlock == player2.m_nCurBlock) &&
			(player1.m_eCheckBlock == player2.m_eCheckBlock) &&
			(player1.m_nGameScore == player2.m_nGameScore) &&
			(player1.m_bIsGameOver == player2.m_bIsGameOver);
	}

	friend bool operator!=(const CPlayer& player1, const CPlayer& player2)
	{
		return !(player1 == player2);
	}
};