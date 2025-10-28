#define _WIN32_WINNT 0x0A00
#include <windows.h>
#include <string>
#include <iostream>
#include <thread>
#include <chrono>

#ifndef WTYPES
#define WTYPES
#endif
#define MAX_LOADSTRING 100
using namespace std;
char *error;
HANDLE hFile = NULL;
int width = 1920;
int height = 1080;

DWORD dwBytesWritten = 0;
DWORD dwSizeofDIB = 0;
char *lpbitmap = NULL;
HANDLE hDIB = NULL;
DWORD dwBmpSize = 0;
struct CaptureResult
{
    BITMAP bmpScreen;
    HBITMAP hbmScreen;
    HDC hdcScreen;
    HDC hdcMemDC;
};
CaptureResult CaptureAnImage();
void FullTask(int fileNumber);
void saveFile(BITMAP bmpScreen, HBITMAP hbmScreen, HDC hdcScreen, HDC hdcMemDC, int FileNumber);

int main()
{
    int fileNumber = 0;
    while (fileNumber <= 5)
    {
        Sleep(5000);
        FullTask(fileNumber);
        fileNumber += 1;
    }
    return 0;
}

void FullTask(int fileNumber)
{
    // struct for each parameters of each screen shot
    CaptureResult captureResult;
    // make thread
    std::thread imageCapture([&]()
                             { captureResult = CaptureAnImage(); 
                                saveFile( captureResult.bmpScreen, captureResult.hbmScreen, captureResult.hdcScreen, captureResult.hdcMemDC, fileNumber); });
    // start thread
    imageCapture.join();
}
CaptureResult CaptureAnImage()
{
    CaptureResult result;

    // get the handle windows of screen
    result.hdcScreen = GetDC(NULL);

    // Create a compatible DC, which is used in a BitBlt from the window DC.
    result.hdcMemDC = CreateCompatibleDC(result.hdcScreen);

    // Create a compatible bitmap from the Window DC.
    result.hbmScreen = CreateCompatibleBitmap(result.hdcScreen, width, height);

    // Select the compatible bitmap into the compatible memory DC.
    SelectObject(result.hdcMemDC, result.hbmScreen);

    // Bit block transfer into our compatible memory DC.
    BitBlt(result.hdcMemDC,
           0, 0,
           width, height,
           result.hdcScreen,
           GetSystemMetrics(SM_XVIRTUALSCREEN), GetSystemMetrics(SM_YVIRTUALSCREEN),
           SRCCOPY | CAPTUREBLT);

    // Get the BITMAP from the HBITMAP.
    GetObject(result.hbmScreen, sizeof(BITMAP), &result.bmpScreen);

    return result;
}

void saveFile(BITMAP bmpScreen, HBITMAP hbmScreen, HDC hdcScreen, HDC hdcMemDC, int FileNumber)
{

    BITMAPFILEHEADER bmfHeader;
    BITMAPINFOHEADER bi;

    bi.biSize = sizeof(BITMAPINFOHEADER);
    bi.biWidth = bmpScreen.bmWidth;
    bi.biHeight = bmpScreen.bmHeight;
    bi.biPlanes = 1;
    bi.biBitCount = 32;
    bi.biCompression = BI_RGB;
    bi.biSizeImage = 0;
    bi.biXPelsPerMeter = 0;
    bi.biYPelsPerMeter = 0;
    bi.biClrUsed = 0;
    bi.biClrImportant = 0;

    dwBmpSize = ((bmpScreen.bmWidth * bi.biBitCount + 31) / 32) * 4 * bmpScreen.bmHeight;

    // Starting with 32-bit Windows, GlobalAlloc and LocalAlloc are implemented as wrapper functions that
    // call HeapAlloc using a handle to the process's default heap. Therefore, GlobalAlloc and LocalAlloc
    // have greater overhead than HeapAlloc.
    hDIB = GlobalAlloc(GHND, dwBmpSize);
    lpbitmap = (char *)GlobalLock(hDIB);

    // Gets the "bits" from the bitmap, and copies them into a buffer
    // that's pointed to by lpbitmap.
    GetDIBits(hdcScreen, hbmScreen, 0,
              (UINT)bmpScreen.bmHeight,
              lpbitmap,
              (BITMAPINFO *)&bi, DIB_RGB_COLORS);

    // A file is created, this is where we will save the screen capture.

    wstring fileName = L"screenshot" + to_wstring(FileNumber) + L"helo" + L".bmp";
    hFile = CreateFileW(fileName.c_str(),
                        GENERIC_WRITE,
                        0,
                        NULL,
                        CREATE_ALWAYS,
                        FILE_ATTRIBUTE_NORMAL, NULL);

    if (hFile == INVALID_HANDLE_VALUE)
    {
        cout << "invalid file error " << endl;
        OutputDebugStringA(error);
    }
    // Add the size of the headers to the size of the bitmap to get the total file size.
    dwSizeofDIB = dwBmpSize + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

    // Offset to where the actual bitmap bits start.
    bmfHeader.bfOffBits = (DWORD)sizeof(BITMAPFILEHEADER) + (DWORD)sizeof(BITMAPINFOHEADER);

    // Size of the file.
    bmfHeader.bfSize = dwSizeofDIB;

    // bfType must always be BM for Bitmaps.
    bmfHeader.bfType = 0x4D42; // BM.

    WriteFile(hFile, (LPSTR)&bmfHeader, sizeof(BITMAPFILEHEADER), &dwBytesWritten, NULL);
    WriteFile(hFile, (LPSTR)&bi, sizeof(BITMAPINFOHEADER), &dwBytesWritten, NULL);
    WriteFile(hFile, (LPSTR)lpbitmap, dwBmpSize, &dwBytesWritten, NULL);

    // Unlock and Free the DIB from the heap.
    GlobalUnlock(hDIB);
    GlobalFree(hDIB);

    // Close the handle for the file that was created.
    CloseHandle(hFile);

    // Clean up.
done:
    DeleteObject(hbmScreen);
    DeleteObject(hdcMemDC);
    ReleaseDC(NULL, hdcScreen);
    // ReleaseDC(hWnd, hdcWindow);
}