#pragma once
#ifndef VIDEO_GENERATOR
#define VIDEO_GENERATOR
#include <opencv2/core/mat.hpp>
#include <opencv2/opencv.hpp>
int videoGenerator(int time = 5);
cv::Mat readImage(int number);
#endif
