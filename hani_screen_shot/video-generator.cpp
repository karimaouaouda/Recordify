#include <opencv2/opencv.hpp>
#include <iostream>

int main()
{
    std::cout << "✅ OpenCV is working! Version: " << CV_VERSION << std::endl;

    // Create a black image
    cv::Mat img = cv::Mat::zeros(300, 600, CV_8UC3);

    // Write some text on it
    cv::putText(img, "Hello OpenCV!", cv::Point(100, 150),
                cv::FONT_HERSHEY_SIMPLEX, 1.5, cv::Scalar(0, 255, 0), 3);

    // Display the image
    cv::imshow("Test Window", img);

    // Wait until a key is pressed
    cv::waitKey(0);

    return 0;
}
