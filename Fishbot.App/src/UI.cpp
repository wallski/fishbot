#include "UI.h"
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include <string>

static ID3D11Device* g_pd3dDevice = nullptr;
static ID3D11DeviceContext* g_pd3dDeviceContext = nullptr;
static IDXGISwapChain* g_pSwapChain = nullptr;
static ID3D11RenderTargetView* g_mainRenderTargetView = nullptr;
static HWND g_hwnd = nullptr;

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

static void CreateRenderTarget() {
    ID3D11Texture2D* pBackBuffer = nullptr;
    if (SUCCEEDED(g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer)))) {
        g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_mainRenderTargetView);
        pBackBuffer->Release();
    }
}

static void CleanupRenderTarget() {
    if (g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = nullptr; }
}

static bool CreateDeviceD3D(HWND hWnd) {
    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 2;
    sd.BufferDesc.Width = 0;
    sd.BufferDesc.Height = 0;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hWnd;
    sd.SampleDesc.Count = 1;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
    if (D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext) != S_OK)
        return false;

    CreateRenderTarget();
    return true;
}

bool UI::Init() {
    WNDCLASSEXW wc = { sizeof(wc), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, L"NotepadUI", nullptr };
    RegisterClassExW(&wc);
    g_hwnd = CreateWindowW(wc.lpszClassName, L"Notepad", WS_OVERLAPPEDWINDOW, 100, 100, 550, 600, nullptr, nullptr, wc.hInstance, nullptr);

    if (!CreateDeviceD3D(g_hwnd)) {
        CleanupRenderTarget();
        if (g_pSwapChain) { g_pSwapChain->Release(); g_pSwapChain = nullptr; }
        if (g_pd3dDeviceContext) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = nullptr; }
        if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = nullptr; }
        UnregisterClassW(wc.lpszClassName, wc.hInstance);
        return false;
    }

    ShowWindow(g_hwnd, SW_SHOWDEFAULT);
    UpdateWindow(g_hwnd);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowPadding = ImVec2(15, 15);
    style.FramePadding = ImVec2(8, 6);
    style.ItemSpacing = ImVec2(10, 10);

    ImGui_ImplWin32_Init(g_hwnd);
    ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

    return true;
}

void UI::Shutdown() {
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
    CleanupRenderTarget();
    if (g_pSwapChain) { g_pSwapChain->Release(); g_pSwapChain = nullptr; }
    if (g_pd3dDeviceContext) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = nullptr; }
    if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = nullptr; }
    DestroyWindow(g_hwnd);
}

