#pragma once

#include "Button.h"

class DDraw;

class Menu final
{
public:
    Menu(DDraw* ddraw);
    Menu(const Menu&) = delete;
    ~Menu();

    void Draw() const;

    bool AddButton(const uint32_t row, const uint32_t col, const eButtonType type, const wchar_t* const text);
    const Button* GetSelectedButtonOrNull() const;

private:
    static constexpr float MENU_RATIO = 0.3f;

    static constexpr uint32_t PANEL_PADDING = 20;
    static constexpr uint32_t PANEL_COLOR = 0xe9ebeeU;  // 연한 회색

    static constexpr uint32_t BUTTON_ROW_NUMS = 9;
    static constexpr uint32_t BUTTON_COL_NUMS = 2;
    static constexpr uint32_t BUTTON_NUMS = BUTTON_ROW_NUMS * BUTTON_COL_NUMS;
    static constexpr uint32_t BUTTON_PADDING = BUTTON_ROW_NUMS;

private:
    DDraw* mDDraw = nullptr;

    uint32_t mPanelX;
    uint32_t mPanelY;
    uint32_t mPanelWidth;
    uint32_t mPanelHeight;

    uint32_t mButtonWidth;
    uint32_t mButtonHeight;
    Button* mButtons[BUTTON_ROW_NUMS][BUTTON_COL_NUMS];

    Button* mClickedButton;
};