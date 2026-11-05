#include "Matcher.h"

Matcher::Matcher() {}
Matcher::~Matcher() {}

bool Matcher::LoadTemplates(const std::vector<std::string>& filePaths) {
    gearTemplates.clear();
    for (const auto& path : filePaths) {
        cv::Mat tmpl = cv::imread(path, cv::IMREAD_GRAYSCALE);
        if (!tmpl.empty()) {
            gearTemplates.push_back(tmpl);
        }
    }
    return !gearTemplates.empty();
}

bool Matcher::FindGear(const cv::Mat& screenImage, cv::Rect searchRegion, cv::Point& outLocation) {
    if (gearTemplates.empty() || screenImage.empty()) return false;

    cv::Rect validRegion = searchRegion & cv::Rect(0, 0, screenImage.cols, screenImage.rows);
    if (validRegion.empty()) return false;

    cv::Mat roi = screenImage(validRegion);
    cv::Mat roiGray;
    cv::cvtColor(roi, roiGray, cv::COLOR_BGR2GRAY);

    double bestScore = 0.0;
    cv::Point bestLoc;
    cv::Size bestSize;

    for (const auto& tmpl : gearTemplates) {
        cv::Mat result;
        cv::matchTemplate(roiGray, tmpl, result, cv::TM_CCOEFF_NORMED);

        double maxVal;
        cv::Point maxLoc;
        cv::minMaxLoc(result, nullptr, &maxVal, nullptr, &maxLoc);

        if (maxVal > bestScore) {
            bestScore = maxVal;
            bestLoc = maxLoc;
            bestSize = tmpl.size();
        }
    }

    if (bestScore >= 0.60) {
        outLocation = cv::Point(validRegion.x + bestLoc.x + (bestSize.width / 2), validRegion.y + bestLoc.y + (bestSize.height / 2));
        return true;
    }
    return false;
}