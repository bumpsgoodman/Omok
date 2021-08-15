#include <assert.h>
#include <cstdint>
#include <string.h>

#include "Menu.h"
#include "../DDraw/DDraw.h"
#include "../Util/SafeDelete.h"

Menu::Menu(DDraw* ddraw)
    : mDDraw(ddraw)
{
    assert(ddraw != nullptr);

    const uint32_t screenWidth = ddraw->GetWidth();
    const uint32_t screenHeight = ddraw->GetHeight();

    mPanelWidth = static_cast<uint32_t>(screenWidth * MENU_RATIO) - PANEL_PADDING;
    mPanelHeight = screenHeight - PANEL_PADDING;
    mPanelX = screenWidth - mPanelWidth - PANEL_PADDING / 2;
    mPanelY = (screenHeight - mPanelHeight) / 2;

    mButtonWidth = (mPanelWidth - BUTTON_PADDING * (BUTTON_COL_NUMS + 1)) / BUTTON_COL_NUMS;
    mButtonHeight = (mPanelHeight - BUTTON_PADDING * (BUTTON_ROW_NUMS + 1)) / BUTTON_ROW_NUMS;
    memset(mButtons, 0, BUTTON_NUMS * sizeof(Button*));

    AddButton(0, 0, eButtonType::Clear, L"Clear");
    AddButton(1, 0, eButtonType::SaveNotation, L"Save Notation");
    AddButton(1, 1, eButtonType::LoadNotation, L"Load Notation");
    AddButton(BUTTON_ROW_NUMS - 1, 0, eButtonType::Undo, L"Undo");
    AddButton(BUTTON_ROW_NUMS - 1, 1, eButtonType::Redo, L"Redo");
}

Menu::~Menu()
{
    for (uint32_t row = 0; row < BUTTON_ROW_NUMS; ++row)
    {
        for (uint32_t col = 0; col < BUTTON_COL_NUMS; ++col)
        {
            SAFE_DELETE(mButtons[row][col]);
        }
    }
}

void Menu::Draw() const
{
    // 패널 그리기
    mDDraw->DrawRectangle(mPanelX, mPanelY, mPanelWidth, mPanelHeight, PANEL_COLOR);

    // 버튼 그리기
    for (uint32_t row = 0; row < BUTTON_ROW_NUMS; ++row)
    {
        for (uint32_t col = 0; col < BUTTON_COL_NUMS; ++col)
        {
            if (mButtons[row][col] != nullptr)
            {
                mButtons[row][col]->Draw();
            }
        }
    }
}

bool Menu::AddButton(const uint32_t row, const uint32_t col, const eButtonType type, const wchar_t* const text)
{
    if (row >= BUTTON_ROW_NUMS || col >= BUTTON_COL_NUMS)
    {
        return false;
    }

    const uint32_t buttonX = mPanelX + mButtonWidth * col + BUTTON_PADDING * (col + 1);
    const uint32_t buttonY = mPanelY + mButtonHeight * row + BUTTON_PADDING * (row + 1);
    mButtons[row][col] = new Button(mDDraw, buttonX, buttonY, mButtonWidth, mButtonHeight, type, text);

    return true;
}

const Button* Menu::GetSelectedButtonOrNull() const
{
    for (uint32_t row = 0; row < BUTTON_ROW_NUMS; ++row)
    {
        for (uint32_t col = 0; col < BUTTON_COL_NUMS; ++col)
        {
            Button* button = mButtons[row][col];
            if (button != nullptr
                && button->IsPressed())
            {
                return button;
            }
        }
    }

    return nullptr;
}