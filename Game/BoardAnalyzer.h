#pragma once

#include "Board.h"
#include "eDirection.h"
#include "eSpotInfo.h"

class BoardAnalyzer final
{
public:
    BoardAnalyzer(Board* board, const eColor color);
    BoardAnalyzer(const BoardAnalyzer&) = default;
    ~BoardAnalyzer();

    void Clear();
    void Update(const Move& move);

    const eSpotInfo(*GetSpotInfos() const)[Board::COLS][(uint32_t)eDirection::Count] { return mSpotInfos; }

    bool IsOverLines(const Vector2& pos) const;
    bool IsFourAndFour(const Vector2& pos) const;
    bool IsThreeAndThree(const Vector2& pos) const;
    bool IsPreFourAndFour(const Vector2& pos) const;

private:
    eSpotInfo getSpotInfoRecursive(const Vector2& pos, const eDirection direction, eColor(*board)[Board::COLS]);
    Vector2 getEmptyPos(const Vector2& pos, const int32_t dx, const int32_t dy, const eColor(*board)[Board::COLS]) const;

private:
    Board* mBoard;
    const eColor mColor;

    eSpotInfo mSpotInfos[Board::ROWS][Board::COLS][(uint32_t)eDirection::Count] = {};
};