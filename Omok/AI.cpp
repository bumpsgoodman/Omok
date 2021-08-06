#include <assert.h>
#include <vector>
#include <Windows.h>

#include "AI.h"
#include "Move.h"

AI::AI(Board* board, const eColor color, const eColor opponentColor)
    : mBoard(board)
    , mColor(color)
    , mOpponentColor(opponentColor)
{

}

AI::~AI()
{
}

const Vector2 AI::GetBestMovePos() const
{
    Board copiedBoard(*mBoard);

    const Move move = getBestMoveRecursive(copiedBoard, 0);
    return move.Pos;
}

const Move AI::getBestMoveRecursive(Board& board, const size_t level) const
{
    Move move = {};

    if (level > 2)
    {
        move = evaluateMove(&board);
        return move;
    }

    if (board.HasWon(mColor))
    {
        move.Pos.X = -1;
        move.Pos.Y = -1;
        move.Score = 100;

        return move;
    }
    else if (board.HasWon(mOpponentColor))
    {
        move.Pos.X = -1;
        move.Pos.Y = -1;
        move.Score = -100;

        return move;
    }
    
    std::vector<Vector2> placeablePosVect;
    board.GetPlaceablePos(&placeablePosVect);

    if (placeablePosVect.empty())
    {
        move.Pos.X = -1;
        move.Pos.Y = -1;
        move.Score = 0;

        return move;
    }

    while (placeablePosVect.size() > 30)
    {
        placeablePosVect.pop_back();
    }

    std::vector<Move> movesVect;
    for (const Vector2& v : placeablePosVect)
    {
        Board copiedBoard(board);
        
        copiedBoard.PlaceStone(v.Y, v.X, copiedBoard.GetCurTurn());

        Move move = getBestMoveRecursive(copiedBoard, level + 1);
        movesVect.push_back(move);
    }

    if (board.GetCurTurn() == mColor)
    {
        return getMaxScoreMove(movesVect);
    }

    return getMinScoreMove(movesVect);
}

const Move AI::getMinScoreMove(const std::vector<Move>& movesVect) const
{
    assert(!movesVect.empty());

    Move minScoreMove = movesVect.at(0);
    for (size_t i = 1; i < movesVect.size(); ++i)
    {
        if (movesVect.at(i).Score < minScoreMove.Score)
        {
            minScoreMove = movesVect.at(i);
        }
    }

    return minScoreMove;
}

const Move AI::getMaxScoreMove(const std::vector<Move>& movesVect) const
{
    assert(!movesVect.empty());

    Move maxScoreMove = movesVect.at(0);
    for (size_t i = 1; i < movesVect.size(); ++i)
    {
        if (movesVect.at(i).Score > maxScoreMove.Score)
        {
            maxScoreMove = movesVect.at(i);
        }
    }

    return maxScoreMove;
}

Move AI::evaluateMove(Board* board) const
{
    assert(board != nullptr);

    int score = 0;

    eColor color = (board->GetCurTurn() == eColor::White) ? eColor::Black : eColor::White;
    int row = board->GetLastPlacedRow();
    int col = board->GetLastPlacedCol();

    score += board->GetChainCount(row, col, color, eDirection::Horizontal);
    score += board->GetChainCount(row, col, color, eDirection::Vertical);
    score += board->GetChainCount(row, col, color, eDirection::LeftDiagonal);
    score += board->GetChainCount(row, col, color, eDirection::RightDiagonal);

    Move move = { col, row, score };

    return move;
}