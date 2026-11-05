#pragma once
#include <opencv2/opencv.hpp>
#include <windows.h>
#include <string>

class ScreenCapture {
public:
    ScreenCapture();
    ~ScreenCapture();

    bool Initialize(const std::string& windowTitle);
    bool GrabFrame(cv::Mat& outFrame);

    int GetWindowX() const;
    int GetWindowY() const;

private:
    HWND targetWindow;
    int width;
    int height;
    int windowX;
    int windowY;
    HDC hScreenDC;
    HDC hMemoryDC;
    HBITMAP hBitmap;
    BITMAPINFOHEADER bi;
    void* bits;
    cv::Mat rawMat;
};