#define _CRT_SECURE_NO_WARNINGS

#include <assert.h>
#include <cstdint>
#include <stdio.h>

#include "Board.h"
#include "BoardAnalyzer.h"
#include "../Util/Event.h"
#include "../Util/SafeDelete.h"

Board::Board(DDraw* ddraw)
    : mDDraw(ddraw)
{
    assert(ddraw != nullptr);

    const uint32_t screenWidth = ddraw->GetWidth();
    const uint32_t screenHeight = ddraw->GetHeight();

    mBoardWidth = static_cast<uint32_t>(screenWidth * BOARD_RATIO) - BOARD_PADDING;
    mBoardHeight = screenHeight - BOARD_PADDING;

    mCellWidth = mBoardWidth / (COLS - 1);
    mCellHeight = mBoardHeight / (ROWS - 1);
    mStoneWidth = static_cast<uint32_t>(mCellWidth / 1.3f);
    mStoneHeight = static_cast<uint32_t>(mCellHeight / 1.3f);
    mCenterPointWidth = mStoneWidth / 3;
    mCenterPointHeight = mStoneHeight / 3;

    mBoardWidth = mCellWidth * (COLS - 1);
    mBoardHeight = mCellHeight * (ROWS - 1);

    mBoardX = BOARD_PADDING / 2;
    mBoardY = (screenHeight - mBoardHeight) / 2;

    mNotationVect.reserve(ROWS* COLS);
    mRedoVect.reserve(ROWS* COLS);

    mBlackAnalyzer = new BoardAnalyzer(this, eColor::Black);
    mWhiteAnalyzer = new BoardAnalyzer(this, eColor::White);

    ClearBoard();
}

Board::~Board()
{
    SAFE_DELETE(mBlackAnalyzer);
    SAFE_DELETE(mWhiteAnalyzer);
}

