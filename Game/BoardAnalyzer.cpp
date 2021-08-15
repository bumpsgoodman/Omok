#include <assert.h>
#include <cstdint>

#include "BoardAnalyzer.h"

BoardAnalyzer::BoardAnalyzer(Board* board, const eColor color)
    : mBoard(board)
    , mColor(color)
{
    assert(board != nullptr);
    assert(color != eColor::None);
}

BoardAnalyzer::~BoardAnalyzer()
{

}

void BoardAnalyzer::Clear()
{
    eColor(*board)[Board::COLS] = mBoard->GetBoard();

    for (uint32_t y = 0; y < Board::ROWS; ++y)
    {
        for (uint32_t x = 0; x < Board::COLS; ++x)
        {
            const Vector2 pos = { x, y };
            for (uint32_t i = 0; i < (uint32_t)eDirection::Count; ++i)
            {
                mSpotInfos[y][x][i] = getSpotInfoRecursive(pos, (eDirection)i, board);
            }

            // 33일 경우 재판정하여 올바르게 표시
            // 1. 현재 위치에 하나의 돌 착수
            // 2. 각 방향에 대해서 금수가 생기는지 판단
            // 2.1. 금수가 생기지 않았으면 열린3
            // 2.2. 금수가 생겼을 때 반대 방향이 닫힌이면 닫힌3,
            // 2.3. 금수가 생겼을 때 반대 방향이 열린이면 열린3
            // 2.4. 양쪽 모두 금수라면 닫힌3
            if (IsThreeAndThree(pos))
            {
                board[y][x] = mColor;
                for (uint32_t i = 0; i < (uint32_t)eDirection::Count; ++i)
                {
                    if (mSpotInfos[y][x][i] != eSpotInfo::Open3)
                    {
                        continue;
                    }

                    uint32_t count = 0;
                    int32_t dx;
                    int32_t dy;
                    TranslateDirection((eDirection)i, &dx, &dy);
                    
                    const Vector2 frontPos = getEmptyPos(pos, dx, dy, board);
                    const Vector2 backPos = getEmptyPos(pos, -dx, -dy, board);
                    if (mBoard->IsValidPos(frontPos))
                    {
                        // 정방향 정보 재판정
                        eSpotInfo temp[(uint32_t)eDirection::Count];
                        for (uint32_t j = 0; j < (uint32_t)eDirection::Count; ++j)
                        {
                            temp[j] = mSpotInfos[frontPos.Y][frontPos.X][j];
                            mSpotInfos[frontPos.Y][frontPos.X][j] = getSpotInfoRecursive(frontPos, (eDirection)j, board);
                        }

                        if (IsFourAndFour(frontPos) || IsOverLines(frontPos) || IsThreeAndThree(frontPos)
                            && mBoard->IsValidPos(backPos))
                        {
                            // 정방향 금수 + 반대 방향 닫힌 = 닫힌3
                            if (!IsOpen(getSpotInfoRecursive(backPos, (eDirection)i, board)))
                            {
                                mSpotInfos[y][x][i] = eSpotInfo::Closed3;
                            }
                            else
                            {
                                ++count;
                            }
                        }

                        // 정방향 정보 되돌리기
                        for (uint32_t j = 0; j < (uint32_t)eDirection::Count; ++j)
                        {
                            mSpotInfos[frontPos.Y][frontPos.X][j] = temp[j];
                        }
                    }
                    
                    if (mBoard->IsValidPos(backPos))
                    {
                        // 반대 방향 정보 재판정
                        eSpotInfo temp[(uint32_t)eDirection::Count];
                        for (uint32_t j = 0; j < (uint32_t)eDirection::Count; ++j)
                        {
                            temp[j] = mSpotInfos[backPos.Y][backPos.X][j];
                            mSpotInfos[backPos.Y][backPos.X][j] = getSpotInfoRecursive(backPos, (eDirection)j, board);
                        }

                        if (IsFourAndFour(backPos) || IsOverLines(backPos) || IsThreeAndThree(backPos)
                            && mBoard->IsValidPos(frontPos))
                        {
                            // 반대 방향 금수 + 정방향 닫힌 = 닫힌3
                            if (!IsOpen(getSpotInfoRecursive(frontPos, (eDirection)i, board)))
                            {
                                mSpotInfos[y][x][i] = eSpotInfo::Closed3;
                            }
                            else
                            {
                                ++count;
                            }
                        }

                        // 반대 방향 정보 되돌리기
                        for (uint32_t j = 0; j < (uint32_t)eDirection::Count; ++j)
                        {
                            mSpotInfos[backPos.Y][backPos.X][j] = temp[j];
                        }
                    }

                    // 양쪽 모두 금수라면 닫힌3
                    if (count == 2)
                    {
                        mSpotInfos[y][x][i] = eSpotInfo::Closed3;
                    }
                }
                board[y][x] = eColor::None;
            }
        }
    }
}

