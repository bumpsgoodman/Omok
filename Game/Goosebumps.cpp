#include <assert.h>
#include <cstdint>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <vector>

#include "Board.h"
#include "BoardAnalyzer.h"
#include "Goosebumps.h"

Goosebumps::Goosebumps(Board* board, const eColor color)
    : mBoard(board)
    , mColor(color)
{
    assert(board != nullptr);
    assert(color != eColor::None);

    mOpponentColor = (color == eColor::Black) ? eColor::White : eColor::Black;

    mAnalyzer = board->GetAnalyzer(color);
    mOpponentAnalyzer = board->GetAnalyzer(mOpponentColor);

    srand((uint32_t)time(nullptr));
}

Vector2 Goosebumps::GetBestMovePos()
{
    Vector2 move = getForcedMovePos();
    if (mBoard->IsPlaceable(move))
    {
        goto EXIT;
    }

EXIT:
    return move;
}

// ���� ���� ���ٸ� �ùٸ��� ���� �� ��ȯ
Vector2 Goosebumps::getForcedMovePos() const
{
    Vector2 result = { UINT32_MAX, UINT32_MAX };

    const std::vector<Vector2>& overLineVect = mAnalyzer->GetOverLineVect();
    const std::vector<Vector2>& fourAndFourVect = mAnalyzer->GetFourAndFourVect();
    const std::vector<Vector2>& open5Vect = mAnalyzer->GetSpotInfoVect(eSpotInfo::Open5);
    const std::vector<Vector2>& open4Vect = mAnalyzer->GetSpotInfoVect(eSpotInfo::Open4);

    const std::vector<Vector2>& opponentOverLineVect = mOpponentAnalyzer->GetOverLineVect();
    const std::vector<Vector2>& opponentFourAndFourVect = mOpponentAnalyzer->GetFourAndFourVect();
    const std::vector<Vector2>& opponentOpen5Vect = mOpponentAnalyzer->GetSpotInfoVect(eSpotInfo::Open5);
    const std::vector<Vector2>& opponentOpen4Vect = mOpponentAnalyzer->GetSpotInfoVect(eSpotInfo::Open4);

    // ���� �ٷ� �̱�� ��
    {
        if (open5Vect.size() > 0)
        {
            result = open5Vect.back();
            goto EXIT;
        }

        if (mColor == eColor::White)
        {
            if (overLineVect.size() > 0)
            {
                result = overLineVect.back();
                goto EXIT;
            }
        }
    }

    // ������ �ٷ� �̱�� ��
    {
        if (opponentOpen5Vect.size() > 0)
        {
            result = opponentOpen5Vect.back();
            goto EXIT;
        }

        if (mOpponentColor == eColor::White)
        {
            if (opponentOverLineVect.size() > 0)
            {
                result = opponentOverLineVect.back();
                goto EXIT;
            }
        }
    }

    /*
    // ������ �̱�� ��
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
                        result = opponentOpen4Vect.back();
                        goto EXIT;
                    }
                }
            }
            else
            {
                result = opponentOpen4Vect.back();
                goto EXIT;
            }
        }
    }

    // ���� �̱�� ��
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
                        result = open4Vect.back();
                        goto EXIT;
                    }
                }
            }
            else
            {
                result = open4Vect.back();
                goto EXIT;
            }
        }
    }
    */

EXIT:
    return result;
}

std::vector<Vector2> Goosebumps::getRootPosVect(const uint32_t maxSize) const
{
    std::vector<Vector2> result;
    result.reserve(maxSize);

EXIT:
    return result;
}