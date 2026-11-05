#pragma once
#include <opencv2/opencv.hpp>
#include <string>
#include <vector>

class Matcher {
public:
    Matcher();
    ~Matcher();

    bool LoadTemplates(const std::vector<std::string>& filePaths);
    bool FindGear(const cv::Mat& screenImage, cv::Rect searchRegion, cv::Point& outLocation);

private:
    std::vector<cv::Mat> gearTemplates;
};