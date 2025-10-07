#ifndef RECORDIFY_CORE_RECORDER_H
#define RECORDIFY_CORE_RECORDER_H

namespace Recordify::Core {

class Recorder {
public:
    void startRecording();
    void stopRecording();
    void pauseRecording();
    bool isRecording() const;
    
private:
    bool recording_ = false;
    // Module instances will be declared here
};

} // namespace Recordify::Core

#endif // RECORDIFY_CORE_RECORDER_H
