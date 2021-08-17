#pragma once

#include "Board.h"
#include "Goosebumps.h"
#include "Menu.h"
#include "../DDraw/DDraw.h"

class Game final
{
public:
    Game() = default;
    Game(const Game&) = delete;
    ~Game();

    bool Initialize(const HWND hWnd);
    void Cleanup();
    void Process();

    void UpdateWindowPos();
   
private:
    void update();
    void draw() const;

private:
    static constexpr uint32_t FPS = 60;
    static constexpr float TICKS_PER_FRAME = 1000.0f / (float)FPS;

    static constexpr uint32_t BACKGROUND_COLOR = 0x4b7a47U; // ³ì»ö

private:
    HWND mhWnd = nullptr;

    DDraw* mDDraw = nullptr;
    Board* mBoard = nullptr;
    Menu* mMenu = nullptr;

    eColor mPlayerColor = eColor::Black;
    Goosebumps* mComputer = nullptr;
};

extern Game* gGame;