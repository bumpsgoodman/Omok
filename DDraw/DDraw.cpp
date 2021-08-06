#include <assert.h>

#include "DDraw.h"
#include "../Util/SafeDelete.h"

DDraw::~DDraw()
{
    Cleanup();
}

bool DDraw::Initialize(const HWND hWnd)
{
    HRESULT hr;
    DDSURFACEDESC2 ddsd = {};

    mhWnd = hWnd;

    UpdateWindowPos();
    mWidth = mWindowRect.right - mWindowRect.left;
    mHeight = mWindowRect.bottom - mWindowRect.top;

    hr = DirectDrawCreate(nullptr, &mDD, nullptr);
    if (FAILED(hr))
    {
        goto exit;
    }

    hr = mDD->QueryInterface(IID_IDirectDraw7, (LPVOID*)&mDD7);
    if (FAILED(hr))
    {
        goto exit;
    }

    hr = mDD7->SetCooperativeLevel(hWnd, DDSCL_NORMAL);
    if (FAILED(hr))
    {
        goto exit;
    }

    ddsd.dwSize = sizeof(DDSURFACEDESC2);
    ddsd.dwFlags = DDSD_CAPS;
    ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;

    hr = mDD7->CreateSurface(&ddsd, &mPrimary, nullptr);
    if (FAILED(hr))
    {
        goto exit;
    }

    if (!CreateBackBuffer(mWidth, mHeight))
    {
        goto exit;
    }

    hr = mDD7->CreateClipper(0, &mClipper, nullptr);
    if (FAILED(hr))
    {
        goto exit;
    }
    mClipper->SetHWnd(0, hWnd);
    mPrimary->SetClipper(mClipper);

    return true;

exit:
    Cleanup();
    return false;
}

void DDraw::Cleanup()
{
    SAFE_RELEASE(mClipper);
    SAFE_RELEASE(mBack);
    SAFE_RELEASE(mPrimary);
    SAFE_RELEASE(mDD7);
    SAFE_RELEASE(mDD);
}

void DDraw::UpdateWindowPos()
{
    GetClientRect(mhWnd, &mWindowRect);
    ClientToScreen(mhWnd, (POINT*)&mWindowRect.left);
    ClientToScreen(mhWnd, (POINT*)&mWindowRect.right);
}

void DDraw::UpdateWindowSize()
{
    SAFE_RELEASE(mBack);

    UpdateWindowPos();
    size_t width = mWindowRect.right - mWindowRect.left;
    size_t height = mWindowRect.bottom - mWindowRect.top;
    CreateBackBuffer(width, height);
}

bool DDraw::CreateBackBuffer(const size_t width, const size_t height)
{
    assert(mDD7 != nullptr);

    HRESULT hr;
    DDSURFACEDESC2 ddsd = {};

    ddsd.dwSize = sizeof(DDSURFACEDESC2);
    ddsd.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;
    ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY;
    ddsd.dwHeight = height;
    ddsd.dwWidth = width;

    hr = mDD7->CreateSurface(&ddsd, &mBack, nullptr);
    if (FAILED(hr))
    {
        goto exit;
    }

    mWidth = width;
    mHeight = height;

    return true;

exit:
    return false;
}

bool DDraw::BeginDraw()
{
    assert(mBack != nullptr);

    HRESULT hr;
    DDSURFACEDESC2 ddsd = {};

    ddsd.dwSize = sizeof(DDSURFACEDESC2);

    hr = mBack->Lock(nullptr, &ddsd, DDLOCK_WAIT | DDLOCK_WRITEONLY, nullptr);
    if (FAILED(hr))
    {
        assert(false);
        goto exit;
    }
    
    assert(ddsd.dwWidth == mWidth);
    assert(ddsd.dwHeight == mHeight);

    mLockedBackBuffer = (char*)ddsd.lpSurface;
    mLockedBackBufferPitch = ddsd.lPitch;

    return true;

exit:
    return false;
}

void DDraw::EndDraw()
{
    assert(mBack != nullptr);
    mBack->Unlock(nullptr);

    mLockedBackBuffer = nullptr;
    mLockedBackBufferPitch = 0;
}

void DDraw::Clear(unsigned int color)
{
    assert(mLockedBackBuffer != nullptr);

    for (size_t y = 0; y < mHeight; ++y)
    {
        for (size_t x = 0; x < mWidth; ++x)
        {
            DWORD* dest = (DWORD*)(mLockedBackBuffer + y * mLockedBackBufferPitch + x * 4);
            *dest = color;
        }
    }
}

void DDraw::Blt()
{
    mPrimary->Blt(&mWindowRect, mBack, nullptr, DDBLT_WAIT, nullptr);
}

bool DDraw::BeginGDI(HDC* outHdc)
{
    HRESULT hr;
    HDC hdc;

    hr = mBack->GetDC(&hdc);
    if (FAILED(hr))
    {
        assert(false);
        return false;
    }

    *outHdc = hdc;

    return true;
}

void DDraw::EndGDI(HDC hdc)
{
    mBack->ReleaseDC(hdc);
}

void DDraw::PrintText(HDC hdc, const wchar_t* text, const int destX, const int destY, const size_t length, const unsigned int color)
{
    size_t r = (color & 0x00ff0000) >> 16;
    size_t g = (color & 0x0000ff00) >> 8;
    size_t b = (color & 0x000000ff);

    SetTextColor(hdc, RGB(r, g, b));
    SetBkMode(hdc, TRANSPARENT);

    TextOut(hdc, destX, destY, text, length);
}

void DDraw::DrawRectangle(const int srcX, const int srcY, const int width, const int height, const unsigned int color)
{
    assert(mLockedBackBuffer != nullptr);

    size_t startX = max(srcX, 0);
    size_t startY = max(srcY, 0);
    size_t endX = min(startX + width, mWidth);
    size_t endY = min(startY + height, mHeight);

    for (size_t y = startY; y <= endY; ++y)
    {
        for (size_t x = startX; x <= endX; ++x)
        {
            DWORD* dest = (DWORD*)(mLockedBackBuffer + y * mLockedBackBufferPitch + x * 4);
            *dest = color;
        }
    }
}