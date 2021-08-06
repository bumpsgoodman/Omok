#pragma once

#include "../DDraw/DDraw.h"
#include "eColor.h"

class Board;
class Menu;

class Game
{
public:
    Game() = default;
    Game(const Game&) = delete;
    ~Game();

    bool Initailize(const HWND hWnd);
    void Cleanup();
    void Process();
    void Update();
    void Draw() const;

    bool IsRunning() const { return mbRunning; }

    void UpdateWindowPos();
    void UpdateWindowSize();

private:
    const size_t mFPS = 60;
    float mTicksPerFrame = 1000.f / (float)mFPS;

    HWND mhWnd = 0;
    DDraw* mDDraw = nullptr;
    const unsigned int mBackGroundColor = 0x4b7a47U;

    Board* mBoard = nullptr;

    Menu* mMenu = nullptr;

    bool mbRunning = true;
};

extern Game* g_Game;