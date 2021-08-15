#include <assert.h>
#include <cstdint>

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
        goto EXIT;
    }

    hr = mDD->QueryInterface(IID_IDirectDraw7, (LPVOID*)&mDD7);
    if (FAILED(hr))
    {
        goto EXIT;
    }

    hr = mDD7->SetCooperativeLevel(hWnd, DDSCL_NORMAL);
    if (FAILED(hr))
    {
        goto EXIT;
    }

    ddsd.dwSize = sizeof(DDSURFACEDESC2);
    ddsd.dwFlags = DDSD_CAPS;
    ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;

    hr = mDD7->CreateSurface(&ddsd, &mPrimary, nullptr);
    if (FAILED(hr))
    {
        goto EXIT;
    }

    if (!CreateBackBuffer(mWidth, mHeight))
    {
        goto EXIT;
    }

    hr = mDD7->CreateClipper(0, &mClipper, nullptr);
    if (FAILED(hr))
    {
        goto EXIT;
    }
    mClipper->SetHWnd(0, hWnd);
    mPrimary->SetClipper(mClipper);

    return true;

EXIT:
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

    const uint32_t width = mWindowRect.right - mWindowRect.left;
    const uint32_t height = mWindowRect.bottom - mWindowRect.top;
    CreateBackBuffer(width, height);
}

bool DDraw::CreateBackBuffer(const uint32_t width, const uint32_t height)
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
        goto EXIT;
    }

    mWidth = width;
    mHeight = height;

    return true;

EXIT:
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
        goto EXIT;
    }
    
    assert(ddsd.dwWidth == mWidth);
    assert(ddsd.dwHeight == mHeight);

    mLockedBackBuffer = (char*)ddsd.lpSurface;
    mLockedBackBufferPitch = ddsd.lPitch;

    return true;

EXIT:
    return false;
}

void DDraw::EndDraw()
{
    assert(mBack != nullptr);
    mBack->Unlock(nullptr);

    mLockedBackBuffer = nullptr;
    mLockedBackBufferPitch = 0;
}

void DDraw::Clear(const uint32_t color)
{
    assert(mLockedBackBuffer != nullptr);

    for (size_t y = 0; y < mHeight; ++y)
    {
        for (size_t x = 0; x < mWidth; ++x)
        {
            uint32_t* dest = (uint32_t*)(mLockedBackBuffer + y * mLockedBackBufferPitch + x * 4);
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

void DDraw::EndGDI(const HDC hdc)
{
    mBack->ReleaseDC(hdc);
}

void DDraw::PrintText(const HDC hdc, const wchar_t* const text, const int32_t destX, const int32_t destY, const uint32_t length, const uint32_t color)
{
    const uint32_t r = (color & 0x00ff0000) >> 16;
    const uint32_t g = (color & 0x0000ff00) >> 8;
    const uint32_t b = (color & 0x000000ff);

    SetTextColor(hdc, RGB(r, g, b));
    SetBkMode(hdc, TRANSPARENT);

    TextOut(hdc, destX, destY, text, length);
}

void DDraw::DrawRectangle(const int32_t destX, const int32_t destY, const uint32_t width, const uint32_t height, const uint32_t color)
{
    assert(mLockedBackBuffer != nullptr);

    const uint32_t startX = max(destX, 0);
    const uint32_t startY = max(destY, 0);
    const uint32_t endX = min(destX + width, mWidth);
    const uint32_t endY = min(destY + height, mHeight);

    for (uint32_t y = startY; y < endY; ++y)
    {
        for (uint32_t x = startX; x < endX; ++x)
        {
            uint32_t* dest = (uint32_t*)(mLockedBackBuffer + y * mLockedBackBufferPitch + x * 4);
            *dest = color;
        }
    }
}

void DDraw::DrawBitmap(const int32_t destX, const int32_t destY, const uint32_t width, const uint32_t height, const char* const bitmap)
{
    assert(mLockedBackBuffer != nullptr);
    assert(bitmap != nullptr);

    const uint32_t srcX = (destX >= 0) ? 0 : -destX;
    const uint32_t srcY = (destY >= 0) ? 0 : -destY;

    const uint32_t startX = max(destX, 0);
    const uint32_t startY = max(destY, 0);
    const uint32_t endX = min(destX + (int32_t)width, (int32_t)mWidth);
    const uint32_t endY = min(destY + (int32_t)height, (int32_t)mHeight);

    const uint32_t destWidth = endX - startX;
    const uint32_t destHeight = endY - startY;
    if (destWidth > mWidth || destHeight > mHeight)
    {
        return;
    }

    const char* src = bitmap + (srcY * width + srcX) * 4;
    char* dest = mLockedBackBuffer + startY * mLockedBackBufferPitch + startX * 4;

    for (uint32_t y = startY; y < endY; ++y)
    {
        for (uint32_t x = startX; x < endX; ++x)
        {
            *(uint32_t*)dest = *(uint32_t*)src;

            src += 4;
            dest += 4;
        }

        src -= destWidth * 4;
        src += width * 4;
        dest -= destWidth * 4;
        dest += mLockedBackBufferPitch;
    }
}

void DDraw::DrawBitmapWithColorKey(const int32_t destX, const int32_t destY, const uint32_t width, const uint32_t height, const char* const bitmap, const uint32_t colorKey)
{
    assert(mLockedBackBuffer != nullptr);
    assert(bitmap != nullptr);

    const uint32_t srcX = (destX >= 0) ? 0 : -destX;
    const uint32_t srcY = (destY >= 0) ? 0 : -destY;

    const uint32_t startX = max(destX, 0);
    const uint32_t startY = max(destY, 0);
    const uint32_t endX = min(destX + (int32_t)width, (int32_t)mWidth);
    const uint32_t endY = min(destY + (int32_t)height, (int32_t)mHeight);

    const uint32_t destWidth = endX - startX;
    const uint32_t destHeight = endY - startY;
    if (destWidth > mWidth || destHeight > mHeight)
    {
        return;
    }

    const char* src = bitmap + (srcY * width + srcX) * 4;
    char* dest = mLockedBackBuffer + startY * mLockedBackBufferPitch + startX * 4;

    for (uint32_t y = 0; y < destHeight; ++y)
    {
        for (uint32_t x = 0; x < destWidth; ++x)
        {
            if (*(uint32_t*)src != colorKey)
            {
                *(uint32_t*)dest = *(uint32_t*)src;
            }

            src += 4;
            dest += 4;
        }

        src -= destWidth * 4;
        src += width * 4;
        dest -= destWidth * 4;
        dest += mLockedBackBufferPitch;
    }
}