void BoardAnalyzer::Update(const Move& move)
{
    eColor(*board)[Board::COLS] = mBoard->GetBoard();
    Clear();
}

eSpotInfo BoardAnalyzer::getSpotInfoRecursive(const Vector2& pos, const eDirection direction, eColor(*board)[Board::COLS])
{
    if (!mBoard->IsPlaceable(pos))
    {
        return eSpotInfo::None;
    }

    int32_t dx = 0;
    int32_t dy = 0;
    TranslateDirection(direction, &dx, &dy);

    board[pos.Y][pos.X] = mColor;

    const Move move = { pos, mColor };
    const uint32_t curChainCount = mBoard->GetChainCount(move, direction);
    uint32_t frontChainCount = curChainCount;
    uint32_t backChainCount = curChainCount;

    // 정방향에 돌을 하나 더 뒀을 때 개수
    const Vector2 frontPos = getEmptyPos(pos, dx, dy, board);
    if (mBoard->IsPlaceable(frontPos))
    {
        Move frontMove = { frontPos, mColor };

        board[frontPos.Y][frontPos.X] = mColor;
        frontChainCount = mBoard->GetChainCount(frontMove, direction);
        board[frontPos.Y][frontPos.X] = eColor::None;
    }

    // 반대방향에 돌을 하나 더 뒀을 때 개수
    const Vector2 backPos = getEmptyPos(pos, -dx, -dy, board);
    if (mBoard->IsPlaceable(backPos))
    {
        Move backMove = { backPos, mColor };

        board[backPos.Y][backPos.X] = mColor;
        backChainCount = mBoard->GetChainCount(backMove, direction);
        board[backPos.Y][backPos.X] = eColor::None;
    }

    eSpotInfo result;
    if (curChainCount == 5)
    {
        result = eSpotInfo::Open5;
    }
    else if (curChainCount >= 6)
    {
        result = eSpotInfo::Open6;
    }
    else if (frontChainCount == 5 && backChainCount == 5)
    {
        result = eSpotInfo::Open4;
    }
    else if (frontChainCount == 5 || backChainCount == 5)
    {
        result = eSpotInfo::Closed4;
    }
    else if (frontChainCount == curChainCount && backChainCount == curChainCount)
    {
        result = eSpotInfo::None;
    }
    else if (frontChainCount != curChainCount && backChainCount != curChainCount
        && frontChainCount < 5 && backChainCount < 5)
    {
        const eSpotInfo frontResult = getSpotInfoRecursive(frontPos, direction, board);
        const eSpotInfo backResult = getSpotInfoRecursive(backPos, direction, board);
        if (frontResult == eSpotInfo::None && backResult == eSpotInfo::None)
        {
            result = eSpotInfo::None;
        }
        else if (IsOpen(frontResult) || IsOpen(backResult))
        {
            result = (eSpotInfo)(max(frontChainCount, backChainCount) - 1);
        }
        else
        {
            result = (eSpotInfo)(max(frontChainCount, backChainCount) - 1 + 6);
        }
    }
    else if (frontChainCount != curChainCount && frontChainCount < 5)
    {
        const eSpotInfo frontResult = getSpotInfoRecursive(frontPos, direction, board);
        result = (frontResult == eSpotInfo::None) ? eSpotInfo::None : (eSpotInfo)(frontChainCount - 1 + 6);
    }
    else if (backChainCount != curChainCount && backChainCount < 5)
    {
        const eSpotInfo backResult = getSpotInfoRecursive(backPos, direction, board);
        result = (backResult == eSpotInfo::None) ? eSpotInfo::None : (eSpotInfo)(backChainCount - 1 + 6);
    }
    else
    {
        result = eSpotInfo::None;
    }

    board[pos.Y][pos.X] = eColor::None;

    return result;
}

