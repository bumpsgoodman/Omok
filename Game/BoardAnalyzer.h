#pragma once

#include <unordered_map>
#include <vector>

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
    void Update(const Vector2& pos);

    const eSpotInfo(*GetSpotInfos() const)[Board::COLS][(uint32_t)eDirection::Count] { return mSpotInfos; }

    const std::vector<Vector2> GetOverLineVect() const { return mOverLineVect; }
    const std::vector<Vector2> GetFourAndFourVect() const { return mFourAndFourVect; }
    const std::vector<Vector2> GetThreeAndThreeVect() const { return mThreeAndThreeVect; }

    bool IsOverLines(const Vector2& pos) const;
    bool IsFourAndFour(const Vector2& pos) const;
    bool IsThreeAndThree(const Vector2& pos) const;

private:
    void updateSpotInfoRecursive(const Vector2& pos, eColor(*board)[Board::COLS]);
    void updateIllegalMoveVect(const Vector2& pos);
    void removePosFromIllegalMove(const Vector2& pos);
    
    eSpotInfo getSpotInfoRecursive(const Vector2& pos, const eDirection direction, eColor(*board)[Board::COLS]);
    Vector2 getEmptyPos(const Vector2& pos, const int32_t dx, const int32_t dy, const eColor(*board)[Board::COLS]) const;
    void recheckThreeAndThree(const Vector2& pos, eColor(*board)[Board::COLS]);

private:
    Board* mBoard;
    const eColor mColor;

    eSpotInfo mSpotInfos[Board::ROWS][Board::COLS][(uint32_t)eDirection::Count] = {};
    bool mbUpdatedSpots[Board::ROWS][Board::COLS] = {};

    std::vector<Vector2> mOverLineVect;
    std::vector<Vector2> mFourAndFourVect;
    std::vector<Vector2> mThreeAndThreeVect;

    // key: eSpotInfo의 해시 (GetSpotInfoHash() 함수를 통해 반환된 해시)
    // value: eSpotInfo에 해당하는 위치들이 저장된 vector
    std::unordered_map<uint32_t, std::vector<Vector2>> mSpotInfoMap;
};