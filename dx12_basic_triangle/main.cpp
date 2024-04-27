
// main.cpp
// ウィンドウ作成とウィンドウメッセージ処理をして、アプリケーションの各関数を呼び出す

#include <windows.h>
#include <cassert>

#include "./dx12_basic_triangle.h"


LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow)
{
    // ウィンドウクラスの登録
    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"DirectX12App";
    RegisterClass(&wc);

    RECT wrc = { 0, 0, Dx12BasicTriangle::kRenderWidth, Dx12BasicTriangle::kRenderHeight };
    AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME, false);

    // ウィンドウの作成
    HWND hWnd = CreateWindowEx(
        0,
        L"DirectX12App",
        L"DirectX 12 App",
        WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME, // サイズ変更を無効にする
        CW_USEDEFAULT, CW_USEDEFAULT, 
        wrc.right - wrc.left, wrc.bottom - wrc.top,
        nullptr,
        nullptr,
        hInstance,
        nullptr
    );
    assert(hWnd != NULL);

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    Dx12BasicTriangle app;

    app.init(hWnd);

    UINT64 frameNumber = 1;
    LARGE_INTEGER frequency;
    QueryPerformanceFrequency(&frequency);
    LARGE_INTEGER prevTime;
    QueryPerformanceCounter(&prevTime);

    // メイン メッセージ ループ:
    MSG msg = {};
    while (msg.message != WM_QUIT)
    {
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else
        {
            LARGE_INTEGER currTime;
            QueryPerformanceCounter(&currTime);

            float deltaTime = static_cast<float>(currTime.QuadPart - prevTime.QuadPart) / frequency.QuadPart;
            prevTime = currTime;

            app.update(frameNumber, deltaTime);
            app.draw(frameNumber);
            frameNumber++;
        }
    }

    app.finalize();

    return 0;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
}
