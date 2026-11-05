#include "Input.h"
#include <chrono>
#include <thread>

void Input::PressKey(int vkCode) {
    INPUT inputs[2] = { 0 };

    inputs[0].type = INPUT_KEYBOARD;
    inputs[0].ki.wVk = vkCode;

    inputs[1].type = INPUT_KEYBOARD;
    inputs[1].ki.wVk = vkCode;
    inputs[1].ki.dwFlags = KEYEVENTF_KEYUP;

    SendInput(2, inputs, sizeof(INPUT));
}

void Input::MoveSlightly() {
    INPUT inputs[4] = { 0 };

    inputs[0].type = INPUT_KEYBOARD;
    inputs[0].ki.wVk = 'A';

    inputs[1].type = INPUT_KEYBOARD;
    inputs[1].ki.wVk = 'A';
    inputs[1].ki.dwFlags = KEYEVENTF_KEYUP;

    inputs[2].type = INPUT_KEYBOARD;
    inputs[2].ki.wVk = 'D';

    inputs[3].type = INPUT_KEYBOARD;
    inputs[3].ki.wVk = 'D';
    inputs[3].ki.dwFlags = KEYEVENTF_KEYUP;

    SendInput(1, &inputs[0], sizeof(INPUT));
    std::this_thread::sleep_for(std::chrono::milliseconds(250));
    SendInput(1, &inputs[1], sizeof(INPUT));

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    SendInput(1, &inputs[2], sizeof(INPUT));
    std::this_thread::sleep_for(std::chrono::milliseconds(250));
    SendInput(1, &inputs[3], sizeof(INPUT));
}