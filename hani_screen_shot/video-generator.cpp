#include <opencv2/opencv.hpp>
#include <iostream>
#include <opencv2/core/mat.hpp>
#include <iostream>
#include <codecvt>
// #include <fstream>
#include <vector>
using namespace std;
cv::Mat readImage();
// define size of out images frame
int frame_width, frame_height;

// define class of write video
cv::VideoWriter writeVideo;

// int main()
// {

//     // define the video codec used in the video
//     int videoCodec = writeVideo.fourcc('X', 'V', 'I', 'D');

//     // define resolution of video from the frames
//     cv::Size resolution = cv::Size(frame_width, frame_height);

//     // Define the codec and create VideoWriter object with informations about our video
//     cv::VideoWriter out("output.avi", videoCodec, 30.0, resolution);

//     // lop the images frames
//     while (true)
//     {

//         if (!ret)
//         {
//             std::cout << "End of video or error occurred." << std::endl;
//             break;
//         }

//         // Write the frame to the output video file
//         out.write(frame);

//         // Display the frame
//         cv::imshow("Frame", frame);
//         if (cv::waitKey(1) == 'q')
//         {
//             break;
//         }
//     }
//     return 0;
// }

int main()
{
    readImage();
}

cv::Mat readImage()
{
    int number = 0;
    while (number <= 100)
    {
        string imagepath;

        std::string filename = "test.jpg";
        cout << "before start read the image" << endl;
        wstring imageName = L"test" + to_wstring(number) + L".jpg";
        wstring imagepathC = L"C:/c++_projects/Recordify/hani_screen_shot/" + imageName;
        std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
        imagepath = converter.to_bytes(imagepathC);
        cv::Mat image = cv::imread(imagepath, cv::IMREAD_COLOR);
        cout << "after read the image" << endl;
        // Display the image in a window
        cv::imshow("Displayed Image", image);

        // Wait for a key press before closing the window
        cv::waitKey(0);
        cv::destroyAllWindows();
        // Open the file in binary input mode
        // std::ifstream file(filename, std::ios::in | std::ios::binary);

        // if (!file.is_open())
        // {
        //     std::cerr << "Error opening file: " << filename << std::endl;
        //     return 1;
        // }

        // // Seek to the end of the file to determine its size
        // file.seekg(0, std::ios::end);
        // std::streampos fileSize = file.tellg();
        // file.seekg(0, std::ios::beg); // Seek back to the beginning

        // // Create a vector to store the image data
        // std::vector<char> imageData(fileSize);

        // // Read the entire file into the vector
        // file.read(imageData.data(), fileSize);

        // // Check for read errors
        // if (!file)
        // {
        //     std::cerr << "Error reading file: " << filename << std::endl;
        //     return 1;
        // }

        // file.close(); // Close the file

        // std::cout << "Successfully read " << fileSize << " bytes from " << filename << std::endl;
        number += 1;
        return image;
    }

    // The 'imageData' vector now contains the raw binary data of the image.
    // You can then process this data, for example, by passing it to an image processing library.
}