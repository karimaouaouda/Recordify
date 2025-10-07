// Core recorder coordinator
#include "core/recorder.h"
#include "screen_handler/screen_handler.h"
#include "audio_handler/audio_capture.h"
#include "video_handler/video_encoder.h"
#include "file_manager/file_writer.h"
#include <iostream>
#include <memory>

namespace Recordify::Core {

class Recorder {
public:
    Recorder();
    ~Recorder();
    
    bool initialize();
    void startRecording();
    void stopRecording();
    void pauseRecording();
    void resumeRecording();
    
    bool isRecording() const { return m_isRecording; }
    bool isPaused() const { return m_isPaused; }
    
private:
    std::unique_ptr<ScreenHandler::ScreenHandler> m_screenHandler;
    // AudioHandler::AudioCapture* m_audioCapture;
    // VideoHandler::VideoEncoder* m_videoEncoder;
    // FileManager::FileWriter* m_fileWriter;
    
    bool m_isRecording;
    bool m_isPaused;
    bool m_initialized;
};

Recorder::Recorder() 
    : m_isRecording(false)
    , m_isPaused(false) 
    , m_initialized(false) {
    std::cout << "[Recorder] Created" << std::endl;
}

Recorder::~Recorder() {
    if (m_isRecording) {
        stopRecording();
    }
    std::cout << "[Recorder] Destroyed" << std::endl;
}

bool Recorder::initialize() {
    std::cout << "[Recorder] Initializing..." << std::endl;
    
    // Initialize screen handler
    m_screenHandler = std::make_unique<ScreenHandler::ScreenHandler>();
    if (!m_screenHandler->initialize()) {
        std::cout << "[Recorder] Failed to initialize screen handler" << std::endl;
        return false;
    }
    
    m_initialized = true;
    std::cout << "[Recorder] Initialization completed successfully" << std::endl;
    return true;
}

void Recorder::startRecording() {
    if (!m_initialized) {
        std::cout << "[Recorder] Cannot start recording - not initialized" << std::endl;
        return;
    }
    
    if (m_isRecording) {
        std::cout << "[Recorder] Already recording" << std::endl;
        return;
    }
    
    std::cout << "[Recorder] Starting recording..." << std::endl;
    
    // Start screen capture
    if (m_screenHandler) {
        m_screenHandler->startCapture();
    }
    
    m_isRecording = true;
    m_isPaused = false;
    
    std::cout << "[Recorder] Recording started successfully" << std::endl;
}

void Recorder::stopRecording() {
    if (!m_isRecording) {
        std::cout << "[Recorder] Not recording" << std::endl;
        return;
    }
    
    std::cout << "[Recorder] Stopping recording..." << std::endl;
    
    // Stop screen capture
    if (m_screenHandler) {
        m_screenHandler->stopCapture();
    }
    
    m_isRecording = false;
    m_isPaused = false;
    
    std::cout << "[Recorder] Recording stopped" << std::endl;
}

void Recorder::pauseRecording() {
    if (!m_isRecording || m_isPaused) {
        std::cout << "[Recorder] Cannot pause - not recording or already paused" << std::endl;
        return;
    }
    
    std::cout << "[Recorder] Pausing recording..." << std::endl;
    
    if (m_screenHandler) {
        m_screenHandler->pauseCapture();
    }
    
    m_isPaused = true;
    std::cout << "[Recorder] Recording paused" << std::endl;
}

void Recorder::resumeRecording() {
    if (!m_isRecording || !m_isPaused) {
        std::cout << "[Recorder] Cannot resume - not recording or not paused" << std::endl;
        return;
    }
    
    std::cout << "[Recorder] Resuming recording..." << std::endl;
    
    if (m_screenHandler) {
        m_screenHandler->resumeCapture();
    }
    
    m_isPaused = false;
    std::cout << "[Recorder] Recording resumed" << std::endl;
}

} // namespace Recordify::Core