void Board::Draw() const
{
    HDC hdc;
    mDDraw->BeginGDI(&hdc);

    // 보드판 그리기
    mDDraw->DrawRectangle(mBoardX, mBoardY, mBoardWidth, mBoardHeight, BOARD_COLOR);

    // 보드판 수직선, 수평선 그리기
    for (uint32_t i = 0; i < COLS; ++i)
    {
        mDDraw->DrawRectangle(mBoardX + mCellWidth * i, mBoardY, 1, mBoardHeight, LINE_COLOR);
        mDDraw->DrawRectangle(mBoardX, mBoardY + mCellHeight * i, mBoardWidth, 1, LINE_COLOR);
    }

    // 중앙점 그리기
    const uint32_t centerPointX = mBoardX + (mCellWidth * ((COLS - 1) / 2)) - (mCenterPointWidth / 2);
    const uint32_t centerPointY = mBoardY + (mCellHeight * ((ROWS - 1) / 2)) - (mCenterPointHeight / 2);
    const uint32_t quadrant1CenterPointX = centerPointX + mCellWidth * ((COLS - 1) / 3);
    const uint32_t quadrant1CenterPointY = centerPointY - mCellHeight * ((ROWS - 1) / 3);
    const uint32_t quadrant2CenterPointX = centerPointX - mCellWidth * ((COLS - 1) / 3);
    const uint32_t quadrant2CenterPointY = centerPointY - mCellHeight * ((ROWS - 1) / 3);
    const uint32_t quadrant3CenterPointX = centerPointX - mCellWidth * ((COLS - 1) / 3);
    const uint32_t quadrant3CenterPointY = centerPointY + mCellHeight * ((ROWS - 1) / 3);
    const uint32_t quadrant4CenterPointX = centerPointX + mCellWidth * ((COLS - 1) / 3);
    const uint32_t quadrant4CenterPointY = centerPointY + mCellHeight * ((ROWS - 1) / 3);

    mDDraw->DrawRectangle(centerPointX, centerPointY, mCenterPointWidth, mCenterPointHeight, CENTER_POINT_COLOR);
    mDDraw->DrawRectangle(quadrant1CenterPointX, quadrant1CenterPointY, mCenterPointWidth, mCenterPointHeight, CENTER_POINT_COLOR);
    mDDraw->DrawRectangle(quadrant2CenterPointX, quadrant2CenterPointY, mCenterPointWidth, mCenterPointHeight, CENTER_POINT_COLOR);
    mDDraw->DrawRectangle(quadrant3CenterPointX, quadrant3CenterPointY, mCenterPointWidth, mCenterPointHeight, CENTER_POINT_COLOR);
    mDDraw->DrawRectangle(quadrant4CenterPointX, quadrant4CenterPointY, mCenterPointWidth, mCenterPointHeight, CENTER_POINT_COLOR);

    // 돌 그리기
    static const HFONT stoneCountFont = CreateFont((int)mStoneHeight / 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, DEFAULT_QUALITY, FIXED_PITCH, L"Consolas");
    SelectObject(hdc, stoneCountFont);
    wchar_t stoneCountText[10];
    uint32_t stoneCount = 1;
    for (const Move& move : mNotationVect)
    {
        const uint32_t stoneX = (mBoardX - mStoneWidth / 2) + (mCellWidth * move.Pos.X);
        const uint32_t stoneY = (mBoardY - mStoneHeight / 2) + (mCellHeight * move.Pos.Y);
        const eColor stoneColor = move.Color;
        switch (stoneColor)
        {
        case eColor::Black:
            mDDraw->DrawRectangle(stoneX, stoneY, mStoneWidth, mStoneHeight, BLACK_STONE_COLOR);
            break;
        case eColor::White:
            mDDraw->DrawRectangle(stoneX, stoneY, mStoneWidth, mStoneHeight, WHITE_STONE_COLOR);
            break;
        default:
            break;
        }

        // 돌 번호 그리기
        swprintf_s(stoneCountText, L"%u", stoneCount++);

        uint32_t textColor = (stoneColor == eColor::Black) ? WHITE_STONE_COLOR : BLACK_STONE_COLOR;
        SIZE textSize;
        const uint32_t textLen = lstrlen(stoneCountText);
        GetTextExtentPoint32(hdc, stoneCountText, textLen, &textSize);

        const uint32_t textX = stoneX + (mStoneWidth / 2) - textSize.cx / 2;
        const uint32_t textY = stoneY + (mStoneHeight / 2) - textSize.cy / 2;

        mDDraw->PrintText(hdc, stoneCountText, textX, textY, textLen, textColor);
    }

    // 열린/닫힌 그리기
    const eSpotInfo(*spotInfos)[COLS][(uint32_t)eDirection::Count] = mBlackAnalyzer->GetSpotInfos();
    static const HFONT spotInfoFont = CreateFont((int)mStoneHeight / 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, DEFAULT_QUALITY, FIXED_PITCH, L"Consolas");
    SelectObject(hdc, spotInfoFont);
    wchar_t spotInfoText[20];
    for (uint32_t y = 0; y < ROWS; ++y)
    {
        for (uint32_t x = 0; x < COLS; ++x)
        {
            const uint32_t stoneX = (mBoardX - mStoneWidth / 2) + (mCellWidth * x);
            const uint32_t stoneY = (mBoardY - mStoneHeight / 2) + (mCellHeight * y);

            uint32_t textColor = (mBoard[y][x] == eColor::Black) ? 0xffffff : 0x000000;

            const Vector2 pos = { x, y };
            if (mBlackAnalyzer->IsFourAndFour(pos))
            {
                drawIllegalMove(hdc, L"44", stoneX, stoneY);
            }
            else if (mBlackAnalyzer->IsOverLines(pos))
            {
                drawIllegalMove(hdc, L"66", stoneX, stoneY);
            }
            else if (mBlackAnalyzer->IsThreeAndThree(pos))
            {
                drawIllegalMove(hdc, L"33", stoneX, stoneY);
            }

            for (uint32_t i = 0; i < (uint32_t)eDirection::Count; ++i)
            {
                const eSpotInfo spotInfo = spotInfos[y][x][i];
                if (spotInfo == eSpotInfo::None)
                {
                    swprintf_s(spotInfoText, L"xx");
                }
                else if (spotInfo >= eSpotInfo::Open1 && spotInfo <= eSpotInfo::Open6)
                {
                    swprintf_s(spotInfoText, L"o%u", spotInfo);
                }
                else if (spotInfo >= eSpotInfo::Closed1 && spotInfo <= eSpotInfo::Closed4)
                {
                    swprintf_s(spotInfoText, L"c%u", (uint32_t)spotInfo - 6);
                }
                else
                {
                    assert(false);
                }

                const uint32_t textLen = lstrlen(spotInfoText);
                SIZE textSize;
                GetTextExtentPoint32(hdc, spotInfoText, textLen, &textSize);

                uint32_t textX;
                uint32_t textY;
                switch ((eDirection)i)
                {
                case eDirection::Horizontal:
                    textX = stoneX;
                    textY = stoneY + mStoneHeight - textSize.cy;
                    break;
                case eDirection::Vertical:
                    textX = stoneX + mStoneWidth - textSize.cx;
                    textY = stoneY + mStoneHeight - textSize.cy;
                    break;
                case eDirection::LeftDiagonal:
                    textX = stoneX;
                    textY = stoneY;
                    break;
                case eDirection::RightDiagonal:
                    textX = stoneX + mStoneWidth - textSize.cx;
                    textY = stoneY;
                    break;
                default:
                    assert(false);
                    break;
                }
                
                mDDraw->PrintText(hdc, spotInfoText, textX, textY, textLen, textColor);
            }
        }
    }

    mDDraw->EndGDI(hdc);
}

bool Board::IsPlaceable(const Vector2& pos) const
{
    return (IsValidPos(pos) && mBoard[pos.Y][pos.X] == eColor::None);
}

