#include "Config.h"
#include <fstream>
#include <sstream>
#include <shlobj.h>

std::string ConfigManager::GetConfigPath() {
    char path[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_PERSONAL, NULL, 0, path))) {
        std::string dir = std::string(path) + "\\notepad";
        CreateDirectoryA(dir.c_str(), NULL);
        return dir + "\\notepad.txt";
    }
    return "notepad.txt";
}

void ConfigManager::Load(BotConfig& config) {
    std::ifstream file(GetConfigPath());
    if (!file.is_open()) return;

    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string key;
        float value;
        if (iss >> key >> value) {
            if (key == "castKey") config.castKey = (int)value;
            else if (key == "interactKey") config.interactKey = (int)value;
            else if (key == "pauseKey") config.pauseKey = (int)value;
            else if (key == "quitKey") config.quitKey = (int)value;
            else if (key == "audioThreshold") config.audioThreshold = value;
        }
    }
}

void ConfigManager::Save(const BotConfig& config) {
    std::ofstream file(GetConfigPath());
    if (!file.is_open()) return;

    file << "castKey " << config.castKey << "\n";
    file << "interactKey " << config.interactKey << "\n";
    file << "pauseKey " << config.pauseKey << "\n";
    file << "quitKey " << config.quitKey << "\n";
    file << "audioThreshold " << config.audioThreshold << "\n";
}