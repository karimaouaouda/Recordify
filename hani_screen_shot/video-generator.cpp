#include <opencv2/opencv.hpp>
#include <iostream>
#include <opencv2/core/mat.hpp>
#include <iostream>
#include <codecvt>
#include <windows.h>
#include <vector>
using namespace std;
cv::Mat readImage(int number);
// define size of out images frame
int frame_width = 1920;
int frame_height = 1080;

// define class of write video
cv::VideoWriter writeVideo;

cv::Mat image;
int main()
{
    // define the video codec used in the video
    int videoCodec = writeVideo.fourcc('M', 'J', 'P', 'G');
    // define resolution of video from the frames
    cv::Size resolution = cv::Size(frame_width, frame_height);

    // Define the codec and create VideoWriter object with informations about our video
    cv::VideoWriter out("output.avi", videoCodec, 30, cv::Size(frame_width, frame_height));

       // check if is video writer is opened
    if (!out.isOpened())
    {
        std::cerr << "Error: Could not open the video writer." << std::endl;

        cout << "the error is: " << GetLastError() << endl;
        return -1;
    }
    // lop the images frames
    for (int number = 0; number <= 200; number++)
    {
        cv::Mat image = readImage(number);

        if (image.empty())
        {
            std::cout << "End of video or error occurred." << std::endl;
            break;
        }

        // Write the frame to the output video file
        out.write(image);

        // Display the frame
    }
    out.release();
    return 0;
}

cv::Mat readImage(int number)
{
    // resized image
    cv::Mat resizedImage;

    string imagepath;

    wstring imageName = L"image_" + to_wstring(number) + L".jpg";

    // the path of out images(frames)
    wstring imagepathC = L"C:\\python_train\\python_downloader\\downloaded_images\\" + imageName;

    // object used to convert between wstring and string UTF-8
    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;

    // convert our variable from wstring to string(UTF-8)
    imagepath = converter.to_bytes(imagepathC);

    // read the image from that path with color
    cv::Mat image = cv::imread(imagepath, cv::IMREAD_COLOR);

    // resize our image with speicifique width and height without any Distorted
    cv::resize(image, resizedImage, cv::Size(frame_width, frame_height), 0, 0, cv::INTER_LINEAR);

    // return the resized image
    return resizedImage;
}