#include <windows.h>
#include <string>
#include <iostream>
#ifndef WTYPES
#define WTYPES
#endif
#define MAX_LOADSTRING 100
using namespace std;

char *error;
HINSTANCE hInst;                     // current instance
WCHAR szTitle[MAX_LOADSTRING];       // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING]; // the main window class name
HWND hButton = NULL;
HANDLE hFile = NULL;

int CaptureAnImage();
std::string GetLastErrorAsString();

int main()
{
    CaptureAnImage();
    return 0;
}

int CaptureAnImage()
{

    cout << "get screen metrics";
     // 1️⃣ Get screen dimensions
    int screenWidth  = 1920;//GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = 1080;//GetSystemMetrics(SM_CYSCREEN);

    cout << "Screen Width: " << screenWidth << ", Screen Height: " << screenHeight << endl;
    HDC hdcScreen;
    HDC hdcWindow;
    HDC hdcMemDC = NULL;
    HBITMAP hbmScreen = NULL;
    BITMAP bmpScreen;
    DWORD dwBytesWritten = 0;
    DWORD dwSizeofDIB = 0;
    // HANDLE hFile = NULL;
    char *lpbitmap = NULL;
    HANDLE hDIB = NULL;
    DWORD dwBmpSize = 0;

    // Retrieve the handle to a display device context for the client
    // area of the window.
    hdcScreen = GetDC(NULL);
    if (hdcScreen == NULL)
    {
        cout << "err in GetDc" << endl;
    }
    // Create a compatible DC, which is used in a BitBlt from the window DC.
    hdcMemDC = CreateCompatibleDC(hdcScreen);
    if (hdcMemDC == NULL)
    {
        cout << "error hdcMemDC" << endl;
    }
    if (!hdcMemDC)
    {
        cout << "CreateCompatibleDC has failed" << endl;
        MessageBoxW(NULL, L"CreateCompatibleDC has failed", L"Failed", MB_OK);
        DWORD err = GetLastError();
        LPVOID msg;
        FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                       NULL, err, 0, (LPWSTR)&msg, 0, NULL);
        wprintf(L"Error (line %d): %s\n", __LINE__, (LPWSTR)msg);
        LocalFree(msg);
        return 0;
    }

    // get windows Metrics
    int v = GetSystemMetrics(SM_CXSCREEN);
    int h = GetSystemMetrics(SM_CYSCREEN);
    if (v == 0)
    {
        cout << "v errr" << endl;
    }
    else if (h == 0)
    {
        cout << "h errr" << endl;
    }

    // This is the best stretch mode.
    int setstrec = SetStretchBltMode(hdcScreen, HALFTONE);
    if (setstrec == 0)
    {
        cout << "setscret err: " << GetLastErrorAsString() << "sure" << setstrec << endl;
    }
    // The source DC is the entire screen, and the destination DC is the current window (HWND).
    if (!StretchBlt(hdcMemDC,
                    0, 0,
                    0, 0,
                    hdcScreen,
                    0, 0,
                    GetSystemMetrics(SM_CXSCREEN),
                    GetSystemMetrics(SM_CYSCREEN),
                    SRCCOPY))
    {
        cout << "StretchBlt has failed: " << GetLastErrorAsString() << "and handle is : " << hdcScreen << endl;
        return 0;
    }

    // Create a compatible bitmap from the Window DC.
    /* int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN); */
    hbmScreen = CreateCompatibleBitmap(hdcScreen, screenWidth, screenHeight);
    if (!hbmScreen)
    {
        cout << "CreateCompatibleBitmap Failed" << endl;
        MessageBoxW(NULL, L"CreateCompatibleBitmap Failed", L"Failed", MB_OK);

        return 0;
    }

    // Select the compatible bitmap into the compatible memory DC.

    SelectObject(hdcMemDC, hbmScreen);
    BitBlt(hdcMemDC, 0, 0, screenWidth, screenHeight, hdcScreen, 0, 0, SRCCOPY);

    BOOL first;
    BOOL second;
    BOOL third;
    int getD;
    DWORD err;
    LPVOID msg;
    // Bit block transfer into our compatible memory DC.
    if (!BitBlt(hdcMemDC,
                0, 0,
                screenWidth, screenHeight,
                hdcScreen,
                0, 0,
                SRCCOPY))
    {
        cout << "BitBlt has failed" << endl;
        MessageBoxW(NULL, L"BitBlt has failed", L"Failed", MB_OK);

        DeleteDC(hdcMemDC);
        ReleaseDC(NULL, hdcScreen);
        goto done;
    }

    // Get the BITMAP from the HBITMAP.
    GetObject(hbmScreen, sizeof(BITMAP), &bmpScreen);

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


    dwBmpSize = ((screenWidth * bi.biBitCount + 31) / 32) * 4 * screenHeight;

    // Starting with 32-bit Windows, GlobalAlloc and LocalAlloc are implemented as wrapper functions that
    // call HeapAlloc using a handle to the process's default heap. Therefore, GlobalAlloc and LocalAlloc
    // have greater overhead than HeapAlloc.
    hDIB = GlobalAlloc(GHND, dwBmpSize);
    lpbitmap = (char *)GlobalLock(hDIB);

    // Gets the "bits" from the bitmap, and copies them into a buffer
    // that's pointed to by lpbitmap.
    getD = GetDIBits(hdcScreen, hbmScreen, 0,
                     (UINT)bmpScreen.bmHeight,
                     lpbitmap,
                     (BITMAPINFO *)&bi, DIB_RGB_COLORS);

    if (getD == 0)
    {
        cout << "function fails" << endl;
    }
    OutputDebugStringW(L"start capture");
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

    first = WriteFile(hFile, (LPSTR)&bmfHeader, sizeof(BITMAPFILEHEADER), &dwBytesWritten, NULL);
    second = WriteFile(hFile, (LPSTR)&bi, sizeof(BITMAPINFOHEADER), &dwBytesWritten, NULL);
    third = WriteFile(hFile, (LPSTR)lpbitmap, dwBmpSize, &dwBytesWritten, NULL);

    if (first == 0)
    {
        cout << "first errr" << endl;
    }
    else if (second == 0)
    {
        cout << "second errr" << endl;
    }
    else if (third == 0)
    {
        cout << "third errr" << endl;
    }

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

    return 0;
}

std::string GetLastErrorAsString()
{
    // Get the error message ID, if any.
    DWORD errorMessageID = ::GetLastError();
    if (errorMessageID == 0)
    {
        return std::string(); // No error message has been recorded
    }

    LPSTR messageBuffer = nullptr;

    // Ask Win32 to give us the string version of that message ID.
    // The parameters we pass in, tell Win32 to create the buffer that holds the message for us (because we don't yet know how long the message string will be).
    size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                                 NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

    // Copy the error message into a std::string.
    std::string message(messageBuffer, size);

    // Free the Win32's string's buffer.
    LocalFree(messageBuffer);

    return message;
}