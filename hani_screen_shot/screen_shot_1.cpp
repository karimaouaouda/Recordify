// GDI_CapturingAnImage.cpp : Defines the entry point for the application.
//
// #include "framework.h"
// #include "GDI_CapturingAnImage.h"
#include <windows.h>
#include <string>
#include <iostream>
#include <cstdio>
#ifndef WTYPES
#define WTYPES
#include <wtypes.h>
#endif
#define MAX_LOADSTRING 100
using namespace std;

#define IDS_APP_TITLE 103

// #define IDR_MAINFRAME 128
// #define IDD_GDICAPTURINGANIMAGE_DIALOG 102
// #define IDD_ABOUTBOX 103
// #define IDM_ABOUT 104
#define IDM_EXIT 105
#define IDI_GDICAPTURINGANIMAGE 107
#define IDI_SMALL 108
#define IDC_GDICAPTURINGANIMAGE 109
// #define IDC_MYICON 2
// #ifndef IDC_STATIC
// #define IDC_STATIC -1
// #endif
// Next default values for new objects
//
// #ifdef APSTUDIO_INVOKED
// #ifndef APSTUDIO_READONLY_SYMBOLS

// #define _APS_NO_MFC 130
// #define _APS_NEXT_RESOURCE_VALUE 129
// #define _APS_NEXT_COMMAND_VALUE 32771
// #define _APS_NEXT_CONTROL_VALUE 1000
// #define _APS_NEXT_SYMED_VALUE 110
// #endif
// #endif
// Global Variables:
char *error;
HINSTANCE hInst;                     // current instance
WCHAR szTitle[MAX_LOADSTRING];       // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING]; // the main window class name
HWND hButton = NULL;
HANDLE hFile = NULL;
// Forward declarations of functions included in this code module:
ATOM MyRegisterClass(HINSTANCE hInstance);
BOOL InitInstance(HINSTANCE, int);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR lpCmdLine,
                     int nCmdShow)
{
    // Create a console window
    int alc = AllocConsole();
    if (alc == 0)
    {
        MessageBoxW(NULL, L"failed", L"Failed", MB_OK);
    }
    else
    {
        MessageBoxW(NULL, L"suc", L"suc", MB_OK);
    }
    freopen("CONOUT$", "w", stdout);
    std::cout << "This works" << std::endl;
    cout << "start" << endl;
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_GDICAPTURINGANIMAGE, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance(hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_GDICAPTURINGANIMAGE));

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int)msg.wParam;
}

//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_GDICAPTURINGANIMAGE));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_GDICAPTURINGANIMAGE);
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    hInst = hInstance; // Store instance handle in our global variable

    HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
                              CW_USEDEFAULT, 0, 100, 100, nullptr, nullptr, hInstance, nullptr);

    hButton = CreateWindowW(
        L"BUTTON",          // Predefined class for button
        L"make screenshot", // Button text
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
        2, 10, 100, 30, // Position & size
        hWnd,           // Parent window
        (HMENU)1,       // Button ID
        hInstance,
        NULL);
    if (!hWnd)
    {
        return FALSE;
    }

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    return TRUE;
}

// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

//
//   FUNCTION: CaptureAnImage(HWND hWnd)
//
//   PURPOSE: Captures a screenshot into a window ,and then saves it in a .bmp file.
//
//   COMMENTS:
//
//      Note: This function attempts to create a file called captureqwsx.bmp
//