bool UI::Render(BotConfig& config, bool& isRunning, bool& isPaused) {
    MSG msg;
    while (PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
        if (msg.message == WM_QUIT) return false;
    }

    if (!IsWindowVisible(g_hwnd)) return true;

    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
    ImGui::Begin("Notepad Settings", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);

    ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1.0f), "FISHING UTILITY");
    ImGui::Separator();
    ImGui::Spacing();

    if (isPaused) {
        ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), "STATUS: PAUSED");
    }
    else if (isRunning) {
        ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "STATUS: RUNNING");
    }
    else {
        ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "STATUS: STOPPED");
    }

    ImGui::SameLine(ImGui::GetWindowWidth() - 150);

    if (isRunning) {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.6f, 0.1f, 0.1f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.8f, 0.2f, 0.2f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.9f, 0.3f, 0.3f, 1.0f));
        if (ImGui::Button("Stop", ImVec2(120, 35))) {
            isRunning = false;
            isPaused = false;
        }
        ImGui::PopStyleColor(3);
    }
    else {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.1f, 0.5f, 0.1f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.2f, 0.7f, 0.2f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.3f, 0.8f, 0.3f, 1.0f));
        if (ImGui::Button("Start", ImVec2(120, 35))) {
            isRunning = true;
            isPaused = false;
        }
        ImGui::PopStyleColor(3);
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1.0f), "KEYBINDS");
    ImGui::Spacing();

    auto BindKey = [](const char* label, int& keyVar, BotConfig& cfg) {
        static int* capturing = nullptr;
        ImGui::Text("%s", label);

        char buf[64];
        UINT scanCode = MapVirtualKeyA(keyVar, MAPVK_VK_TO_VSC);
        GetKeyNameTextA(scanCode << 16, buf, sizeof(buf));

        std::string buttonText = (capturing == &keyVar) ? "Press any key..." : buf;

        ImGui::NextColumn();
        ImGui::SetNextItemWidth(-1);
        if (ImGui::Button(buttonText.c_str(), ImVec2(-1, 0))) {
            capturing = &keyVar;
        }

        if (capturing == &keyVar) {
            for (int i = 8; i < 255; i++) {
                if (GetAsyncKeyState(i) & 0x8000) {
                    if (i != VK_LBUTTON && i != VK_RBUTTON) {
                        keyVar = i;
                        capturing = nullptr;
                        ConfigManager::Save(cfg);
                        break;
                    }
                }
            }
        }
        ImGui::NextColumn();
        };

    ImGui::Columns(2, "keybind_columns", false);
    ImGui::SetColumnWidth(0, 150);

    BindKey("Cast Key", config.castKey, config);
    BindKey("Interact Key", config.interactKey, config);
    BindKey("Pause Key", config.pauseKey, config);
    BindKey("Quit Key", config.quitKey, config);

    ImGui::Columns(1);

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    if (ImGui::Button("How to Setup (Guide)", ImVec2(200, 30))) {
        ImGui::OpenPopup("Setup Guide");
    }

    ImGui::SetNextWindowSize(ImVec2(520, 0), ImGuiCond_Appearing);
    if (ImGui::BeginPopupModal("Setup Guide", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("1. In WoW, go to Options > Gameplay > Controls.");
        ImGui::Text("2. Check 'Enable Interact Key'.");
        ImGui::Text("3. Bind 'Interact With Target' to a key (e.g., '[').");
        ImGui::Text("4. Set the SAME Interact key in Fishbot.");
        ImGui::Text("5. Place your Fishing skill on an action bar (e.g., '1').");
        ImGui::Text("6. Set the SAME Cast key in Fishbot.");
        ImGui::Spacing();
        ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), "REQUIRED FOR ANTI-AFK:");
        ImGui::Text("You must have strafe left bound to 'A' and strafe right bound to 'D'.");
        ImGui::Spacing();
        ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), "IMPORTANT:");
        ImGui::Text("7. Stand near water, click Start in Fishbot.");
        ImGui::Text("8. Go back to WoW and MANUALLY cast once.");
        ImGui::Text("9. The bot will take over from there.");
        ImGui::Spacing();
        if (ImGui::Button("Close", ImVec2(120, 0))) ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }

    float footerHeight = 80.0f;
    ImGui::SetCursorPosY(ImGui::GetWindowHeight() - footerHeight);

    ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Note: This app and its config are intentionally named 'Notepad'");
    ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "to avoid triggering anti-cheat systems that scan window titles.");
    ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "It is completely safe and does not interact with game memory.");

    ImGui::End();

    ImGui::Render();
    const float clear_color_with_alpha[4] = { 0.1f, 0.1f, 0.1f, 1.0f };
    g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, nullptr);
    g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clear_color_with_alpha);
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
    g_pSwapChain->Present(1, 0);

    return true;
}

LRESULT WINAPI UI::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam)) return true;
    switch (msg) {
    case WM_SIZE:
        if (g_pd3dDevice != nullptr && wParam != SIZE_MINIMIZED) {
            CleanupRenderTarget();
            g_pSwapChain->ResizeBuffers(0, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam), DXGI_FORMAT_UNKNOWN, 0);
            CreateRenderTarget();
        }
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU) return 0;
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hWnd, msg, wParam, lParam);
}