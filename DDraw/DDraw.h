#pragma once

#include <ddraw.h>

#pragma comment(lib, "ddraw.lib")
#pragma comment(lib, "dxguid.lib")

class DDraw
{
public:
    DDraw() = default;
    DDraw(const DDraw&) = delete;
    ~DDraw();

    bool Initialize(const HWND hWnd);
    void Cleanup();

    void UpdateWindowPos();
    void UpdateWindowSize();
    bool CreateBackBuffer(const size_t width, const size_t height);

    size_t GetWidth() const { return mWidth; }
    size_t GetHeight() const { return mHeight; }

    bool BeginDraw();
    void EndDraw();
    void Clear(unsigned int color);
    void Blt();

    bool BeginGDI(HDC* outHdc);
    void EndGDI(HDC hdc);
    void PrintText(HDC hdc, const wchar_t* text, const int destX, const int destY, const size_t length, const unsigned int color);

    void DrawRectangle(const int srcX, const int srcY, const int width, const int height, const unsigned int color);

private:
    HWND mhWnd = 0;
    RECT mWindowRect = {};
    size_t mWidth = 0;
    size_t mHeight = 0;

    char* mLockedBackBuffer = nullptr;
    size_t mLockedBackBufferPitch = 0;

    IDirectDraw* mDD = nullptr;
    IDirectDraw7* mDD7 = nullptr;
    IDirectDrawSurface7* mPrimary = nullptr;
    IDirectDrawSurface7* mBack = nullptr;
    IDirectDrawClipper* mClipper = nullptr;
};