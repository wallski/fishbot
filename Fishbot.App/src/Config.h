#pragma once
#include <string>
#include <windows.h>

struct BotConfig {
    int castKey = '1';
    int interactKey = VK_OEM_4;
    int pauseKey = VK_F8;
    int quitKey = VK_OEM_PLUS;
    float audioThreshold = 0.05f;
};

class ConfigManager {
public:
    static void Load(BotConfig& config);
    static void Save(const BotConfig& config);
private:
    static std::string GetConfigPath();
};