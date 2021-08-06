#define _CRT_SECURE_NO_WARNINGS

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "AI.h"
#include "Board.h"
#include "BoardAnalyzer.h"
#include "../Util/MouseEvent.h"
#include "../Util/SafeDelete.h"

Board::Board(const Board& other)
    : mbBlackWon(other.mbBlackWon)
    , mbWhiteWon(other.mbWhiteWon)
    , mCurTurn(other.mCurTurn)
    , mLastPlacedRow(other.mLastPlacedRow)
    , mLastPlacedCol(other.mLastPlacedCol)
    , mStoneCount(other.mStoneCount)
{
    mBoard = new eColor[ROWS * COLS];
    memcpy(mBoard, other.mBoard, ROWS * COLS * sizeof(eColor));

    mAnalyzer = new BoardAnalyzer;
    mAnalyzer->Initailize(this);
}

Board::~Board()
{
    Cleanup();
}

bool Board::Initailize(DDraw* ddraw)
{
    assert(ddraw != nullptr);

    mDDraw = ddraw;
    mBoard = new eColor[ROWS * COLS];

    UpdateBoardSize();

    mAnalyzer = new BoardAnalyzer;
    if (!mAnalyzer->Initailize(this))
    {
        return false;
    }

    Clear();

    srand((unsigned int)GetTickCount64());
    mPlayerColor = static_cast<eColor>(rand() % 2 + 1);
    mAiColor = (mPlayerColor == eColor::Black) ? eColor::White : eColor::Black;

    mAI = new AI(this, mAiColor, mPlayerColor);

    Vector2 pos = { COLS / 2, ROWS / 2 };
    PlaceStone(pos.Y, pos.X, eColor::Black);
    mNotationVect.push_back(pos);

    return true;
}

void Board::Cleanup()
{
    SAFE_DELETE(mAnalyzer);
    SAFE_DELETE_ARRAY(mBoard);
}

void Board::Update()
{
    if (HasWon())
    {
        return;
    }

    Vector2 pos = {};
    if (mCurTurn == mPlayerColor)
    {
        pos.Y = TranslateToBoardY(MouseEvent::GetY());
        pos.X = TranslateToBoardX(MouseEvent::GetX());
    }
    else
    {
        //pos = mAI->GetBestMovePos();
        if (pos.X == -1 || pos.Y == -1)
        {
            exit(1);
        }
        pos.Y = TranslateToBoardY(MouseEvent::GetY());
        pos.X = TranslateToBoardX(MouseEvent::GetX());
    }

    if (PlaceStone(pos.Y, pos.X, mCurTurn))
    {
        mNotationVect.push_back(pos);

        mUndoStack.push(pos);
        while (!mRedoStack.empty())
        {
            mRedoStack.pop();
        }
    }
}