int CaptureAnImage(HWND hWnd)
{
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
    hdcWindow = GetDC(hWnd);

    // Create a compatible DC, which is used in a BitBlt from the window DC.
    hdcMemDC = CreateCompatibleDC(hdcWindow);

    if (!hdcMemDC)
    {
        cout << "CreateCompatibleDC has failed" << endl;
        MessageBoxW(hWnd, L"CreateCompatibleDC has failed", L"Failed", MB_OK);
        DWORD err = GetLastError();
        LPVOID msg;
        FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                       NULL, err, 0, (LPWSTR)&msg, 0, NULL);
        wprintf(L"Error (line %d): %s\n", __LINE__, (LPWSTR)msg);
        LocalFree(msg);
        return 0;
    }

    // Get the client area for size calculation.
    RECT rcClient;
    GetClientRect(hWnd, &rcClient);

    // This is the best stretch mode.
    int setstrec = SetStretchBltMode(hdcWindow, HALFTONE);
    if (setstrec == 0)
    {
        cout << "setscret err" << endl;
    }
    // The source DC is the entire screen, and the destination DC is the current window (HWND).
    if (!StretchBlt(hdcWindow,
                    0, 0,
                    rcClient.right, rcClient.bottom,
                    hdcScreen,
                    0, 0,
                    GetSystemMetrics(SM_CXSCREEN),
                    GetSystemMetrics(SM_CYSCREEN),
                    SRCCOPY))
    {
        cout << "StretchBlt has failed" << endl;
        MessageBoxW(hWnd, L"StretchBlt has failed", L"Failed", MB_OK);

        return 0;
    }

    // Create a compatible bitmap from the Window DC.
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);
    hbmScreen = CreateCompatibleBitmap(hdcScreen, screenWidth, screenHeight);
    if (!hbmScreen)
    {
        cout << "CreateCompatibleBitmap Failed" << endl;
        MessageBoxW(hWnd, L"CreateCompatibleBitmap Failed", L"Failed", MB_OK);

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
        MessageBoxW(hWnd, L"BitBlt has failed", L"Failed", MB_OK);

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

    dwBmpSize = ((bmpScreen.bmWidth * bi.biBitCount + 31) / 32) * 4 * bmpScreen.bmHeight;

    // Starting with 32-bit Windows, GlobalAlloc and LocalAlloc are implemented as wrapper functions that
    // call HeapAlloc using a handle to the process's default heap. Therefore, GlobalAlloc and LocalAlloc
    // have greater overhead than HeapAlloc.
    hDIB = GlobalAlloc(GHND, dwBmpSize);
    lpbitmap = (char *)GlobalLock(hDIB);

    // Gets the "bits" from the bitmap, and copies them into a buffer
    // that's pointed to by lpbitmap.
    getD = GetDIBits(hdcWindow, hbmScreen, 0,
                     (UINT)bmpScreen.bmHeight,
                     lpbitmap,
                     (BITMAPINFO *)&bi, DIB_RGB_COLORS);

    cout << "just to make sure " << getD << endl;

    if (getD == 0)
    {
        cout << "function fails" << endl;
    }
    OutputDebugStringW(L"start capture");
    // A file is created, this is where we will save the screen capture.
    // captureqwsx.bmp
    std::cout << "before create file" << std::endl;
    cout << "error before create file: " << GetLastError() << endl;

    hFile = CreateFileW(L"C:\\Users\\USER\\Desktop\\screenshot.bmp",
                        GENERIC_WRITE,
                        FILE_SHARE_READ | FILE_SHARE_WRITE,
                        NULL,
                        CREATE_ALWAYS,
                        FILE_ATTRIBUTE_NORMAL, NULL);

    cout << "before check file " << hFile << endl;
    if (hFile == INVALID_HANDLE_VALUE)
    {
        error = "this is error , file not created  ";
        cout << error << GetLastError() << endl;

        OutputDebugStringA(error);
    }
    else
    {
        std::cout << "no error" << std::endl;
        OutputDebugStringA("no error");
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
    else
    {
        cout << "nothing else" << endl;
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
    ReleaseDC(hWnd, hdcWindow);

    return 0;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam);
        // Parse the menu selections:
        string mes;
        mes.append("message is: ").append(std::to_string(message));

        OutputDebugStringA(mes.c_str());
        switch (wmId)
        {
        case 1:
        {
            OutputDebugStringW(L"button clicked\n");
            cout << "button clicked" << endl;

            int mesg = MessageBoxW(hWnd, L"SCREENSHOT MADE SUCCESSFULLY", L"MAKE SCREENSHOT!", MB_OK);
            if (mesg == IDOK)
            {
                CaptureAnImage(hWnd);
            }
        }
        break;

        case IDM_EXIT:
            DestroyWindow(hWnd);
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
    }
    break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
