#include <assert.h>

#include "Board.h"
#include "BoardAnalyzer.h"

BoardAnalyzer::~BoardAnalyzer()
{
    Cleanup();
}

bool BoardAnalyzer::Initailize(Board* board)
{
    assert(board != nullptr);
    mBoard = board;

    return true;
}

void BoardAnalyzer::Cleanup()
{
}

void BoardAnalyzer::Update()
{
    Clear();

    calculateIllegalPos();
}

void BoardAnalyzer::Clear()
{
    mIllegalPosVect.clear();
    mOverLinesPosVect.clear();
    mFourAndFourPosVect.clear();
    mThreeAndThreePosVect.clear();
}

void BoardAnalyzer::calculateIllegalPos()
{
    size_t boardRows = mBoard->GetRows();
    size_t boardCols = mBoard->GetCols();

    eColor color = mBoard->GetCurTurn();

    for (size_t row = 0; row < boardRows; ++row)
    {
        for (size_t col = 0; col < boardCols; ++col)
        {
            if (!mBoard->IsPlaceable(row, col))
            {
                continue;
            }

            mBoard->SetColor(row, col, color);

            Vector2 pos;
            pos.X = col;
            pos.Y = row;

            // 돌을 뒀을 때 만들어지는 수 판단
            if (isOmok(row, col))
            {
                goto CONTINUE;
            }
            else if (IsFourAndFour(row, col, color))
            {
                mIllegalPosVect.push_back(pos);
                mFourAndFourPosVect.push_back(pos);
            }
            else if (isOverLines(row, col))
            {
                mIllegalPosVect.push_back(pos);
                mOverLinesPosVect.push_back(pos);
            }
            else if (IsThreeAndThree(row, col, color))
            {
                mIllegalPosVect.push_back(pos);
                mThreeAndThreePosVect.push_back(pos);
            }

        CONTINUE:
            mBoard->SetColor(row, col, eColor::None);
        }
    }
}

// 돌을 뒀을 때 5개가 만들어지면 오목
bool BoardAnalyzer::isOmok(const size_t row, const size_t col) const
{
    size_t horizontal = mBoard->GetChainCount(row, col, eColor::Black, eDirection::Horizontal);
    size_t vertical = mBoard->GetChainCount(row, col, eColor::Black, eDirection::Vertical);
    size_t leftDiagonal = mBoard->GetChainCount(row, col, eColor::Black, eDirection::LeftDiagonal);
    size_t rightDiagonal = mBoard->GetChainCount(row, col, eColor::Black, eDirection::RightDiagonal);

    return (horizontal == 5 || vertical == 5 || leftDiagonal == 5 || rightDiagonal == 5);
}

// 돌을 뒀을 때 6개 이상이 만들어지면 장목
bool BoardAnalyzer::isOverLines(const size_t row, const size_t col) const
{
    size_t horizontal = mBoard->GetChainCount(row, col, eColor::Black, eDirection::Horizontal);
    size_t vertical = mBoard->GetChainCount(row, col, eColor::Black, eDirection::Vertical);
    size_t leftDiagonal = mBoard->GetChainCount(row, col, eColor::Black, eDirection::LeftDiagonal);
    size_t rightDiagonal = mBoard->GetChainCount(row, col, eColor::Black, eDirection::RightDiagonal);

    return (horizontal >= 6 || vertical >= 6 || leftDiagonal >= 6 || rightDiagonal >= 6);
}

// 돌을 뒀을 때 열린/닫힌 4가 두 개 이상이면 44
bool BoardAnalyzer::IsFourAndFour(const size_t row, const size_t col, const eColor color) const
{
    size_t count = 0;
    count += getOpenClosedFourCount(row, col, eDirection::Horizontal);
    count += getOpenClosedFourCount(row, col, eDirection::Vertical);
    count += getOpenClosedFourCount(row, col, eDirection::LeftDiagonal);
    count += getOpenClosedFourCount(row, col, eDirection::RightDiagonal);

    return (count >= 2);
}