void Board::Draw() const
{
    assert(mDDraw != nullptr);

    HDC hdc;
    mDDraw->BeginGDI(&hdc);

    HFONT oldFont = (HFONT)SelectObject(hdc, mDefaultBoardFont);

    size_t screenWidth = mDDraw->GetWidth();
    size_t screenHeight = mDDraw->GetHeight();
    
    // 보드판 틀 그리기
    mDDraw->DrawRectangle(mBoardX, mBoardY, mCellWidth * (COLS - 1), mCellHeight * (ROWS - 1), WOOD_COLOR);

    // 보드판 수직선 그리기
    for (size_t x = 0; x < COLS; ++x)
    {
        mDDraw->DrawRectangle(mBoardX + mCellWidth * x, mBoardY, 1, mCellHeight * (ROWS - 1), LINE_COLOR);
    }

    // 보드판 수평선 그리기
    for (size_t y = 0; y < ROWS; ++y)
    {
        mDDraw->DrawRectangle(mBoardX, mBoardY + mCellHeight * y, mCellWidth * (COLS - 1), 1, LINE_COLOR);
    }

    // 돌 그리기
    for (size_t y = 0; y < ROWS; ++y)
    {
        for (size_t x = 0; x < COLS; ++x)
        {
            int stoneX = mBoardX + (x * mCellWidth) - mStoneWidth / 2;
            int stoneY = mBoardY + (y * mCellHeight) - mStoneHeight / 2;

            switch (GetColor(y, x))
            {
            case eColor::White:
                mDDraw->DrawRectangle(stoneX, stoneY, mStoneWidth, mStoneHeight, WHITE_STONE_COLOR);
                break;
            case eColor::Black:
                mDDraw->DrawRectangle(stoneX, stoneY, mStoneWidth, mStoneHeight, BLACK_STONE_COLOR);
                break;
            default:
                break;
            }
        }
    }

    // 돌 번호 그리기
    if (!mNotationVect.empty())
    {
        unsigned int fontColor = WHITE_STONE_COLOR;
        for (size_t i = 0; i < mStoneCount; ++i)
        {
            wchar_t text[10];
            swprintf_s(text, L"%u", i + 1);

            SIZE textSize;
            size_t len = lstrlen(text);
            GetTextExtentPoint32(hdc, text, len, &textSize);

            const Vector2& v = mNotationVect.at(i);

            int textX = mBoardX + (v.X * mCellWidth) - textSize.cx / 2;
            int textY = mBoardY + (v.Y * mCellHeight) - textSize.cy / 2;

            mDDraw->PrintText(hdc, text, textX, textY, lstrlen(text), fontColor);

            fontColor = (fontColor == WHITE_STONE_COLOR) ? BLACK_STONE_COLOR : WHITE_STONE_COLOR;
        }
    }

    // 장목 그리기
    for (const Vector2& v : mAnalyzer->GetOverLinesPosVect())
    {
        const wchar_t* text = L"6";
        SIZE textSize;
        size_t len = lstrlen(text);
        GetTextExtentPoint32(hdc, text, len, &textSize);

        int boxX = mBoardX + (v.X * mCellWidth) - mStoneWidth / 2;
        int boxY = mBoardY + (v.Y * mCellHeight) - mStoneHeight / 2;

        int textX = mBoardX + (v.X * mCellWidth) - textSize.cx / 2;
        int textY = mBoardY + (v.Y * mCellHeight) - textSize.cy / 2;

        mDDraw->DrawRectangle(boxX, boxY, mStoneWidth, mStoneHeight, ILLEGAL_BOX_COLOR);
        mDDraw->PrintText(hdc, text, textX, textY, len, 0x000000U);
    }

    // 44 그리기
    for (const Vector2& v : mAnalyzer->GetFourAndFourPosVect())
    {
        const wchar_t* text = L"44";
        SIZE textSize;
        size_t len = lstrlen(text);
        GetTextExtentPoint32(hdc, text, len, &textSize);

        int boxX = mBoardX + (v.X * mCellWidth) - mStoneWidth / 2;
        int boxY = mBoardY + (v.Y * mCellHeight) - mStoneHeight / 2;

        int textX = mBoardX + (v.X * mCellWidth) - textSize.cx / 2;
        int textY = mBoardY + (v.Y * mCellHeight) - textSize.cy / 2;

        mDDraw->DrawRectangle(boxX, boxY, mStoneWidth, mStoneHeight, ILLEGAL_BOX_COLOR);
        mDDraw->PrintText(hdc, text, textX, textY, len, 0x000000U);
    }

    // 33 그리기
    for (const Vector2& v : mAnalyzer->GetThreeAndThreePosVect())
    {
        const wchar_t* text = L"33";
        SIZE textSize;
        size_t len = lstrlen(text);
        GetTextExtentPoint32(hdc, text, len, &textSize);

        int boxX = mBoardX + (v.X * mCellWidth) - mStoneWidth / 2;
        int boxY = mBoardY + (v.Y * mCellHeight) - mStoneHeight / 2;

        int textX = mBoardX + (v.X * mCellWidth) - textSize.cx / 2;
        int textY = mBoardY + (v.Y * mCellHeight) - textSize.cy / 2;

        mDDraw->DrawRectangle(boxX, boxY, mStoneWidth, mStoneHeight, ILLEGAL_BOX_COLOR);
        mDDraw->PrintText(hdc, text, textX, textY, len, 0x000000U);
    }

    // 승패 그리기
    SelectObject(hdc, mLargeBoardFont);

    const wchar_t* whoWonText = (mbBlackWon) ? L"Black Won" : (mbWhiteWon) ? L"White Won" : L"";
    size_t whoWonTextLen = lstrlen(whoWonText);
    SIZE whoWonTextSize;
    GetTextExtentPoint32(hdc, whoWonText, whoWonTextLen, &whoWonTextSize);

    const size_t whoWonTextX = (mBoardX + mBoardWidth / 2) - (whoWonTextSize.cx / 2);
    const size_t whoWonTextY = (mBoardY / 2) - (whoWonTextSize.cy / 2);
    mDDraw->PrintText(hdc, whoWonText, whoWonTextX, whoWonTextY, whoWonTextLen, 0xca2029U);

    SelectObject(hdc, oldFont);

    mDDraw->EndGDI(hdc);
}

