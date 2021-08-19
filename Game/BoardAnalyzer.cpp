#include <assert.h>
#include <cstdint>

#include "BoardAnalyzer.h"

BoardAnalyzer::BoardAnalyzer(Board* board, const eColor color)
    : mBoard(board)
    , mColor(color)
{
    assert(board != nullptr);
    assert(color != eColor::None);

    const uint32_t ROWS = Board::ROWS;
    const uint32_t COLS = Board::COLS;

    mOverLineVect.reserve(ROWS * COLS / 2);
    mFourAndFourVect.reserve(ROWS * COLS / 2);
    mThreeAndThreeVect.reserve(ROWS * COLS / 2);

    for (uint32_t i = 0; i < (uint32_t)eSpotInfo::Open6; ++i)
    {
        mSpotInfoVectMap[GetSpotInfoHash((eSpotInfo)i)].reserve(ROWS * COLS);
    }
}

void BoardAnalyzer::Clear()
{
    eColor(*board)[Board::COLS] = mBoard->GetBoard();

    mOverLineVect.clear();
    mFourAndFourVect.clear();
    mThreeAndThreeVect.clear();

    for (uint32_t i = 0; i < 11; ++i)
    {
        mSpotInfoVectMap[GetSpotInfoHash((eSpotInfo)i)].clear();
    }

    for (uint32_t y = 0; y < Board::ROWS; ++y)
    {
        for (uint32_t x = 0; x < Board::COLS; ++x)
        {
            const Vector2 pos = { x, y };
            for (uint32_t i = 0; i < (uint32_t)eDirection::Count; ++i)
            {
                const eSpotInfo spotInfo = getSpotInfoRecursive(pos, (eDirection)i, board);
                mSpotInfos[y][x][i] = spotInfo;
                addSpotInfo(spotInfo, pos);
            }

            updateIllegalMoveVect(pos);
        }
    }
}

void BoardAnalyzer::Update(const Vector2& pos)
{
    eColor(*board)[Board::COLS] = mBoard->GetBoard();

    std::vector<Vector2> stackFrame;

    stackFrame.push_back(pos);
    while (!stackFrame.empty())
    {
        const Vector2 updatedPos = stackFrame.back();
        stackFrame.pop_back();

        if (!mBoard->IsValidPos(updatedPos))
        {
            continue;
        }

        bool bUpdated = false;
        for (uint32_t i = 0; i < (uint32_t)eDirection::Count; ++i)
        {
            const eSpotInfo prevSpotInfo = mSpotInfos[updatedPos.Y][updatedPos.X][i];
            const eSpotInfo newSpotInfo = getSpotInfoRecursive(updatedPos, (eDirection)i, board);
            if (newSpotInfo != prevSpotInfo)
            {
                removeSpotInfo(prevSpotInfo, pos);
                addSpotInfo(newSpotInfo, pos);
                mSpotInfos[updatedPos.Y][updatedPos.X][i] = newSpotInfo;
                bUpdated = true;
            }
        }

        updateIllegalMoveVect(updatedPos);

        if (bUpdated)
        {
            for (uint32_t i = 0; i < (uint32_t)eDirection::Count; ++i)
            {
                int32_t dx;
                int32_t dy;
                TranslateDirection((eDirection)i, &dx, &dy);

                stackFrame.push_back(getEmptyPos(updatedPos, dx, dy, board));
                stackFrame.push_back(getEmptyPos(updatedPos, -dx, -dy, board));
            }
        }
    }
}

void BoardAnalyzer::updateIllegalMoveVect(const Vector2& pos)
{
    removePosFromIllegalMove(pos);

    if (isFourAndFour(pos))
    {
        mFourAndFourVect.push_back(pos);
    }
    else if (isOverLines(pos))
    {
        mOverLineVect.push_back(pos);
    }
    else if (isThreeAndThree(pos))
    {
        mThreeAndThreeVect.push_back(pos);
    }
}

void BoardAnalyzer::removePosFromIllegalMove(const Vector2& pos)
{
    for (auto iter = mFourAndFourVect.begin(); iter != mFourAndFourVect.end(); ++iter)
    {
        if (iter->X == pos.X && iter->Y == pos.Y)
        {
            mFourAndFourVect.erase(iter);
            break;
        }
    }

    for (auto iter = mOverLineVect.begin(); iter != mOverLineVect.end(); ++iter)
    {
        if (iter->X == pos.X && iter->Y == pos.Y)
        {
            mOverLineVect.erase(iter);
            break;
        }
    }

    for (auto iter = mThreeAndThreeVect.begin(); iter != mThreeAndThreeVect.end(); ++iter)
    {
        if (iter->X == pos.X && iter->Y == pos.Y)
        {
            mThreeAndThreeVect.erase(iter);
            break;
        }
    }
}

void BoardAnalyzer::addSpotInfo(const eSpotInfo spotInfo, const Vector2& pos)
{
    for (const Vector2& p : mSpotInfoVectMap[GetSpotInfoHash(spotInfo)])
    {
        if (p.X == pos.X && p.Y == pos.Y)
        {
            return;
        }
    }

    mSpotInfoVectMap[GetSpotInfoHash(spotInfo)].push_back(pos);
}

void BoardAnalyzer::removeSpotInfo(const eSpotInfo spotInfo, const Vector2& pos)
{
    for (auto iter = mSpotInfoVectMap[GetSpotInfoHash(spotInfo)].begin(); iter != mSpotInfoVectMap[GetSpotInfoHash(spotInfo)].end(); ++iter)
    {
        if (iter->X == pos.X && iter->Y == pos.Y)
        {
            mSpotInfoVectMap[GetSpotInfoHash(spotInfo)].erase(iter);
            break;
        }
    }
}

