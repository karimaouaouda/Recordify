#ifndef RECORDIFY_SCREEN_HANDLER_SCREEN_CAPTURE_H
#define RECORDIFY_SCREEN_HANDLER_SCREEN_CAPTURE_H

#include <string>

namespace recordify {
namespace screen_handler {

class ScreenCapture {
public:
    ScreenCapture();
    ~ScreenCapture();
    
    bool initialize();
    bool startCapture();
    bool stopCapture();
    bool takeScreenshot(const std::string& filename);
    
private:
    bool isInitialized = false;
    bool isCapturing = false;
};

} // namespace screen_handler
} // namespace recordify

#endif // RECORDIFY_SCREEN_HANDLER_SCREEN_CAPTURE_H