// 찾을 수 없다면 올바르지 않은 위치 반환
Vector2 BoardAnalyzer::getEmptyPos(const Vector2& pos, const int32_t dx, const int32_t dy, const eColor(*board)[Board::COLS]) const
{
    Vector2 result = { pos.X + dx, pos.Y + dy };
    while (mBoard->IsValidPos(result) && board[result.Y][result.X] != eColor::None)
    {
        if (board[result.Y][result.X] != mColor)
        {
            result.X = UINT32_MAX;
            result.Y = UINT32_MAX;
            break;
        }

        result.X += dx;
        result.Y += dy;
    }

    return result;
}

bool BoardAnalyzer::IsOverLines(const Vector2& pos) const
{
    uint32_t count = 0;
    for (uint32_t i = 0; i < (uint32_t)eDirection::Count; ++i)
    {
        const eSpotInfo spotInfo = mSpotInfos[pos.Y][pos.X][i];
        if (spotInfo == eSpotInfo::Open6)
        {
            ++count;
        }
        else if (spotInfo == eSpotInfo::Open5)
        {
            return false;
        }
    }

    return (count >= 1);
}

bool BoardAnalyzer::IsFourAndFour(const Vector2& pos) const
{
    uint32_t count = 0;
    eDirection dir;

    for (uint32_t i = 0; i < (uint32_t)eDirection::Count; ++i)
    {
        const eSpotInfo spotInfo = mSpotInfos[pos.Y][pos.X][i];
        if (spotInfo == eSpotInfo::Open4 || spotInfo == eSpotInfo::Closed4)
        {
            ++count;
            dir = (eDirection)i;
        }
        else if (spotInfo == eSpotInfo::Open5)
        {
            return false;
        }
    }

    // 같은 방향에서 열린4가 두 개일 경우 44
    // 1. 현재 위치에 돌을 뒀을 때 이어지는 개수가 4개가 안될 경우
    // 2. 양쪽 모두 열린/닫힌4일 경우 같은 방향에서의 44
    if (count == 1)
    {
        const Move move = { pos, mColor };

        eColor(*board)[Board::COLS] = mBoard->GetBoard();
        const eColor temp = board[pos.Y][pos.X];
        board[pos.Y][pos.X] = mColor;
        if (mBoard->GetChainCount(move, dir) <= 3)
        {
            int32_t dx;
            int32_t dy;
            TranslateDirection(dir, &dx, &dy);

            const Vector2 frontPos = getEmptyPos(pos, dx, dy, mBoard->GetBoard());
            const Vector2 backPos = getEmptyPos(pos, -dx, -dy, mBoard->GetBoard());
            if (mBoard->IsPlaceable(frontPos) && mBoard->IsPlaceable(backPos)
                && (mSpotInfos[frontPos.Y][frontPos.X][(uint32_t)dir] == eSpotInfo::Closed4
                || mSpotInfos[frontPos.Y][frontPos.X][(uint32_t)dir] == eSpotInfo::Open4)
                && (mSpotInfos[backPos.Y][backPos.X][(uint32_t)dir] == eSpotInfo::Closed4
                || mSpotInfos[backPos.Y][backPos.X][(uint32_t)dir] == eSpotInfo::Open4))
            {
                count = 2;
            }
        }
        board[pos.Y][pos.X] = temp;
    }

    return (count >= 2);
}

bool BoardAnalyzer::IsThreeAndThree(const Vector2& pos) const
{
    uint32_t count = 0;
    for (uint32_t i = 0; i < (uint32_t)eDirection::Count; ++i)
    {
        const eSpotInfo spotInfo = mSpotInfos[pos.Y][pos.X][i];
        if (spotInfo == eSpotInfo::Open3)
        {
            ++count;
        }
        else if (spotInfo == eSpotInfo::Open5)
        {
            return false;
        }
    }

    return (count >= 2);
}