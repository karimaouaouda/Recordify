#define _WIN32_WINNT 0x0A00
#include <windows.h>
#include <string>
#include <iostream>

#ifndef WTYPES
#define WTYPES
#endif
#define MAX_LOADSTRING 100
using namespace std;
char *error;
HANDLE hFile = NULL;
int width = 1920;
int height = 1080;
HDC hdcScreen;
HDC hdcWindow;
HDC hdcMemDC = NULL;
HBITMAP hbmScreen = NULL;
BITMAP bmpScreen;
DWORD dwBytesWritten = 0;
DWORD dwSizeofDIB = 0;
char *lpbitmap = NULL;
HANDLE hDIB = NULL;
DWORD dwBmpSize = 0;
int CaptureAnImage();
// std::string GetLastErrorAsString();

int main()
{

    CaptureAnImage();
    return 0;
}

int CaptureAnImage()
{

    // get the handle windows of screen
    hdcScreen = GetDC(NULL);

    // Create a compatible DC, which is used in a BitBlt from the window DC.
    hdcMemDC = CreateCompatibleDC(hdcScreen);

    // Create a compatible bitmap from the Window DC.
    hbmScreen = CreateCompatibleBitmap(hdcScreen, width, height);

    // Select the compatible bitmap into the compatible memory DC.
    SelectObject(hdcMemDC, hbmScreen);

    // Bit block transfer into our compatible memory DC.
    !BitBlt(hdcMemDC,
            0, 0,
            width, height,
            hdcScreen,
            GetSystemMetrics(SM_XVIRTUALSCREEN), GetSystemMetrics(SM_YVIRTUALSCREEN),
            SRCCOPY | CAPTUREBLT);

    // Get the BITMAP from the HBITMAP.
    GetObject(hbmScreen, sizeof(BITMAP), &bmpScreen);

    return 0;
}

void saveFile()
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

    // OutputDebugStringW(L"start capture");
    // A file is created, this is where we will save the screen capture.
    // captureqwsx.bmp

    hFile = CreateFileW(L"screenshot.bmp",
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