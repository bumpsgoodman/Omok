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

private:
    void updateSpotInfoRecursive(const Vector2& pos, eColor(*board)[Board::COLS]);
    eSpotInfo getSpotInfoRecursive(const Vector2& pos, const eDirection direction, eColor(*board)[Board::COLS]);
    Vector2 getEmptyPos(const Vector2& pos, const int32_t dx, const int32_t dy, const eColor(*board)[Board::COLS]) const;
    void recheckThreeAndThree(const Vector2& pos, eColor(*board)[Board::COLS]);

private:
    Board* mBoard;
    const eColor mColor;

    eSpotInfo mSpotInfos[Board::ROWS][Board::COLS][(uint32_t)eDirection::Count] = {};
    bool mbUpdatedSpots[Board::ROWS][Board::COLS] = {};
};