bool Board::PlaceStone(const Move& move)
{
    if (!IsPlaceable(move.Pos))
    {
        return false;
    }

    mBoard[move.Pos.Y][move.Pos.X] = move.Color;

    mNotationVect.push_back(move);
    mRedoVect.clear();

    mBlackAnalyzer->Update(move);
    mWhiteAnalyzer->Update(move);

    return true;
}

// 마지막에 둔 돌이 없다면
// pos.X = UINT32_MAX
// pos.Y = UINT32_MAX
// color = eColor::None
Move Board::GetLastMove() const
{
    if (mNotationVect.empty())
    {
        Move move = { { UINT32_MAX, UINT32_MAX }, eColor::None };
        return move;
    }

    return mNotationVect.back();
}

uint32_t Board::GetChainCount(const Move& move, const eDirection direction) const
{
    int32_t dx = 0;
    int32_t dy = 0;
    Vector2 pos;
    uint32_t count = 0;

    TranslateDirection(direction, &dx, &dy);

    // 현위치 + 정방향
    pos.X = move.Pos.X;
    pos.Y = move.Pos.Y;
    while (IsValidPos(pos) && mBoard[pos.Y][pos.X] == move.Color)
    {
        ++count;
        pos.Y += dy;
        pos.X += dx;
    }

    // 역방향
    pos.X = move.Pos.X - dx;
    pos.Y = move.Pos.Y - dy;
    while (IsValidPos(pos) && mBoard[pos.Y][pos.X] == move.Color)
    {
        ++count;
        pos.Y -= dy;
        pos.X -= dx;
    }

    return count;
}

bool Board::HasWon(const eColor color) const
{
    assert(color != eColor::None);

    const Move move = { GetLastMove().Pos, color };
    return (GetChainCount(move, eDirection::Horizontal) >= 5
        || GetChainCount(move, eDirection::Vertical) >= 5
        || GetChainCount(move, eDirection::LeftDiagonal) >= 5
        || GetChainCount(move, eDirection::RightDiagonal) >= 5);
}

void Board::ClearBoard()
{
    memset(mBoard, static_cast<int>(eColor::None), ROWS * COLS * sizeof(eColor));
    mNotationVect.clear();

    mBlackAnalyzer->Clear();
    mWhiteAnalyzer->Clear();
}

bool Board::Undo()
{
    if (!CanUndo())
    {
        return false;
    }

    const Move& move = mNotationVect.back();
    mNotationVect.pop_back();
    mRedoVect.push_back(move);

    mBoard[move.Pos.Y][move.Pos.X] = eColor::None;

    mBlackAnalyzer->Update(move);
    mWhiteAnalyzer->Update(move);

    return true;
}

bool Board::Redo()
{
    if (!CanRedo())
    {
        return false;
    }

    const Move& move = mRedoVect.back();
    mRedoVect.pop_back();
    mNotationVect.push_back(move);

    mBoard[move.Pos.Y][move.Pos.X] = move.Color;

    mBlackAnalyzer->Update(move);
    mWhiteAnalyzer->Update(move);

    return true;
}

bool Board::SaveNotation() const
{
    FILE* fp = fopen("./Notation/notation.nt", "wb");
    if (fp == nullptr)
    {
        assert(false);
        return false;
    }

    for (const Move& move : mNotationVect)
    {
        fwrite(&move, sizeof(Move), 1, fp);
    }

    fclose(fp);

    return true;
}

bool Board::LoadNotation()
{
    FILE* fp = fopen("./Notation/notation.nt", "rb");
    if (fp == nullptr)
    {
        assert(false);
        return false;
    }

    ClearBoard();

    Move move;
    while (fread(&move, sizeof(Move), 1, fp) == 1)
    {
        mBoard[move.Pos.Y][move.Pos.X] = move.Color;
        mNotationVect.push_back(move);
    }

    mBlackAnalyzer->Update(move);
    mWhiteAnalyzer->Update(move);

    fclose(fp);

    return true;
}

Vector2 Board::MousePosToBoardPos() const
{
    Vector2 pos;
    pos.X = (event::mouse::GetX() - (mBoardX - mCellWidth / 2)) / mCellWidth;
    pos.Y = (event::mouse::GetY() - (mBoardY - mCellHeight / 2)) / mCellHeight;
    return pos;
}

void Board::drawIllegalMove(HDC hdc, const wchar_t* text, const uint32_t x, const uint32_t y) const
{
    SIZE size;
    const uint32_t len = lstrlen(text);
    GetTextExtentPoint32(hdc, text, len, &size);

    const uint32_t illegalTextX = x + (mStoneWidth / 2) - size.cx / 2;
    const uint32_t illegalTextY = y + (mStoneHeight / 2) - size.cy / 2;

    mDDraw->DrawRectangle(x, y, mStoneWidth, mStoneHeight, ILLEGAL_MOVE_COLOR);
    mDDraw->PrintText(hdc, text, illegalTextX, illegalTextY, len, ILLEGAL_MOVE_TEXT_COLOR);
}