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
    , mSpotInfoColor(eColor::None)
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

    // 돌, 돌 번호 그리기
    static const HFONT stoneCountFont = CreateFont((int)mStoneHeight / 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, DEFAULT_QUALITY, FIXED_PITCH, L"Consolas");
    SelectObject(hdc, stoneCountFont);
    wchar_t stoneCountText[10];
    uint32_t stoneCount = 1;
    eColor stoneColor = eColor::Black;
    for (const Vector2& pos : mNotationVect)
    {
        const uint32_t stoneX = (mBoardX - mStoneWidth / 2) + (mCellWidth * pos.X);
        const uint32_t stoneY = (mBoardY - mStoneHeight / 2) + (mCellHeight * pos.Y);
        const uint32_t stoneColorRGB = (stoneColor == eColor::Black) ? BLACK_STONE_COLOR : WHITE_STONE_COLOR;

        swprintf_s(stoneCountText, L"%u", stoneCount++);
        uint32_t textColor = (stoneColor == eColor::Black) ? WHITE_STONE_COLOR : BLACK_STONE_COLOR;
        SIZE textSize;
        const uint32_t textLen = lstrlen(stoneCountText);
        GetTextExtentPoint32(hdc, stoneCountText, textLen, &textSize);

        const uint32_t textX = stoneX + (mStoneWidth / 2) - textSize.cx / 2;
        const uint32_t textY = stoneY + (mStoneHeight / 2) - textSize.cy / 2;

        mDDraw->DrawRectangle(stoneX, stoneY, mStoneWidth, mStoneHeight, stoneColorRGB);
        mDDraw->PrintText(hdc, stoneCountText, textX, textY, textLen, textColor);

        stoneColor = (stoneColor == eColor::Black) ? eColor::White : eColor::Black;
    }

    // 금수 그리기
    const BoardAnalyzer* analyzer;
    const eSpotInfo(*spotInfos)[COLS][(uint32_t)eDirection::Count];
    if (mSpotInfoColor == eColor::None)
    {
        spotInfos = nullptr;
        analyzer = mBlackAnalyzer;
    }
    else if (mSpotInfoColor == eColor::Black)
    {
        spotInfos = mBlackAnalyzer->GetSpotInfos();
        analyzer = mBlackAnalyzer;
    }
    else
    {
        spotInfos = mWhiteAnalyzer->GetSpotInfos();
        analyzer = mWhiteAnalyzer;
    }

    for (const Vector2& pos : analyzer->GetOverLineVect())
    {
        drawIllegalMove(hdc, L"6", pos);
    }

    for (const Vector2& pos : analyzer->GetFourAndFourVect())
    {
        drawIllegalMove(hdc, L"44", pos);
    }

    for (const Vector2& pos : analyzer->GetThreeAndThreeVect())
    {
        drawIllegalMove(hdc, L"33", pos);
    }

    // 열린/닫힌 그리기
    if (spotInfos == nullptr)
    {
        goto EXIT;
    }

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

            for (uint32_t i = 0; i < (uint32_t)eDirection::Count; ++i)
            {
                const eSpotInfo spotInfo = spotInfos[y][x][i];
                if (spotInfo == eSpotInfo::None)
                {
                    swprintf_s(spotInfoText, L"xx");
                }
                else if (spotInfo >= eSpotInfo::Closed1 && spotInfo <= eSpotInfo::Closed4)
                {
                    swprintf_s(spotInfoText, L"c%u", (uint32_t)spotInfo);
                }
                else if (spotInfo >= eSpotInfo::Open1 && spotInfo <= eSpotInfo::Open6)
                {
                    swprintf_s(spotInfoText, L"o%u", (uint32_t)spotInfo - 4);
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

EXIT:
    mDDraw->EndGDI(hdc);
}

bool Board::IsPlaceable(const Vector2& pos) const
{
    if (!IsValidPos(pos))
    {
        return false;
    }

    if (mBoard[pos.Y][pos.X] != eColor::None)
    {
        return false;
    }

    if (mCurTurn == eColor::Black)
    {
        const std::vector<Vector2>& overLineVect = mBlackAnalyzer->GetOverLineVect();
        const std::vector<Vector2>& fourAndFourVect = mBlackAnalyzer->GetFourAndFourVect();
        const std::vector<Vector2>& threeAndThreeVect = mBlackAnalyzer->GetThreeAndThreeVect();
        std::vector<Vector2> illegalVect;

        illegalVect.insert(std::end(illegalVect), std::begin(overLineVect), std::end(overLineVect));
        illegalVect.insert(std::end(illegalVect), std::begin(fourAndFourVect), std::end(fourAndFourVect));
        illegalVect.insert(std::end(illegalVect), std::begin(threeAndThreeVect), std::end(threeAndThreeVect));
        for (const Vector2& illegalPos : illegalVect)
        {
            if (pos.X == illegalPos.X && pos.Y == illegalPos.Y)
            {
                return false;
            }
        }
    }

    return true;
}

bool Board::PlaceStone(const Vector2& pos)
{
    if (!IsPlaceable(pos))
    {
        return false;
    }

    mBoard[pos.Y][pos.X] = mCurTurn;
    changeTurn();

    mNotationVect.push_back(pos);
    mRedoVect.clear();

    mBlackAnalyzer->Update(pos);
    mWhiteAnalyzer->Update(pos);

    return true;
}

Vector2 Board::GetLastPlacedPos() const
{
    assert(!mNotationVect.empty());
    return mNotationVect.back();
}

bool Board::HasWon(const eColor color) const
{
    assert(color != eColor::None);

    const Vector2 pos = GetLastPlacedPos();
    return (GetChainCount(pos, color, eDirection::Horizontal) >= 5
        || GetChainCount(pos, color, eDirection::Vertical) >= 5
        || GetChainCount(pos, color, eDirection::LeftDiagonal) >= 5
        || GetChainCount(pos, color, eDirection::RightDiagonal) >= 5);
}

uint32_t Board::GetChainCount(const Vector2& pos, const eColor color, const eDirection direction) const
{
    uint32_t count = 0;

    int32_t dx = 0;
    int32_t dy = 0;
    TranslateDirection(direction, &dx, &dy);

    // 현위치 + 정방향
    Vector2 frontPos = { pos.X, pos.Y };
    while (IsValidPos(frontPos) && mBoard[frontPos.Y][frontPos.X] == color)
    {
        ++count;
        frontPos.Y += dy;
        frontPos.X += dx;
    }

    // 반대 방향
    Vector2 backPos = { pos.X - dx, pos.Y - dy };
    while (IsValidPos(backPos) && mBoard[backPos.Y][backPos.X] == color)
    {
        ++count;
        backPos.Y -= dy;
        backPos.X -= dx;
    }

    return count;
}

void Board::ClearBoard()
{
    memset(mBoard, (uint32_t)eColor::None, ROWS * COLS * sizeof(eColor));
    mNotationVect.clear();

    mBlackAnalyzer->Clear();
    mWhiteAnalyzer->Clear();

    mCurTurn = eColor::Black;
    const Vector2 pos = { COLS / 2, ROWS / 2 };
    PlaceStone(pos);
}

bool Board::Undo()
{
    if (!CanUndo())
    {
        return false;
    }

    const Vector2& pos = mNotationVect.back();
    mNotationVect.pop_back();
    mRedoVect.push_back(pos);

    mBoard[pos.Y][pos.X] = eColor::None;
    changeTurn();

    mBlackAnalyzer->Update(pos);
    mWhiteAnalyzer->Update(pos);

    return true;
}

bool Board::Redo()
{
    if (!CanRedo())
    {
        return false;
    }

    const Vector2& pos = mRedoVect.back();
    mRedoVect.pop_back();
    mNotationVect.push_back(pos);

    mBoard[pos.Y][pos.X] = mCurTurn;
    changeTurn();

    mBlackAnalyzer->Update(pos);
    mWhiteAnalyzer->Update(pos);

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

    for (const Vector2& pos : mNotationVect)
    {
        fwrite(&pos, sizeof(Vector2), 1, fp);
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

    Vector2 pos;
    fread(&pos, sizeof(Vector2), 1, fp);  // 첫 수 버리기
    while (fread(&pos, sizeof(Vector2), 1, fp) == 1)
    {
        mBoard[pos.Y][pos.X] = mCurTurn;
        mNotationVect.push_back(pos);
        changeTurn();

        mBlackAnalyzer->Update(pos);
        mWhiteAnalyzer->Update(pos);
    }

    mBlackAnalyzer->Clear();
    mWhiteAnalyzer->Clear();

    fclose(fp);

    return true;
}

Vector2 Board::MousePosToBoardPos(const uint32_t mouseX, const uint32_t mouseY) const
{
    Vector2 pos;
    pos.X = (mouseX - (mBoardX - mCellWidth / 2)) / mCellWidth;
    pos.Y = (mouseY - (mBoardY - mCellHeight / 2)) / mCellHeight;
    return pos;
}

const BoardAnalyzer* Board::GetAnalyzer(const eColor color) const
{
    assert(color != eColor::None);
    return (color == eColor::Black) ? mBlackAnalyzer : mWhiteAnalyzer;
}
     
void Board::drawIllegalMove(HDC hdc, const wchar_t* text, const Vector2& pos) const
{
    SIZE size;
    const uint32_t len = lstrlen(text);
    GetTextExtentPoint32(hdc, text, len, &size);

    const uint32_t stoneX = (mBoardX - mStoneWidth / 2) + (mCellWidth * pos.X);
    const uint32_t stoneY = (mBoardY - mStoneHeight / 2) + (mCellHeight * pos.Y);
    const uint32_t illegalTextX = stoneX + (mStoneWidth / 2) - size.cx / 2;
    const uint32_t illegalTextY = stoneY + (mStoneHeight / 2) - size.cy / 2;

    mDDraw->DrawRectangle(stoneX, stoneY, mStoneWidth, mStoneHeight, ILLEGAL_MOVE_COLOR);
    mDDraw->PrintText(hdc, text, illegalTextX, illegalTextY, len, ILLEGAL_MOVE_TEXT_COLOR);
}