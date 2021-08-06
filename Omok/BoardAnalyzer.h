#pragma once

#include <vector>

#include "eColor.h"
#include "eDirection.h"
#include "../Util/Vector2.h"

class Board;

class BoardAnalyzer
{
public:
    BoardAnalyzer() = default;
    BoardAnalyzer(const Board&) = delete;
    ~BoardAnalyzer();

    bool Initailize(Board* board);
    void Cleanup();
    void Update();

    void Clear();

    const std::vector<Vector2>& GetIllegalPos() const { return mIllegalPosVect; }
    const std::vector<Vector2>& GetOverLinesPosVect() const { return mOverLinesPosVect; }
    const std::vector<Vector2>& GetFourAndFourPosVect() const { return mFourAndFourPosVect; }
    const std::vector<Vector2>& GetThreeAndThreePosVect() const { return mThreeAndThreePosVect; }

    bool IsFourAndFour(const size_t row, const size_t col, const eColor color) const;
    bool IsThreeAndThree(const size_t row, const size_t col, const eColor color) const;

    size_t getOpenClosedFourCount(const size_t row, const size_t col, const eDirection direction) const;
    bool IsOpenFour(const size_t row, const size_t col, const eDirection direction, const eColor color) const;
    bool IsOpenThree(const size_t row, const size_t col, const eDirection direction, const eColor color) const;

private:
    void calculateIllegalPos();

    bool isOmok(const size_t row, const size_t col) const;
    bool isOverLines(const size_t row, const size_t col) const;

private:
    Board* mBoard = nullptr;

    std::vector<Vector2> mIllegalPosVect;
    std::vector<Vector2> mOverLinesPosVect;
    std::vector<Vector2> mFourAndFourPosVect;
    std::vector<Vector2> mThreeAndThreePosVect;
};