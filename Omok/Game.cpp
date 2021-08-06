#include <assert.h>

#include "Board.h"
#include "Game.h"
#include "Menu.h"
#include "../Util/MouseEvent.h"
#include "../Util/SafeDelete.h"

Game* g_Game;

Game::~Game()
{
    Cleanup();
}

bool Game::Initailize(const HWND hWnd)
{
    mhWnd = hWnd;

    mDDraw = new DDraw;
    if (!mDDraw->Initialize(hWnd))
    {
        goto exit;
    }

    mBoard = new Board;
    if (!mBoard->Initailize(mDDraw))
    {
        goto exit;
    }

    mMenu = new Menu;
    if (!mMenu->Initailize(mDDraw, mBoard))
    {
        goto exit;
    }

    return true;

exit:
    Cleanup();
    return false;
}

void Game::Cleanup()
{
    SAFE_DELETE(mMenu);
    SAFE_DELETE(mBoard);
    SAFE_DELETE(mDDraw);
}

void Game::Process()
{
    static ULONGLONG prevTick = GetTickCount64();
    ULONGLONG curTick = GetTickCount64();
    ULONGLONG elapsedTick = curTick - prevTick;

    if (elapsedTick > mTicksPerFrame)
    {
        Update();
        Draw();

        prevTick = curTick;
    }
}

void Game::Update()
{
    assert(mBoard != nullptr);
    assert(mMenu != nullptr);

    mBoard->Update();
    mMenu->Update();

    MouseEvent::Release();
}

void Game::Draw() const
{
    assert(mDDraw != nullptr);
    assert(mBoard != nullptr);
    assert(mMenu != nullptr);

    if (mDDraw->BeginDraw())
    {
        mDDraw->Clear(mBackGroundColor);
        mBoard->Draw();
        mMenu->Draw();

        mDDraw->EndDraw();
    }

    mDDraw->Blt();
}

void Game::UpdateWindowPos()
{
    assert(mDDraw != nullptr);

    mDDraw->UpdateWindowPos();
}

void Game::UpdateWindowSize()
{
    assert(mDDraw != nullptr);
    assert(mBoard != nullptr);
    assert(mMenu != nullptr);

    mDDraw->UpdateWindowSize();
    mBoard->UpdateBoardSize();
    mMenu->UpdateMenuSize();
}