// 돌을 뒀을 때 열린 3이 두 개 이상이면 33
bool BoardAnalyzer::IsThreeAndThree(const size_t row, const size_t col, const eColor color) const
{
    size_t count = 0;
    count += IsOpenThree(row, col, eDirection::Horizontal, color);
    count += IsOpenThree(row, col, eDirection::Vertical, color);
    count += IsOpenThree(row, col, eDirection::LeftDiagonal, color);
    count += IsOpenThree(row, col, eDirection::RightDiagonal, color);

    return (count >= 2);
}

size_t BoardAnalyzer::getOpenClosedFourCount(const size_t row, const size_t col, const eDirection direction) const
{
    int dx = 0;
    int dy = 0;

    switch(direction)
    {
    case eDirection::Horizontal:
        dx = -1;
        dy = 0;
        break;
    case eDirection::Vertical:
        dx = 0;
        dy = -1;
        break;
    case eDirection::LeftDiagonal:
        dx = -1;
        dy = -1;
        break;
    case eDirection::RightDiagonal:
        dx = 1;
        dy = -1;
        break;
    default:
        assert(false);
        break;
    }

    size_t count = 0;

    // 정방향 검사
    size_t r = row + dy;
    size_t c = col + dx;
    while (mBoard->IsValidPos(r, c))
    {
        eColor color = mBoard->GetColor(r, c);
        if (color == eColor::None)
        {
            mBoard->SetColor(r, c, eColor::Black);
            size_t chainCount = mBoard->GetChainCount(row, col, eColor::Black, direction);
            mBoard->SetColor(r, c, eColor::None);

            if (chainCount == 5)
            {
                ++count;
            }

            break;
        }
        else if (color == eColor::White)
        {
            break;
        }

        r += dy;
        c += dx;
    }

    // 역방향 검사
    r = row - dy;
    c = col - dx;
    while (mBoard->IsValidPos(r, c))
    {
        eColor color = mBoard->GetColor(r, c);
        if (color == eColor::None)
        {
            mBoard->SetColor(r, c, eColor::Black);
            size_t chainCount = mBoard->GetChainCount(row, col, eColor::Black, direction);
            mBoard->SetColor(r, c, eColor::None);

            if (chainCount == 5)
            {
                ++count;
            }

            break;
        }
        else if (color == eColor::White)
        {
            break;
        }

        r -= dy;
        c -= dx;
    }

    // 열린 4일 경우
    if (count == 2)
    {
        // 검사 위치에 돌을 뒀을 때 이어지는 돌의 개수가 4개면 닫힌 4로 처리
        // => 다음과 같은 상황에 대한 예외처리
        // 1 2 3 4 5 6 7 8
        // O . O . O . O        => 4번 자리에서 44
        // O O . . O . O O      => 4번 자리에서 44
        // O . . O O . O        => 3번 자리에서 44
        if (mBoard->GetChainCount(row, col, eColor::Black, direction) == 4)
        {
            count = 1;
        }
    }

    return count;
}

