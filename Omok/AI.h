#pragma once

#include <vector>

#include "Board.h"

struct Move;

class AI final
{
public:
    AI(Board* board, const eColor color, const eColor opponentColor);
    AI(const AI&) = delete;
    ~AI();

    const Vector2 GetBestMovePos() const;

private:
    const Move getBestMoveRecursive(Board& board, const size_t level) const;
    const Move getMinScoreMove(const std::vector<Move>& movesVect) const;
    const Move getMaxScoreMove(const std::vector<Move>& movesVect) const;
    Move evaluateMove(Board* board) const;

private:
    Board* mBoard;
    eColor mColor;
    eColor mOpponentColor;
};