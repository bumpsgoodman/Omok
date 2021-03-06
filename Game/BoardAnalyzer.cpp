#include <assert.h>
#include <cstdint>

#include "BoardAnalyzer.h"

BoardAnalyzer::BoardAnalyzer(Board* board, const eColor color)
    : mBoard(board)
    , mColor(color)
{
    assert(board != nullptr);
    assert(color != eColor::None);

    mOverLineVect.reserve(Board::ROWS * Board::COLS / 2);
    mFourAndFourVect.reserve(Board::ROWS * Board::COLS / 2);
    mThreeAndThreeVect.reserve(Board::ROWS * Board::COLS / 2);

    for (uint32_t i = 0; i < (uint32_t)eSpotInfo::Open6; ++i)
    {
        mSpotInfoVectMap[GetSpotInfoHash((eSpotInfo)i)].reserve(Board::ROWS * Board::COLS);
    }
}

void BoardAnalyzer::Clear()
{
    eColor(*board)[Board::COLS] = mBoard->GetBoard();

    mOverLineVect.clear();
    mFourAndFourVect.clear();
    mThreeAndThreeVect.clear();

    for (uint32_t i = (uint32_t)eSpotInfo::None; i <= (uint32_t)eSpotInfo::Open6; ++i)
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
    stackFrame.reserve(Board::ROWS * Board::COLS);
    stackFrame.push_back(pos);
    while (!stackFrame.empty())
    {
        const Vector2 updatePos = stackFrame.back();
        stackFrame.pop_back();

        if (!mBoard->IsValidPos(updatePos))
        {
            continue;
        }

        bool bUpdated = false;
        for (uint32_t i = 0; i < (uint32_t)eDirection::Count; ++i)
        {
            const eSpotInfo prevSpotInfo = mSpotInfos[updatePos.Y][updatePos.X][i];
            const eSpotInfo newSpotInfo = getSpotInfoRecursive(updatePos, (eDirection)i, board);
            if (newSpotInfo != prevSpotInfo)
            {
                removeSpotInfo(prevSpotInfo, updatePos);
                addSpotInfo(newSpotInfo, updatePos);
                mSpotInfos[updatePos.Y][updatePos.X][i] = newSpotInfo;
                bUpdated = true;
            }
        }

        updateIllegalMoveVect(updatePos);

        if (bUpdated)
        {
            stackFrame.push_back(getEmptyPos(updatePos, -1, 0, board));
            stackFrame.push_back(getEmptyPos(updatePos, 1, 0, board));
            stackFrame.push_back(getEmptyPos(updatePos, 0, -1, board));
            stackFrame.push_back(getEmptyPos(updatePos, 0, 1, board));
            stackFrame.push_back(getEmptyPos(updatePos, -1, -1, board));
            stackFrame.push_back(getEmptyPos(updatePos, 1, 1, board));
            stackFrame.push_back(getEmptyPos(updatePos, 1, -1, board));
            stackFrame.push_back(getEmptyPos(updatePos, -1, 1, board));
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
    const uint32_t key = GetSpotInfoHash(spotInfo);
    for (const Vector2& p : mSpotInfoVectMap[key])
    {
        if (p.X == pos.X && p.Y == pos.Y)
        {
            return;
        }
    }
    mSpotInfoVectMap[key].push_back(pos);
}

void BoardAnalyzer::removeSpotInfo(const eSpotInfo spotInfo, const Vector2& pos)
{
    const uint32_t key = GetSpotInfoHash(spotInfo);
    for (auto iter = mSpotInfoVectMap[key].begin(); iter != mSpotInfoVectMap[key].end(); ++iter)
    {
        if (iter->X == pos.X && iter->Y == pos.Y)
        {
            mSpotInfoVectMap[key].erase(iter);
            break;
        }
    }
}

// 1. pos에 돌을 뒀을 때 5개를 만들면 오목 (o5)
// 2. pos에 돌을 뒀을 때 6개 이상을 만들면 장목 (o6)
// 3. pos에 돌을 두고 양쪽에 돌을 뒀을 때 양쪽 모두 오목을 만들면 열린4 (o4)
// 4. pos에 돌을 두고 양쪽에 돌을 뒀을 때 한쪽만 오목을 만들면 닫힌4 (c4)
// 5. pos에 돌을 두고 양쪽 모두 돌을 둘 수 없다면 무의미한 수 (xx)
// 6. pos에 돌을 두고 한쪽에만 돌을 둘 수 있다면 열린/닫힌 (가장 큰 위치를 따름)
// 7. pos에 돌을 두고 양쪽에 돌을 뒀을 때
// 7.1. 양쪽 모두 금수라면 각 방향으로 한 번 더 돌을 두고 가장 큰 위치를 따름 (열린/닫힌)
// 7.2. 어느 한쪽이 금수이고 한 번 더 돌을 두고 금수라면 반대 방향을 따름 (열린/닫힌)
// 7.3. 어느 한쪽이 금수이고 한 번 더 돌을 두고 금수가 아니라면 가장 큰 위치를 따름 (열린/닫힌)
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
        if (frontResult == eSpotInfo::None && backResult == eSpotInfo::None)
        {
            result = eSpotInfo::None;
        }
        else
        {
            const eSpotInfo tempFrontSpot = mSpotInfos[frontPos.Y][frontPos.X][(uint32_t)direction];
            const eSpotInfo tempBackSpot = mSpotInfos[backPos.Y][backPos.X][(uint32_t)direction];
            mSpotInfos[frontPos.Y][frontPos.X][(uint32_t)direction] = frontResult;
            mSpotInfos[backPos.Y][backPos.X][(uint32_t)direction] = backResult;

            const bool bFrontIllegal = (isFourAndFour(frontPos) || isOverLines(frontPos) || isThreeAndThree(frontPos));
            const bool bBackIllegal = (isFourAndFour(backPos) || isOverLines(backPos) || isThreeAndThree(backPos));
            if (bFrontIllegal && bBackIllegal)
            {
                const Vector2 nextFrontPos = getEmptyPos(frontPos, dx, dy, board);
                const Vector2 nextBackPos = getEmptyPos(backPos, -dx, -dy, board);
                eSpotInfo nextFrontSpot = (mBoard->IsValidPos(nextFrontPos))
                    ? getSpotInfoRecursive(nextFrontPos, direction, board)
                    : eSpotInfo::None;
                eSpotInfo nextBackSpot = (mBoard->IsValidPos(nextBackPos))
                    ? getSpotInfoRecursive(nextBackPos, direction, board)
                    : eSpotInfo::None;

                const eSpotInfo tempNextFrontSpot = mSpotInfos[nextFrontPos.Y][nextFrontPos.X][(uint32_t)direction];
                mSpotInfos[nextFrontPos.Y][nextFrontPos.X][(uint32_t)direction] = nextFrontSpot;
                if (isFourAndFour(nextFrontPos) || isOverLines(nextFrontPos) || isThreeAndThree(nextFrontPos))
                {
                    nextFrontSpot = OpenToClosed(nextFrontSpot);
                }
                mSpotInfos[nextFrontPos.Y][nextFrontPos.X][(uint32_t)direction] = tempNextFrontSpot;

                const eSpotInfo tempNextBackSpot = mSpotInfos[nextBackPos.Y][nextBackPos.X][(uint32_t)direction];
                mSpotInfos[nextBackPos.Y][nextBackPos.X][(uint32_t)direction] = nextBackSpot;
                if (isFourAndFour(nextBackPos) || isOverLines(nextBackPos) || isThreeAndThree(nextBackPos))
                {
                    nextBackSpot = OpenToClosed(nextBackSpot);
                }
                mSpotInfos[nextBackPos.Y][nextBackPos.X][(uint32_t)direction] = tempNextBackSpot;

                result = MaxSpotInfo(nextFrontSpot, nextBackSpot) - 1;
            }
            else if (bFrontIllegal)
            {
                const Vector2 nextPos = getEmptyPos(frontPos, dx, dy, board);
                const eSpotInfo tempNextSpot = mSpotInfos[nextPos.Y][nextPos.X][(uint32_t)direction];
                const eSpotInfo nextSpot = (mBoard->IsValidPos(nextPos))
                    ? getSpotInfoRecursive(nextPos, direction, board)
                    : eSpotInfo::None;
                mSpotInfos[nextPos.Y][nextPos.X][(uint32_t)direction] = nextSpot;
                if (isFourAndFour(nextPos) || isOverLines(nextPos) || isThreeAndThree(nextPos))
                {
                    result = backResult - 1;
                }
                else
                {
                    result = MaxSpotInfo(MinSpotInfo(nextSpot, frontResult), backResult) - 1;
                }
                mSpotInfos[nextPos.Y][nextPos.X][(uint32_t)direction] = tempNextSpot;
            }
            else if (bBackIllegal)
            {
                const Vector2 nextPos = getEmptyPos(backPos, -dx, -dy, board);
                const eSpotInfo tempNextSpot = mSpotInfos[nextPos.Y][nextPos.X][(uint32_t)direction];
                const eSpotInfo nextSpot = (mBoard->IsValidPos(nextPos))
                    ? getSpotInfoRecursive(nextPos, direction, board)
                    : eSpotInfo::None;
                mSpotInfos[nextPos.Y][nextPos.X][(uint32_t)direction] = nextSpot;
                if (isFourAndFour(nextPos) || isOverLines(nextPos) || isThreeAndThree(nextPos))
                {
                    result = frontResult - 1;
                }
                else
                {
                    result = MaxSpotInfo(frontResult, MinSpotInfo(nextSpot, backResult)) - 1;
                }
                mSpotInfos[nextPos.Y][nextPos.X][(uint32_t)direction] = tempNextSpot;
            }
            else
            {
                result = MaxSpotInfo(frontResult, backResult) - 1;
            }

            mSpotInfos[frontPos.Y][frontPos.X][(uint32_t)direction] = tempFrontSpot;
            mSpotInfos[backPos.Y][backPos.X][(uint32_t)direction] = tempBackSpot;
        }
    }
    else if (frontChainCount != curChainCount && frontChainCount < 5)
    {
        const eSpotInfo frontResult = getSpotInfoRecursive(frontPos, direction, board);
        result = (frontResult == eSpotInfo::None) ? eSpotInfo::None : frontResult - 1;

        // 정방향을 기준으로 금수이고, 그 다음 위치도 금수라면 무의미 (xx)
        const eSpotInfo tempFrontSpot = mSpotInfos[frontPos.Y][frontPos.X][(uint32_t)direction];
        mSpotInfos[frontPos.Y][frontPos.X][(uint32_t)direction] = frontResult;
        if (isFourAndFour(frontPos) || isOverLines(frontPos) || isThreeAndThree(frontPos))
        {
            const Vector2 nextPos = getEmptyPos(frontPos, dx, dy, board);
            if (mBoard->IsValidPos(nextPos))
            {
                const eSpotInfo tempNextSpot = mSpotInfos[nextPos.Y][nextPos.X][(uint32_t)direction];
                mSpotInfos[nextPos.Y][nextPos.X][(uint32_t)direction] = getSpotInfoRecursive(nextPos, direction, board);
                if (isFourAndFour(nextPos) || isOverLines(nextPos) || isThreeAndThree(nextPos))
                {
                    result = eSpotInfo::None;
                }
                mSpotInfos[nextPos.Y][nextPos.X][(uint32_t)direction] = tempNextSpot;
            }
        }
        mSpotInfos[frontPos.Y][frontPos.X][(uint32_t)direction] = tempFrontSpot;
    }
    else if (backChainCount != curChainCount && backChainCount < 5)
    {
        const eSpotInfo backResult = getSpotInfoRecursive(backPos, direction, board);
        result = (backResult == eSpotInfo::None) ? eSpotInfo::None : backResult - 1;

        // 반대 방향을 기준으로 금수이고, 그 다음 위치도 금수라면 무의미 (xx)
        const eSpotInfo tempBackSpot = mSpotInfos[backPos.Y][backPos.X][(uint32_t)direction];
        mSpotInfos[backPos.Y][backPos.X][(uint32_t)direction] = backResult;
        if (isFourAndFour(backPos) || isOverLines(backPos) || isThreeAndThree(backPos))
        {
            const Vector2 nextPos = getEmptyPos(backPos, -dx, -dy, board);
            if (mBoard->IsValidPos(nextPos))
            {
                const eSpotInfo tempNextSpot = mSpotInfos[nextPos.Y][nextPos.X][(uint32_t)direction];
                mSpotInfos[nextPos.Y][nextPos.X][(uint32_t)direction] = getSpotInfoRecursive(nextPos, direction, board);
                if (isFourAndFour(nextPos) || isOverLines(nextPos) || isThreeAndThree(nextPos))
                {
                    result = eSpotInfo::None;
                }
                mSpotInfos[nextPos.Y][nextPos.X][(uint32_t)direction] = tempNextSpot;
            }
        }
        mSpotInfos[backPos.Y][backPos.X][(uint32_t)direction] = tempBackSpot;
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
    // => 현재 위치에 돌을 뒀을 때 이어지는 개수가 4개 미만이고 양쪽 모두 열린5가 만들어지면 44
    if (count == 1)
    {
        eColor(*board)[Board::COLS] = mBoard->GetBoard();
        const eColor temp = board[pos.Y][pos.X];
        board[pos.Y][pos.X] = mColor;
        if (mBoard->GetChainCount(pos, mColor, dir) < 4)
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