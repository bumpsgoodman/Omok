#pragma once

#include <vector>

#include "Button.h"
#include "../DDraw/DDraw.h"

class Board;
class DDraw;

class Menu
{
public:
    Menu() = default;
    Menu(const Menu&) = delete;
    ~Menu();

    bool Initailize(DDraw* ddraw, Board* board);
    void Cleanup();
    void Update();
    void Draw() const;

    void UpdateMenuSize();

private:
    int calculateButtonX(const size_t col) const;
    int calculateButtonY(const size_t row) const;

private:
    static const size_t BUTTON_COLS = 2;
    static const size_t BUTTON_ROWS = 10;
    static const size_t BUTTON_NUMS = BUTTON_ROWS * BUTTON_COLS;

    static const unsigned int PANEL_COLOR = 0xe9ebeeU;
    static const unsigned int BUTTON_COLOR = 0x8c8c8cU;
    static const unsigned int BUTTON_TEXT_COLOR = 0x000000U;

    HFONT mDefaultButtonFont = nullptr;

    DDraw* mDDraw = nullptr;
    Board* mBoard = nullptr;

    size_t mPanelX = 0;
    size_t mPanelY = 0;
    size_t mPanelWidth = 0;
    size_t mPanelHeight = 0;

    size_t mPaddingWidth = 0;
    size_t mPaddingHeight = 0;

    size_t mButtonWidth = 0;
    size_t mButtonHeight = 0;

    std::vector<Button*> mButtons;
};