// 돌을 순서대로 양측에 하나씩 뒀을 때 5개가 만들어지면 열린 4
bool BoardAnalyzer::IsOpenFour(const size_t row, const size_t col, const eDirection direction, const eColor color) const
{
    int dx = 0;
    int dy = 0;

    switch (direction)
    {
    case eDirection::Horizontal:
        dx = -1;
        dy = 0;
        break;
    case eDirection::Vertical:
        dx = 0;
        dy = -1;
        break;
    case eDirection::LeftDiagonal:
        dx = -1;
        dy = -1;
        break;
    case eDirection::RightDiagonal:
        dx = 1;
        dy = -1;
        break;
    default:
        assert(false);
        break;
    }

    size_t count = 0;

    // 정방향 검사
    size_t r = row + dy;
    size_t c = col + dx;
    while (mBoard->IsValidPos(r, c))
    {
        eColor otherColor = mBoard->GetColor(r, c);
        if (otherColor == eColor::None)
        {
            mBoard->SetColor(r, c, color);
            size_t chainCount = mBoard->GetChainCount(row, col, color, direction);
            mBoard->SetColor(r, c, eColor::None);

            if (chainCount == 5)
            {
                ++count;
            }

            break;
        }
        else if (otherColor != color)
        {
            break;
        }

        r += dy;
        c += dx;
    }

    // 역방향 검사
    r = row - dy;
    c = col - dx;
    while (mBoard->IsValidPos(r, c))
    {
        eColor otherColor = mBoard->GetColor(r, c);
        if (otherColor == eColor::None)
        {
            mBoard->SetColor(r, c, color);
            size_t chainCount = mBoard->GetChainCount(row, col, color, direction);
            mBoard->SetColor(r, c, eColor::None);

            if (chainCount == 5)
            {
                ++count;
            }

            break;
        }
        else if (otherColor != color)
        {
            break;
        }

        r -= dy;
        c -= dx;
    }

    return (count == 2);
}

// 열린 3은 하나를 더 뒀을 때 열린 4가 만들어져야 함
bool BoardAnalyzer::IsOpenThree(const size_t row, const size_t col, const eDirection direction, const eColor color) const
{
    int dx = 0;
    int dy = 0;

    switch (direction)
    {
    case eDirection::Horizontal:
        dx = -1;
        dy = 0;
        break;
    case eDirection::Vertical:
        dx = 0;
        dy = -1;
        break;
    case eDirection::LeftDiagonal:
        dx = -1;
        dy = -1;
        break;
    case eDirection::RightDiagonal:
        dx = 1;
        dy = -1;
        break;
    default:
        assert(false);
        break;
    }

    size_t count = 0;

    // 정방향 검사
    size_t r = row + dy;
    size_t c = col + dx;
    while (mBoard->IsValidPos(r, c))
    {
        eColor otherColor = mBoard->GetColor(r, c);
        if (otherColor == eColor::None)
        {
            mBoard->SetColor(r, c, color);

            // 열린 4 + 다른 금수가 없으면 33
            if (mBoard->GetChainCount(r, c, color, direction) >= 4
                && IsOpenFour(r, c, direction, color)
                && !isOverLines(r, c)
                && !IsFourAndFour(r, c, color)
                && !IsThreeAndThree(r, c, color))
            {
                ++count;
            }

            // 열린 4 + 다른 금수가 있어도
            // 막을 수 없는 수면 33
            if (IsOpenFour(r, c, direction, color) && isOmok(r, c))
            {
                ++count;
            }
            mBoard->SetColor(r, c, eColor::None);

            break;
        }
        else if (otherColor != color)
        {
            break;
        }

        r += dy;
        c += dx;
    }

    // 역방향 검사
    r = row - dy;
    c = col - dx;
    while (mBoard->IsValidPos(r, c))
    {
        eColor otherColor = mBoard->GetColor(r, c);
        if (otherColor == eColor::None)
        {
            mBoard->SetColor(r, c, color);

            // 열린 4 + 다른 금수가 없으면 33
            if (mBoard->GetChainCount(r, c, eColor::Black, direction) >= 4
                && IsOpenFour(r, c, direction, color)
                && !isOverLines(r, c)
                && !IsFourAndFour(r, c, color)
                && !IsThreeAndThree(r, c, color))
            {
                ++count;
            }

            // 열린 4 + 다른 금수가 있어도
            // 막을 수 없는 수면 33
            if (IsOpenFour(r, c, direction, color) && isOmok(r, c))
            {
                ++count;
            }
            mBoard->SetColor(r, c, eColor::None);

            break;
        }
        else if (otherColor != color)
        {
            break;
        }

        r -= dy;
        c -= dx;
    }

    return count >= 1;
}