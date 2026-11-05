#include "ScreenCapture.h"

ScreenCapture::ScreenCapture() {
    targetWindow = NULL;
    width = 0;
    height = 0;
    windowX = 0;
    windowY = 0;
    hScreenDC = NULL;
    hMemoryDC = NULL;
    hBitmap = NULL;
    bits = nullptr;
}

ScreenCapture::~ScreenCapture() {
    if (hBitmap) DeleteObject(hBitmap);
    if (hMemoryDC) DeleteDC(hMemoryDC);
    if (hScreenDC) ReleaseDC(NULL, hScreenDC);
}

bool ScreenCapture::Initialize(const std::string& windowTitle) {
    targetWindow = FindWindowA(NULL, windowTitle.c_str());
    if (!targetWindow) return false;

    RECT rect;
    GetClientRect(targetWindow, &rect);
    width = rect.right - rect.left;
    height = rect.bottom - rect.top;

    POINT topLeft = { 0, 0 };
    ClientToScreen(targetWindow, &topLeft);
    windowX = topLeft.x;
    windowY = topLeft.y;

    hScreenDC = GetDC(NULL);
    hMemoryDC = CreateCompatibleDC(hScreenDC);

    bi.biSize = sizeof(BITMAPINFOHEADER);
    bi.biWidth = width;
    bi.biHeight = -height;
    bi.biPlanes = 1;
    bi.biBitCount = 32;
    bi.biCompression = BI_RGB;
    bi.biSizeImage = 0;
    bi.biXPelsPerMeter = 0;
    bi.biYPelsPerMeter = 0;
    bi.biClrUsed = 0;
    bi.biClrImportant = 0;

    hBitmap = CreateDIBSection(hScreenDC, (BITMAPINFO*)&bi, DIB_RGB_COLORS, &bits, NULL, 0);
    SelectObject(hMemoryDC, hBitmap);

    rawMat = cv::Mat(height, width, CV_8UC4, bits);

    return true;
}

bool ScreenCapture::GrabFrame(cv::Mat& outFrame) {
    if (!targetWindow || !hScreenDC) return false;

    if (!BitBlt(hMemoryDC, 0, 0, width, height, hScreenDC, windowX, windowY, SRCCOPY | CAPTUREBLT)) {
        return false;
    }

    cv::cvtColor(rawMat, outFrame, cv::COLOR_BGRA2BGR);
    return true;
}

int ScreenCapture::GetWindowX() const { return windowX; }
int ScreenCapture::GetWindowY() const { return windowY; }