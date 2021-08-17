#pragma once

#include <vector>

class Board;
class BoardAnalyzer;
enum class eColor;
struct Vector2;

class Goosebumps final
{
public:
    Goosebumps(Board* board, const eColor color);
    Goosebumps(const Goosebumps&) = delete;
    ~Goosebumps() = default;

    inline eColor GetColor() const { return mColor; }

    Vector2 GetBestMovePos();

private:
    Vector2 getForcedMovePos() const;
    std::vector<Vector2> getRootPosVect(const uint32_t maxSize) const;

private:
    Board* mBoard;

    eColor mColor;
    eColor mOpponentColor;

    const BoardAnalyzer* mAnalyzer;
    const BoardAnalyzer* mOpponentAnalyzer;
};