#include <assert.h>
#include <cstdint>
#include <time.h>
#include <Windows.h>

#include "Game.h"
#include "../Util/Event.h"
#include "../Util/SafeDelete.h"

Game* gGame = nullptr;

Game::~Game()
{
    Cleanup();
}

bool Game::Initialize(const HWND hWnd)
{
    assert(hWnd != nullptr);

    mhWnd = hWnd;

    mDDraw = new DDraw;
    if (!mDDraw->Initialize(hWnd))
    {
        goto EXIT;
    }

    mBoard = new Board(mDDraw);
    mMenu = new Menu(mDDraw);

    srand((unsigned int)GetTickCount64());
    mPlayerColor = static_cast<eColor>(rand() % 2 + 1);
    mComputer = new Goosebumps(mBoard, (mPlayerColor == eColor::Black) ? eColor::White : eColor::Black);

    assert(mPlayerColor != eColor::None);
    assert(mPlayerColor != mComputer->GetColor());

    return true;

EXIT:
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
    static uint64_t prevTick = GetTickCount64();
    const uint64_t curTick = GetTickCount64();
    const float elapsedTick = (float)(curTick - prevTick);
    if (elapsedTick > TICKS_PER_FRAME)
    {
        update();
        draw();

        prevTick = curTick;
    }
}

void Game::UpdateWindowPos()
{
    mDDraw->UpdateWindowPos();
}

void Game::update()
{
    // 보드 업데이트
    if (!mBoard->HasWon(mPlayerColor) && !mBoard->HasWon(mComputer->GetColor()))
    {
        if (mBoard->GetCurTurn() == mPlayerColor)
        {
            if (event::mouse::IsClicked())
            {
                const uint32_t mouseX = event::mouse::GetX();
                const uint32_t mouseY = event::mouse::GetY();
                const Vector2 pos = mBoard->MousePosToBoardPos(mouseX, mouseY);

                mBoard->PlaceStone(pos);
            }
        }
        else
        {
            const Vector2 computerPos = mComputer->GetBestMovePos();
            //if (mBoard->IsPlaceable(computerPos))
            {
                //mBoard->PlaceStone(computerPos);
            }
            //else if (event::mouse::IsClicked())
            {
                const uint32_t mouseX = event::mouse::GetX();
                const uint32_t mouseY = event::mouse::GetY();
                const Vector2 pos = mBoard->MousePosToBoardPos(mouseX, mouseY);

                mBoard->PlaceStone(pos);
            }
        }
    }

    // 메뉴 업데이트
    const Button* button = mMenu->GetSelectedButtonOrNull();
    if (button != nullptr)
    {
        const eButtonType buttonType = button->GetButtonType();
        switch (buttonType)
        {
        case eButtonType::Clear:
            mBoard->ClearBoard();
            break;
        case eButtonType::SaveNotation:
            mBoard->SaveNotation();
            break;
        case eButtonType::LoadNotation:
            mBoard->LoadNotation();
            break;
        case eButtonType::Undo:
            mBoard->Undo();
            break;
        case eButtonType::Redo:
            mBoard->Redo();
            break;
        case eButtonType::SpotInfoOff:
            mBoard->SetSpotInfoColor(eColor::None);
            break;
        case eButtonType::SpotInfoBlack:
            mBoard->SetSpotInfoColor(eColor::Black);
            break;
        case eButtonType::SpotInfoWhite:
            mBoard->SetSpotInfoColor(eColor::White);
            break;
        default:
            break;
        }
    }

    event::mouse::Release();
}

void Game::draw() const
{
    mDDraw->BeginDraw();
    {
        mDDraw->Clear(BACKGROUND_COLOR);
        mBoard->Draw();
        mMenu->Draw();
    }
    mDDraw->EndDraw();

    mDDraw->Blt();
}