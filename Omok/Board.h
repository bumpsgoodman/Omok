#pragma once

#include <stack>
#include <vector>

#include "BoardAnalyzer.h"
#include "eColor.h"
#include "eDirection.h"
#include "../DDraw/DDraw.h"
#include "../Util/Vector2.h"

class AI;

class Board
{
public:
    Board() = default;
    Board(const Board& other);
    ~Board();

    bool Initailize(DDraw* ddraw);
    void Cleanup();
    void Update();
    void Draw() const;

    void UpdateBoardSize();

    static size_t GetRows() { return ROWS; }
    static size_t GetCols() { return COLS; }

    size_t GetBoardWidth() const { return mBoardWidth; }
    size_t GetBoardHeight() const { return mBoardHeight; }
    size_t GetCellWidth() const { return mCellWidth; }
    size_t GetCellHeight() const { return mCellHeight; }
    size_t GetStoneWidth() const { return mStoneWidth; }
    size_t GetStoneHeight() const { return mStoneHeight; }
    size_t GetBoardX() const { return mBoardX; }
    size_t GetBoardY() const { return mBoardY; }

    const BoardAnalyzer* GetAnalyzer() const { return mAnalyzer; }

    size_t GetLastPlacedRow() const { return mLastPlacedRow; }
    size_t GetLastPlacedCol() const { return mLastPlacedCol; }
    eColor GetCurTurn() const { return mCurTurn; }

    eColor GetColor(const size_t row, const size_t col) const;
    void SetColor(const size_t row, const size_t col, const eColor color);

    bool IsValidPos(const size_t row, const size_t col) const { return (row < ROWS && col < COLS); }
    bool IsPlaceable(const size_t row, const size_t col) const;
    bool PlaceStone(const size_t row, const size_t col, const eColor color);
    size_t GetChainCount(const size_t row, const size_t col, const eColor color, const eDirection direction) const;
    bool HasWon();
    bool HasWon(const eColor color);

    void GetPlaceablePos(std::vector<Vector2>* outPosVect);

    void Clear();
    bool CanUndo() const { return !mUndoStack.empty(); }
    bool CanRedo() const { return !mRedoStack.empty(); }
    bool Undo();
    bool Redo();

    bool SaveNotation();
    bool LoadNotation();

    size_t TranslateToBoardX(const size_t x) const;
    size_t TranslateToBoardY(const size_t y) const;

    void ChangeTurn();

private:
    static const size_t ROWS = 15;
    static const size_t COLS = 15;

    static const unsigned int WOOD_COLOR = 0xd4bb72U;
    static const unsigned int LINE_COLOR = 0x000000U;
    static const unsigned int BLACK_STONE_COLOR = 0x000000U;
    static const unsigned int WHITE_STONE_COLOR = 0xffffffU;
    static const unsigned int ILLEGAL_BOX_COLOR = 0xed2939U;

    HFONT mDefaultBoardFont = nullptr;
    HFONT mLargeBoardFont = nullptr;

    DDraw* mDDraw = nullptr;
    BoardAnalyzer* mAnalyzer = nullptr;

    size_t mBoardWidth = 0;
    size_t mBoardHeight = 0;
    size_t mCellWidth = 0;
    size_t mCellHeight = 0;
    size_t mStoneWidth = 0;
    size_t mStoneHeight = 0;
    size_t mBoardX = 0;
    size_t mBoardY = 0;

    bool mbBlackWon = false;
    bool mbWhiteWon = false;
    eColor* mBoard = nullptr;
    eColor mCurTurn = eColor::Black;
    size_t mLastPlacedRow = 0;
    size_t mLastPlacedCol = 0;
    size_t mStoneCount = 0;

    AI* mAI = nullptr;
    eColor mPlayerColor = eColor::None;
    eColor mAiColor = eColor::None;

    std::stack<Vector2> mUndoStack;
    std::stack<Vector2> mRedoStack;

    std::vector<Vector2> mNotationVect;
};