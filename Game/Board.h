#pragma once

#include <vector>

#include "eDirection.h"
#include "Move.h"
#include "../DDraw/DDraw.h"

class BoardAnalyzer;

class Board final
{
public:
    enum { ROWS = 15, COLS = 15 };

public:
    Board(DDraw* ddraw);
    ~Board();

    void Draw() const;

    inline bool IsValidPos(const Vector2& pos) const { return (pos.X < COLS && pos.Y < ROWS); }
    bool IsPlaceable(const Vector2& pos) const;
    bool PlaceStone(const Move& move);
    Move GetLastMove() const;
    inline eColor(*GetBoard())[COLS] { return mBoard; }
    
    uint32_t GetChainCount(const Move& move, const eDirection direction) const;
    bool HasWon(const eColor color) const;

    void ClearBoard();
    inline bool CanUndo() const { return !mNotationVect.empty(); }
    inline bool CanRedo() const { return !mRedoVect.empty(); }
    bool Undo();
    bool Redo();

    bool SaveNotation() const;
    bool LoadNotation();

    Vector2 MousePosToBoardPos() const;

private:
    void drawIllegalMove(HDC hdc, const wchar_t* text, const uint32_t x, const uint32_t y) const;

private:
    static constexpr float BOARD_RATIO = 0.7f;
    static constexpr uint32_t BOARD_PADDING = 30;

    static constexpr uint32_t BOARD_COLOR = 0xd4bb72U;          // 나무 색상
    static constexpr uint32_t LINE_COLOR = 0x000000U;
    static constexpr uint32_t BLACK_STONE_COLOR = 0x000000U;
    static constexpr uint32_t WHITE_STONE_COLOR = 0xffffffU;
    static constexpr uint32_t CENTER_POINT_COLOR = 0x000000U;
    static constexpr uint32_t ILLEGAL_MOVE_COLOR = 0xed2939U;    // 빨강
    static constexpr uint32_t ILLEGAL_MOVE_TEXT_COLOR = 0x000000U;

private:
    DDraw* mDDraw;

    uint32_t mBoardWidth;
    uint32_t mBoardHeight;
    uint32_t mCellWidth;
    uint32_t mCellHeight;
    uint32_t mStoneWidth;
    uint32_t mStoneHeight;
    uint32_t mCenterPointWidth;
    uint32_t mCenterPointHeight;
    uint32_t mBoardX;
    uint32_t mBoardY;

    eColor mBoard[ROWS][COLS];
    std::vector<Move> mNotationVect;
    std::vector<Move> mRedoVect;

    BoardAnalyzer* mBlackAnalyzer;
    BoardAnalyzer* mWhiteAnalyzer;
};