void Board::UpdateBoardSize()
{
    assert(mDDraw != nullptr);

    size_t screenWidth = mDDraw->GetWidth();
    size_t screenHeight = mDDraw->GetHeight();

    size_t squareSize = (size_t)((screenWidth + screenHeight) / 2.75f);

    mBoardWidth = squareSize;
    mBoardHeight = squareSize;
    mCellWidth = mBoardWidth / COLS;
    mCellHeight = mBoardHeight / ROWS;
    mStoneWidth = (size_t)(mCellWidth / 1.5f);
    mStoneHeight = (size_t)(mCellHeight / 1.5f);
    mBoardX = mCellWidth / 2;
    mBoardY = (screenHeight - mBoardHeight + mCellHeight) / 2;

    mDefaultBoardFont = CreateFont((int)(mStoneHeight * 0.75), 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, DEFAULT_QUALITY, FIXED_PITCH, L"Consolas");
    mLargeBoardFont = CreateFont((int)(mBoardHeight * 0.1), 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, DEFAULT_QUALITY, FIXED_PITCH, L"Consolas");
}

eColor Board::GetColor(const size_t row, const size_t col) const
{
    if (!IsValidPos(row, col))
    {
        return eColor::None;
    }

    return mBoard[row * COLS + col];
}

void Board::SetColor(const size_t row, const size_t col, const eColor color)
{
    if (!IsValidPos(row, col))
    {
        return;
    }

    mBoard[row * COLS + col] = color;
}

bool Board::IsPlaceable(const size_t row, const size_t col) const
{
    if (IsValidPos(row, col)
        && GetColor(row, col) == eColor::None)
    {
        if (mCurTurn == eColor::Black)
        {
            for (Vector2 v : mAnalyzer->GetIllegalPos())
            {
                if (row == v.Y && col == v.X)
                {
                    return false;
                }
            }
        }

        return true;
    }

    return false;
}

bool Board::PlaceStone(const size_t row, const size_t col, const eColor color)
{
    if (!IsPlaceable(row, col))
    {
        return false;
    }

    SetColor(row, col, color);

    mLastPlacedRow = row;
    mLastPlacedCol = col;

    ChangeTurn();

    ++mStoneCount;

    mAnalyzer->Update();

    return true;
}

size_t Board::GetChainCount(const size_t row, const size_t col, const eColor color, const eDirection direction) const
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

    // 현 위치 + 정방향 검사
    size_t r = row;
    size_t c = col;
    while (IsValidPos(r, c) && GetColor(r, c) == color)
    {
        ++count;

        r += dy;
        c += dx;
    }

    // 역방향 검사
    r = row - dy;
    c = col - dx;
    while (IsValidPos(r, c) && GetColor(r, c) == color)
    {
        ++count;

        r -= dy;
        c -= dx;
    }

    return count;
}

bool Board::HasWon()
{
    eColor color = (mCurTurn == eColor::Black) ? eColor::White : eColor::Black;
    return HasWon(color);
}

bool Board::HasWon(const eColor color)
{
    size_t horizontal = GetChainCount(mLastPlacedRow, mLastPlacedCol, color, eDirection::Horizontal);
    size_t vertical = GetChainCount(mLastPlacedRow, mLastPlacedCol, color, eDirection::Vertical);
    size_t leftDiagonal = GetChainCount(mLastPlacedRow, mLastPlacedCol, color, eDirection::LeftDiagonal);
    size_t rightDiagonal = GetChainCount(mLastPlacedRow, mLastPlacedCol, color, eDirection::RightDiagonal);

    if (horizontal >= 5 || vertical >= 5 || leftDiagonal >= 5 || rightDiagonal >= 5)
    {
        if (color == eColor::Black)
        {
            mbBlackWon = true;
            mbWhiteWon = false;
        }
        else
        {
            mbBlackWon = false;
            mbWhiteWon = true;
        }

        return true;
    }

    mbBlackWon = false;
    mbWhiteWon = false;
    return false;
}

