#define NOMINMAX
#include <chrono>
#include <thread>
#include "Vision/Matcher.h"
#include "Capture/ScreenCapture.h"
#include "Input/Input.h"
#include "Audio/AudioCapture.h"
#include "UI.h"
#include "Config.h"
#include <windows.h>
#include <opencv2/opencv.hpp>

enum class BotState {
    CASTING,
    WAITING_FOR_GEAR,
    MONITORING,
    REELING
};

int main() {
    FreeConsole();

    CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);

    if (!UI::Init()) {
        MessageBoxA(NULL, "Failed to initialize UI.", "Error", MB_OK | MB_ICONERROR);
        return 1;
    }

    BotConfig config;
    ConfigManager::Load(config);

    Matcher matcher;
    std::vector<std::string> templates = {
        "assets/gear_1.png",
        "assets/gear_2.png",
        "assets/gear_3.png"
    };

    if (!matcher.LoadTemplates(templates)) {
        MessageBoxA(NULL, "Failed to load templates.", "Error", MB_OK | MB_ICONERROR);
        return 1;
    }

    ScreenCapture capture;
    if (!capture.Initialize("World of Warcraft")) {
        MessageBoxA(NULL, "Failed to find WoW window.", "Error", MB_OK | MB_ICONERROR);
        return 1;
    }

    AudioCapture audio;
    if (!audio.Initialize("World of Warcraft")) {
        MessageBoxA(NULL, "Failed to initialize Audio.", "Error", MB_OK | MB_ICONERROR);
        return 1;
    }

    cv::Mat frame;
    BotState state = BotState::CASTING;

    auto stateTimer = std::chrono::steady_clock::now();
    cv::Point lastGearLoc(-1, -1);

    bool isRunning = false;
    bool isPaused = false;

    bool quitPressed = false;
    bool pausePressed = false;

    float audioThreshold = 0.10f;

    while (true) {
        if (GetAsyncKeyState(config.quitKey) & 0x8000) {
            if (!quitPressed) {
                quitPressed = true;
                break;
            }
        }
        else {
            quitPressed = false;
        }

        if (GetAsyncKeyState(config.pauseKey) & 0x8000) {
            if (!pausePressed) {
                isPaused = !isPaused;
                pausePressed = true;
            }
        }
        else {
            pausePressed = false;
        }

        if (!UI::Render(config, isRunning, isPaused)) {
            break;
        }

        if (!isRunning || isPaused) {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            continue;
        }

        if (!capture.GrabFrame(frame)) break;

        cv::Rect searchRegion(
            static_cast<int>(frame.cols * 0.2),
            static_cast<int>(frame.rows * 0.1),
            static_cast<int>(frame.cols * 0.6),
            static_cast<int>(frame.rows * 0.5)
        );

        cv::Point gearLoc;
        bool gearFound = matcher.FindGear(frame, searchRegion, gearLoc);
        if (gearFound) lastGearLoc = gearLoc;

        float currentRMS = audio.GetCurrentRMS();
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - stateTimer).count();

        switch (state) {
        case BotState::CASTING:
            Input::PressKey(config.castKey);
            state = BotState::WAITING_FOR_GEAR;
            stateTimer = now;
            lastGearLoc = cv::Point(-1, -1);
            break;

        case BotState::WAITING_FOR_GEAR:
            if (gearFound) {
                state = BotState::MONITORING;
                stateTimer = now;
            }
            else if (elapsed > 7000) {
                state = BotState::CASTING;
            }
            break;

        case BotState::MONITORING:
            if (elapsed > 2000 && currentRMS > audioThreshold) {
                Input::PressKey(config.interactKey);
                state = BotState::REELING;
                stateTimer = now;
            }
            break;

        case BotState::REELING:
            if (elapsed > 800) {
                Input::MoveSlightly();
                std::this_thread::sleep_for(std::chrono::milliseconds(1000));
                state = BotState::CASTING;
            }
            break;
        }
    }

    UI::Shutdown();
    cv::destroyAllWindows();
    CoUninitialize();
    return 0;
}