eSpotInfo BoardAnalyzer::getSpotInfoRecursive(const Vector2& pos, const eDirection direction, eColor(*board)[Board::COLS])
{
    if (!mBoard->IsValidPos(pos) || board[pos.Y][pos.X] != eColor::None)
    {
        return eSpotInfo::None;
    }

    board[pos.Y][pos.X] = mColor;

    const uint32_t curChainCount = mBoard->GetChainCount(pos, mColor, direction);
    uint32_t frontChainCount = curChainCount;
    uint32_t backChainCount = curChainCount;

    int32_t dx;
    int32_t dy;
    TranslateDirection(direction, &dx, &dy);

    const Vector2 frontPos = getEmptyPos(pos, dx, dy, board);
    if (mBoard->IsValidPos(frontPos))
    {
        board[frontPos.Y][frontPos.X] = mColor;
        frontChainCount = mBoard->GetChainCount(frontPos, mColor, direction);
        board[frontPos.Y][frontPos.X] = eColor::None;
    }

    const Vector2 backPos = getEmptyPos(pos, -dx, -dy, board);
    if (mBoard->IsValidPos(backPos))
    {
        board[backPos.Y][backPos.X] = mColor;
        backChainCount = mBoard->GetChainCount(backPos, mColor, direction);
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
    else if (frontChainCount != curChainCount && frontChainCount < 5
        && backChainCount != curChainCount && backChainCount < 5)
    {
        const eSpotInfo frontResult = getSpotInfoRecursive(frontPos, direction, board);
        const eSpotInfo backResult = getSpotInfoRecursive(backPos, direction, board);
        const eSpotInfo maxResult = MaxSpotInfo(frontResult, backResult);
        if (frontResult == eSpotInfo::None && backResult == eSpotInfo::None)
        {
            result = eSpotInfo::None;
        }
        else
        {
            // 1. 양쪽 모두 금수가 아님 = 열린
            // 2. 양쪽 모두 금수        = 닫힌
            // 3. 한쪽 금수 + 반대 닫힌 = 닫힌
            // 4. 한쪽 금수 + 반대 열린 = 열린

            // 백업
            const eSpotInfo tempFrontSpot = mSpotInfos[frontPos.Y][frontPos.X][(uint32_t)direction];
            const eSpotInfo tempBackSpot = mSpotInfos[backPos.Y][backPos.X][(uint32_t)direction];
            mSpotInfos[frontPos.Y][frontPos.X][(uint32_t)direction] = frontResult;
            mSpotInfos[backPos.Y][backPos.X][(uint32_t)direction] = backResult;

            bool bFrontIllegal = (isFourAndFour(frontPos) || isOverLines(frontPos) || isThreeAndThree(frontPos));
            bool bBackIllegal = (isFourAndFour(backPos) || isOverLines(backPos) || isThreeAndThree(backPos));
            if (!bFrontIllegal && !bBackIllegal)
            {
                result = maxResult - 1;
            }
            else if (bFrontIllegal && bBackIllegal)
            {
                result = MakeSpotInfo(max(frontChainCount - 1, backChainCount - 1), false);
            }
            else if (bFrontIllegal)
            {
                result = backResult - 1;
            }
            else if (bBackIllegal)
            {
                result = frontResult - 1;
            }

            //복원
            mSpotInfos[frontPos.Y][frontPos.X][(uint32_t)direction] = tempFrontSpot;
            mSpotInfos[backPos.Y][backPos.X][(uint32_t)direction] = tempBackSpot;
        }
    }
    else if (frontChainCount != curChainCount && frontChainCount < 5)
    {
        const eSpotInfo frontResult = getSpotInfoRecursive(frontPos, direction, board);
        result = (frontResult == eSpotInfo::None) ? eSpotInfo::None : frontResult - 1;
    }
    else if (backChainCount != curChainCount && backChainCount < 5)
    {
        const eSpotInfo backResult = getSpotInfoRecursive(backPos, direction, board);
        result = (backResult == eSpotInfo::None) ? eSpotInfo::None : backResult - 1;
    }
    else
    {
        result = eSpotInfo::None;
    }

    board[pos.Y][pos.X] = eColor::None;

    assert(result <= eSpotInfo::Open6);
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

bool BoardAnalyzer::isOverLines(const Vector2& pos) const
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

bool BoardAnalyzer::isFourAndFour(const Vector2& pos)
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

    // 같은 방향에서 열린/닫힌4가 두 개일 경우 44
    // 1. 현재 위치에 돌을 뒀을 때 이어지는 개수가 4개 미만이고
    // 2. 양쪽 모두 열린5가 만들어지면 44
    if (count == 1)
    {
        eColor(*board)[Board::COLS] = mBoard->GetBoard();
        const eColor temp = board[pos.Y][pos.X];
        board[pos.Y][pos.X] = mColor;
        if (mBoard->GetChainCount(pos, mColor, dir) <= 3)
        {
            int32_t dx;
            int32_t dy;
            TranslateDirection(dir, &dx, &dy);

            const Vector2 frontPos = getEmptyPos(pos, dx, dy, mBoard->GetBoard());
            const Vector2 backPos = getEmptyPos(pos, -dx, -dy, mBoard->GetBoard());
            if (mBoard->IsValidPos(frontPos) && mBoard->IsValidPos(backPos))
            {
                const eSpotInfo frontInfo = getSpotInfoRecursive(frontPos, dir, board);
                const eSpotInfo backInfo = getSpotInfoRecursive(backPos, dir, board);
                if (frontInfo == eSpotInfo::Open5 && backInfo == eSpotInfo::Open5)
                {
                    count = 2;
                }
            }
        }
        board[pos.Y][pos.X] = temp;
    }

    return (count >= 2);
}

bool BoardAnalyzer::isThreeAndThree(const Vector2& pos) const
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