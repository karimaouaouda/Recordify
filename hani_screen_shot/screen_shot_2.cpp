#include <windows.h>

int main() {
    // 1️⃣ Get screen dimensions
    int screenWidth  = 1920;//GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = 1080;//GetSystemMetrics(SM_CYSCREEN);

    // 2️⃣ Get the device context for the entire screen
    HDC hScreenDC = GetDC(NULL);  // NULL -> entire screen
    HDC hMemoryDC = CreateCompatibleDC(hScreenDC);

    // 3️⃣ Create a compatible bitmap to hold the screenshot
    HBITMAP hBitmap = CreateCompatibleBitmap(hScreenDC, screenWidth, screenHeight);
    SelectObject(hMemoryDC, hBitmap);

    // 4️⃣ Copy screen content into the bitmap
    BitBlt(hMemoryDC, 0, 0, screenWidth, screenHeight, hScreenDC, 0, 0, SRCCOPY);

    // 5️⃣ Save it to a file (optional example)
    // For testing: just save as .bmp
    BITMAPFILEHEADER bmfHeader;
    BITMAPINFOHEADER bi;
    bi.biSize = sizeof(BITMAPINFOHEADER);
    bi.biWidth = screenWidth;
    bi.biHeight = -screenHeight;  // negative -> top-down bitmap
    bi.biPlanes = 1;
    bi.biBitCount = 32;
    bi.biCompression = BI_RGB;
    bi.biSizeImage = 0;
    bi.biXPelsPerMeter = 0;
    bi.biYPelsPerMeter = 0;
    bi.biClrUsed = 0;
    bi.biClrImportant = 0;

    DWORD dwBmpSize = ((screenWidth * bi.biBitCount + 31) / 32) * 4 * screenHeight;
    HANDLE hDIB = GlobalAlloc(GHND, dwBmpSize);
    char* lpbitmap = (char*)GlobalLock(hDIB);

    GetDIBits(hScreenDC, hBitmap, 0, (UINT)screenHeight, lpbitmap, (BITMAPINFO*)&bi, DIB_RGB_COLORS);

    HANDLE hFile = CreateFileW(L"screenshot.bmp", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

    DWORD dwSizeofDIB = dwBmpSize + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
    bmfHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
    bmfHeader.bfSize = dwSizeofDIB;
    bmfHeader.bfType = 0x4D42; // BM

    DWORD dwBytesWritten;
    WriteFile(hFile, (LPSTR)&bmfHeader, sizeof(BITMAPFILEHEADER), &dwBytesWritten, NULL);
    WriteFile(hFile, (LPSTR)&bi, sizeof(BITMAPINFOHEADER), &dwBytesWritten, NULL);
    WriteFile(hFile, (LPSTR)lpbitmap, dwBmpSize, &dwBytesWritten, NULL);

    CloseHandle(hFile);

    // 6️⃣ Cleanup
    GlobalUnlock(hDIB);
    GlobalFree(hDIB);
    ReleaseDC(NULL, hScreenDC);
    DeleteDC(hMemoryDC);
    DeleteObject(hBitmap);

    return 0;
}
