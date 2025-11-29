#ifndef SCREEN_SHOT
#define SCREEN_SHOT
#include <windows.h>
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
void screenShot(int arg);
#endif SCREEN_SHOT
