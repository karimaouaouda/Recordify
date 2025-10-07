#ifndef RECORDIFY_AUDIO_HANDLER_AUDIO_CAPTURE_H
#define RECORDIFY_AUDIO_HANDLER_AUDIO_CAPTURE_H

#include <string>

namespace Recordify::AudioHandler {

class AudioCapture {
public:
    bool initialize();
    void startCapture();
    void stopCapture();
    void setInputDevice(const std::string& deviceName);
    bool isCapturing() const;
    
private:
    bool initialized_ = false;
    bool capturing_ = false;
    std::string currentDevice_;
};

} // namespace Recordify::AudioHandler

#endif // RECORDIFY_AUDIO_HANDLER_AUDIO_CAPTURE_H
