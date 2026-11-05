#pragma once
#include <windows.h>
#include <d3d11.h>
#include "Config.h"

class UI {
public:
    static bool Init();
    static void Shutdown();
    static bool Render(BotConfig& config, bool& isRunning, float currentRMS);
private:
    static LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
};