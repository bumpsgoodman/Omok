#pragma once

#include <vector>

class Board;
class BoardAnalyzer;
enum class eColor;
struct Move;
struct Vector2;

class Goosebumps
{
public:
    Goosebumps(Board* board, const eColor color);
    Goosebumps(const Goosebumps&) = delete;
    ~Goosebumps() = default;

    Move GetBestMove();

private:
    Move getForcedMove() const;
    std::vector<Vector2> getRootPosVect(const uint32_t maxSize) const;

private:
    Board* mBoard;

    eColor mColor;
    eColor mOpponentColor;

    const BoardAnalyzer* mAnalyzer;
    const BoardAnalyzer* mOpponentAnalyzer;
};