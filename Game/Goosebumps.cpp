#include <assert.h>
#include <cstdint>
#include <string.h>
#include <vector>

#include "Board.h"
#include "BoardAnalyzer.h"
#include "Goosebumps.h"
#include "Move.h"

Goosebumps::Goosebumps(Board* board, const eColor color)
    : mBoard(board)
    , mColor(color)
{
    assert(board != nullptr);
    assert(color != eColor::None);

    mOpponentColor = (color == eColor::Black) ? eColor::White : eColor::Black;

    mAnalyzer = board->GetAnalyzer(color);
    mOpponentAnalyzer = board->GetAnalyzer(mOpponentColor);
}

Move Goosebumps::GetBestMove()
{
    Move move = getForcedMove();
    if (mBoard->IsPlaceable(move))
    {
        goto EXIT;
    }

EXIT:
    return move;
}

// 강제 수가 없다면 올바르지 않은 수 반환
Move Goosebumps::getForcedMove() const
{
    Move move = { { UINT32_MAX, UINT32_MAX }, mColor };

    const std::vector<Vector2>& overLineVect = mAnalyzer->GetOverLineVect();
    const std::vector<Vector2>& fourAndFourVect = mAnalyzer->GetFourAndFourVect();
    const std::vector<Vector2>& open5Vect = mAnalyzer->GetSpotInfoVect(eSpotInfo::Open5);
    const std::vector<Vector2>& open4Vect = mAnalyzer->GetSpotInfoVect(eSpotInfo::Open4);

    const std::vector<Vector2>& opponentOverLineVect = mOpponentAnalyzer->GetOverLineVect();
    const std::vector<Vector2>& opponentFourAndFourVect = mOpponentAnalyzer->GetFourAndFourVect();
    const std::vector<Vector2>& opponentOpen5Vect = mOpponentAnalyzer->GetSpotInfoVect(eSpotInfo::Open5);
    const std::vector<Vector2>& opponentOpen4Vect = mOpponentAnalyzer->GetSpotInfoVect(eSpotInfo::Open4);

    // 내가 바로 이기는 수
    {
        if (open5Vect.size() > 0)
        {
            move.Pos = open5Vect.back();
            goto EXIT;
        }

        if (mColor == eColor::White)
        {
            if (overLineVect.size() > 0)
            {
                move.Pos = overLineVect.back();
                goto EXIT;
            }
        }
    }

    // 상대방이 바로 이기는 수
    {
        if (opponentOpen5Vect.size() > 0)
        {
            move.Pos = opponentOpen5Vect.back();
            goto EXIT;
        }

        if (mOpponentColor == eColor::White)
        {
            if (opponentOverLineVect.size() > 0)
            {
                move.Pos = opponentOverLineVect.back();
                goto EXIT;
            }
        }
    }

    // 상대방이 이기는 수
    {
        if (opponentOpen4Vect.size() > 0)
        {
            if (mOpponentColor == eColor::Black)
            {
                for (uint32_t i = 0; i < opponentOpen4Vect.size(); ++i)
                {
                    const Vector2& pos = opponentOpen4Vect.at(i);
                    bool bIllegal = false;
                    for (const Vector2& illegalPos : opponentFourAndFourVect)
                    {
                        if (pos.X == illegalPos.X && pos.Y == illegalPos.Y)
                        {
                            bIllegal = true;
                            break;
                        }
                    }

                    if (!bIllegal)
                    {
                        move.Pos = opponentOpen4Vect.back();
                        goto EXIT;
                    }
                }
            }
            else
            {
                move.Pos = opponentOpen4Vect.back();
                goto EXIT;
            }
        }
    }

    // 내가 이기는 수
    {
        if (open4Vect.size() > 0)
        {
            if (mColor == eColor::Black)
            {
                for (uint32_t i = 0; i < open4Vect.size(); ++i)
                {
                    const Vector2& pos = open4Vect.at(i);
                    bool bIllegal = false;
                    for (const Vector2& illegalPos : fourAndFourVect)
                    {
                        if (pos.X == illegalPos.X && pos.Y == illegalPos.Y)
                        {
                            bIllegal = true;
                            break;
                        }
                    }

                    if (!bIllegal)
                    {
                        move.Pos = open4Vect.back();
                        goto EXIT;
                    }
                }
            }
            else
            {
                move.Pos = open4Vect.back();
                goto EXIT;
            }
        }
    }

EXIT:
    return move;
}

std::vector<Vector2> Goosebumps::getRootPosVect(const uint32_t maxSize) const
{
    std::vector<Vector2> result;
    result.reserve(maxSize);

EXIT:
    return result;
}