void Board::GetPlaceablePos(std::vector<Vector2>* outPosVect)
{
    int row = mLastPlacedRow;
    int col = mLastPlacedCol;

    Vector2 originPos = { col, row };

    for (int i = 1; i < ROWS; ++i)
    {
        // horizontal
        for (int j = row - i; j <= row + i; ++j)
        {
            Vector2 leftHorizontalPos = { col - i, j };
            Vector2 rightHorizontalPos = { col + i, j };

            if (IsPlaceable(leftHorizontalPos.Y, leftHorizontalPos.X))
            {
                outPosVect->push_back(leftHorizontalPos);
            }

            if (IsPlaceable(rightHorizontalPos.Y, rightHorizontalPos.X))
            {
                outPosVect->push_back(rightHorizontalPos);
            }
        }

        // vertical
        for (int j = col - i + 1; j <= col + i - 1; ++j)
        {
            Vector2 topVerticalPos = { j, row - i };
            Vector2 bottomVerticalPos = { j, row + i };

            if (IsPlaceable(topVerticalPos.Y, topVerticalPos.X))
            {
                outPosVect->push_back(topVerticalPos);
            }

            if (IsPlaceable(bottomVerticalPos.Y, bottomVerticalPos.X))
            {
                outPosVect->push_back(bottomVerticalPos);
            }
        }
    }
}

void Board::Clear()
{
    memset(mBoard, static_cast<int>(eColor::None), ROWS * COLS * sizeof(eColor));
    mCurTurn = eColor::Black;
    
    while (!mUndoStack.empty())
    {
        mUndoStack.pop();
    }

    while (!mRedoStack.empty())
    {
        mRedoStack.pop();
    }

    mNotationVect.clear();

    mStoneCount = 0;

    mAnalyzer->Clear();
}

bool Board::Undo()
{
    if (!CanUndo())
    {
        return false;
    }

    mNotationVect.pop_back();

    Vector2 pos = mUndoStack.top();
    mUndoStack.pop();
    mRedoStack.push(pos);

    SetColor(pos.Y, pos.X, eColor::None);
    ChangeTurn();

    mLastPlacedRow = pos.Y;
    mLastPlacedCol = pos.X;

    --mStoneCount;

    mAnalyzer->Update();

    return true;
}

bool Board::Redo()
{
    if (!CanRedo())
    {
        return false;
    }

    Vector2 pos = mRedoStack.top();
    mRedoStack.pop();
    mUndoStack.push(pos);
    
    mNotationVect.push_back(pos);

    SetColor(pos.Y, pos.X, mCurTurn);
    ChangeTurn();

    mLastPlacedRow = pos.Y;
    mLastPlacedCol = pos.X;

    ++mStoneCount;

    mAnalyzer->Update();

    return true;
}

bool Board::SaveNotation()
{
    FILE* fp = fopen("Notation/notation.txt", "w");
    if (fp == nullptr)
    {
        assert(false);
        return false;
    }

    for (Vector2 v : mNotationVect)
    {
        fprintf(fp, "%d %d\n", v.X, v.Y);
    }

    fclose(fp);

    return true;
}

bool Board::LoadNotation()
{
    FILE* fp = fopen("Notation/notation.txt", "r");
    if (fp == nullptr)
    {
        assert(false);
        return false;
    }
    
    Clear(); // 불러오기 전 초기화

    Vector2 pos;
    while (fscanf(fp, "%d %d", &pos.X, &pos.Y) == 2)
    {
        mNotationVect.push_back(pos);
    }

    fclose(fp);

    for (Vector2 v : mNotationVect)
    {
        PlaceStone(v.Y, v.X, mCurTurn);
        mUndoStack.push(v);
    }

    mAnalyzer->Update(); // 불러온 후 분석기 업데이트

    return true;
}

size_t Board::TranslateToBoardX(const size_t x) const
{
    size_t startX = mBoardX - (mCellWidth / 2);
    return (x - startX) / mCellWidth;
}

size_t Board::TranslateToBoardY(const size_t y) const
{
    size_t startY = mBoardY - (mCellWidth / 2);
    return (y - startY) / mCellHeight;
}

void Board::ChangeTurn()
{
    mCurTurn = (mCurTurn == eColor::Black) ? eColor::White : eColor::Black;
}