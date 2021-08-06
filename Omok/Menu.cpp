#define _CRT_SECURE_NO_WARNINGS

#include <assert.h>
#include <vector>
#include <stdio.h>

#include "Menu.h"
#include "Board.h"
#include "../Util/MouseEvent.h"
#include "../Util/SafeDelete.h"

Menu::~Menu()
{
    Cleanup();
}

bool Menu::Initailize(DDraw* ddraw, Board* board)
{
    assert(ddraw != nullptr);
    mDDraw = ddraw;

    assert(board != nullptr);
    mBoard = board;

    UpdateMenuSize();

    mButtons.push_back(new Button(ddraw, calculateButtonX(0), calculateButtonY(0), mButtonWidth, mButtonHeight, L"Clear", eButtonType::Clear));
    mButtons.push_back(new Button(ddraw, calculateButtonX(0), calculateButtonY(1), mButtonWidth, mButtonHeight, L"Save Notation", eButtonType::SaveNotation));
    mButtons.push_back(new Button(ddraw, calculateButtonX(1), calculateButtonY(1), mButtonWidth, mButtonHeight, L"Load Notation", eButtonType::LoadNotation));
    mButtons.push_back(new Button(ddraw, calculateButtonX(0), calculateButtonY(BUTTON_ROWS - 1), mButtonWidth, mButtonHeight, L"Undo", eButtonType::Undo));
    mButtons.push_back(new Button(ddraw, calculateButtonX(1), calculateButtonY(BUTTON_ROWS - 1), mButtonWidth, mButtonHeight, L"Redo", eButtonType::Redo));

    return true;
}

void Menu::Cleanup()
{
    for (Button* button : mButtons)
    {
        SAFE_DELETE(button);
    }
}

void Menu::Update()
{
    for (Button* button : mButtons)
    {
        if (button->IsPressed())
        {
            switch (button->GetButtonType())
            {
            case eButtonType::Undo:
                mBoard->Undo();
                break;
            case eButtonType::Redo:
                mBoard->Redo();
                break;
            case eButtonType::Clear:
                mBoard->Clear();
                break;
            case eButtonType::SaveNotation:
                mBoard->SaveNotation();
                break;
            case eButtonType::LoadNotation:
                mBoard->LoadNotation();
                break;
            default:
                break;
            }
        }
    }
}

void Menu::Draw() const
{
    assert(mDDraw != nullptr);

    // 패널 그리기
    mDDraw->DrawRectangle(mPanelX, mPanelY, mPanelWidth, mPanelHeight, PANEL_COLOR);

    // 버튼 그리기
    for (const Button* button : mButtons)
    {
        button->Draw();
    }
}

void Menu::UpdateMenuSize()
{
    assert(mDDraw != nullptr);

    size_t screenWidth = mDDraw->GetWidth();
    size_t screenHeight = mDDraw->GetHeight();

    size_t boardX = mBoard->GetBoardX();
    size_t boardY = mBoard->GetBoardY();
    size_t boardWidth = mBoard->GetBoardWidth();
    size_t boardHeight = mBoard->GetBoardHeight();
    size_t cellWidth = mBoard->GetCellWidth();
    size_t stoneWidth = mBoard->GetStoneWidth();
    size_t stoneHeight = mBoard->GetStoneHeight();

    mPanelWidth = screenWidth - boardX - (boardWidth + stoneWidth / 2);
    mPanelHeight = (size_t)(screenHeight / 1.1f);
    mPanelX = (boardX + boardWidth - cellWidth + stoneWidth);
    mPanelY = (screenHeight - mPanelHeight) / 2;

    mButtonWidth = mPanelWidth / (BUTTON_COLS + 1);
    mButtonHeight = mPanelHeight / (BUTTON_ROWS + 1);

    mPaddingWidth = (mPanelWidth - mButtonWidth * BUTTON_COLS) / (BUTTON_COLS + 1);
    mPaddingHeight = (mPanelHeight - mButtonHeight * BUTTON_ROWS) / (BUTTON_ROWS + 1);

    mDefaultButtonFont = CreateFont((int)(mButtonHeight / 3.5f), 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, DEFAULT_QUALITY, FIXED_PITCH, L"Consolas");
}

int Menu::calculateButtonX(const size_t col) const
{
    return (mPanelX + (mButtonWidth * col) + mPaddingWidth * (col + 1));
}

int Menu::calculateButtonY(const size_t row) const
{
    return (mPanelY + (mButtonHeight * row) + mPaddingHeight * (row + 1));
}