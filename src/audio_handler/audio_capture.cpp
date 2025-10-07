// Audio capture implementation
#include "audio_handler/audio_capture.h"

namespace Recordify::AudioHandler {

class AudioCapture {
public:
    bool initialize();
    void startCapture();
    void stopCapture();
    void setInputDevice(const std::string& deviceName);
    
private:
    bool initialized_ = false;
    std::string currentDevice_;
};

} // namespace Recordify::AudioHandler
