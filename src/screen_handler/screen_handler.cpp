#include "screen_handler/screen_handler.h"
#include <iostream>
#include <algorithm>
#include <thread>
#include <future>

namespace Recordify {
namespace ScreenHandler {

// ScreenHandler implementation
struct ScreenHandler::ThreadingImpl {
    std::vector<std::thread> workerThreads;
    std::atomic<bool> shouldStop{false};
    std::mutex captureMutex;
    std::condition_variable captureCondition;
    
    void startThreads(int threadCount) {
        shouldStop = false;
        workerThreads.reserve(threadCount);
        
        for (int i = 0; i < threadCount; ++i) {
            workerThreads.emplace_back([this]() {
                while (!shouldStop) {
                    std::unique_lock<std::mutex> lock(captureMutex);
                    captureCondition.wait_for(lock, std::chrono::milliseconds(16));
                    
                    if (!shouldStop) {
                        // Process capture work here
                    }
                }
            });
        }
    }
    
    void stopThreads() {
        shouldStop = true;
        captureCondition.notify_all();
        
        for (auto& thread : workerThreads) {
            if (thread.joinable()) {
                thread.join();
            }
        }
        workerThreads.clear();
    }
};

ScreenHandler::ScreenHandler()
    : m_writer(std::make_unique<ScreenWriter>())
    , m_reader(std::make_unique<ScreenReader>())
    , m_threading(std::make_unique<ThreadingImpl>())
    , m_initialized(false)
    , m_isCapturing(false)
    , m_isPaused(false)
    , m_drawingMode(false)
    , m_interactionMode(InteractionMode::PASSIVE)
    , m_lastError(ErrorCode::SUCCESS)
    , m_currentFrame(0)
    , m_recordingActions(false) {
    
    // Set default configuration
    m_config.mode = RecordingMode::FULLSCREEN;
    m_config.quality = CaptureQuality::MEDIUM;
    m_config.fps = 30.0f;
    m_config.includeCursor = true;
    m_config.enableAnnotations = true;
    
    std::cout << "[ScreenHandler] Created with Writer and Reader components" << std::endl;
}

ScreenHandler::~ScreenHandler() {
    shutdown();
    std::cout << "[ScreenHandler] Destroyed" << std::endl;
}

bool ScreenHandler::initialize() {
    if (m_initialized) {
        logError("Already initialized");
        return true;
    }
    
    std::cout << "[ScreenHandler] Initializing components..." << std::endl;
    
    if (!initializeComponents()) {
        setError(ErrorCode::INITIALIZATION_FAILED, "Failed to initialize components");
        return false;
    }
    
    // Setup event callbacks for coordination
    m_reader->setMouseCallback([this](const MouseState& state) {
        handleReaderEvents();
        
        // Auto-annotation for clicks if enabled
        if (m_config.highlightClicks && state.isAnyButtonPressed()) {
            if (m_writer && m_writer->isInitialized()) {
                ScreenWriter::Annotation clickAnnotation;
                clickAnnotation.type = "click";
                clickAnnotation.points = {state.position};
                clickAnnotation.shapeProps.fillColor = Color::RED;
                m_writer->addAnnotation(clickAnnotation);
            }
        }
    });
    
    m_reader->setKeyboardCallback([this](const KeyboardState& state) {
        handleReaderEvents();
        
        // Record actions if enabled
        if (m_recordingActions && !state.lastTypedText.empty()) {
            RecordedAction action;
            action.type = RecordedAction::KEY_PRESS;
            action.timestamp = std::chrono::steady_clock::now();
            action.text = state.lastTypedText;
            m_recordedActions.push_back(action);
        }
    });
    
    m_initialized = true;
    std::cout << "[ScreenHandler] Initialization completed successfully" << std::endl;
    return true;
}

void ScreenHandler::shutdown() {
    if (!m_initialized) return;
    
    std::cout << "[ScreenHandler] Shutting down..." << std::endl;
    
    stopCapture();
    stopDrawingMode();
    
    shutdownComponents();
    
    m_initialized = false;
    std::cout << "[ScreenHandler] Shutdown completed" << std::endl;
}

bool ScreenHandler::startCapture(const RecordingConfig& config) {
    if (!m_initialized) {
        setError(ErrorCode::INITIALIZATION_FAILED, "ScreenHandler not initialized");
        return false;
    }
    
    if (m_isCapturing) {
        logError("Already capturing");
        return false;
    }
    
    if (!validateConfig(config)) {
        setError(ErrorCode::INVALID_CONFIG, "Invalid recording configuration");
        return false;
    }
    
    std::cout << "[ScreenHandler] Starting capture..." << std::endl;
    
    m_config = config;
    updateCaptureArea();
    
    // Start reader monitoring
    if (!m_reader->isMonitoring()) {
        m_reader->startMonitoring();
    }
    
    // Initialize writer canvas if needed
    if (m_config.enableAnnotations && m_writer) {
        Utils::Size canvasSize(m_config.captureArea.width, m_config.captureArea.height);
        if (canvasSize.isEmpty()) {
            auto primaryDisplay = m_reader->getPrimaryDisplay();
            canvasSize = Utils::Size(primaryDisplay.bounds.width, primaryDisplay.bounds.height);
        }
        
        if (!m_writer->createCanvas(canvasSize)) {
            setError(ErrorCode::WRITER_ERROR, "Failed to create writer canvas");
            return false;
        }
        
        if (m_config.realTimeAnnotations) {
            m_writer->enableOverlay();
        }
    }
    
    // Start threading system
    int threadCount = std::max(1, static_cast<int>(std::thread::hardware_concurrency()) / 2);
    m_threading->startThreads(threadCount);
    
    m_isCapturing = true;
    m_isPaused = false;
    m_captureStartTime = std::chrono::steady_clock::now();
    m_currentFrame = 0;
    
    // Reset statistics
    m_stats = CaptureStats{};
    m_stats.startTime = m_captureStartTime;
    m_stats.targetFPS = m_config.fps;
    
    notifyEvent(ScreenEvent::CAPTURE_STARTED, "Capture started successfully");
    std::cout << "[ScreenHandler] Capture started successfully" << std::endl;
    return true;
}

bool ScreenHandler::stopCapture() {
    if (!m_isCapturing) {
        logError("Not capturing");
        return false;
    }
    
    std::cout << "[ScreenHandler] Stopping capture..." << std::endl;
    
    // Stop threading
    m_threading->stopThreads();
    
    // Stop components
    m_reader->stopMonitoring();
    
    if (m_writer) {
        m_writer->stopRealTimeDrawing();
        m_writer->disableOverlay();
    }
    
    m_isCapturing = false;
    m_isPaused = false;
    
    notifyEvent(ScreenEvent::CAPTURE_STOPPED, "Capture stopped");
    std::cout << "[ScreenHandler] Capture stopped" << std::endl;
    return true;
}

bool ScreenHandler::pauseCapture() {
    if (!m_isCapturing || m_isPaused) {
        logError("Cannot pause - not capturing or already paused");
        return false;
    }
    
    std::cout << "[ScreenHandler] Pausing capture..." << std::endl;
    
    m_isPaused = true;
    notifyEvent(ScreenEvent::CAPTURE_PAUSED, "Capture paused");
    return true;
}

bool ScreenHandler::resumeCapture() {
    if (!m_isCapturing || !m_isPaused) {
        logError("Cannot resume - not capturing or not paused");
        return false;
    }
    
    std::cout << "[ScreenHandler] Resuming capture..." << std::endl;
    
    m_isPaused = false;
    notifyEvent(ScreenEvent::CAPTURE_RESUMED, "Capture resumed");
    return true;
}

// Configuration management
void ScreenHandler::setRecordingConfig(const RecordingConfig& config) {
    if (validateConfig(config)) {
        m_config = config;
        std::cout << "[ScreenHandler] Updated recording configuration" << std::endl;
        
        if (m_isCapturing) {
            updateCaptureArea();
        }
    } else {
        setError(ErrorCode::INVALID_CONFIG, "Invalid recording configuration");
    }
}

void ScreenHandler::setInteractionMode(InteractionMode mode) {
    m_interactionMode = mode;
    
    std::string modeStr;
    switch (mode) {
        case InteractionMode::PASSIVE: modeStr = "PASSIVE"; break;
        case InteractionMode::ANNOTATION: modeStr = "ANNOTATION"; break;
        case InteractionMode::INTERACTIVE: modeStr = "INTERACTIVE"; break;
        case InteractionMode::PRESENTATION: modeStr = "PRESENTATION"; break;
    }
    
    std::cout << "[ScreenHandler] Set interaction mode to: " << modeStr << std::endl;
    
    // Configure components based on mode
    switch (mode) {
        case InteractionMode::PASSIVE:
            if (m_writer) m_writer->disableOverlay();
            break;
            
        case InteractionMode::ANNOTATION:
        case InteractionMode::INTERACTIVE:
        case InteractionMode::PRESENTATION:
            if (m_writer) m_writer->enableOverlay();
            break;
    }
}

// Capture area management
bool ScreenHandler::setCaptureArea(const Utils::Rectangle& area) {
    if (area.isEmpty()) {
        logError("Invalid capture area");
        return false;
    }
    
    m_config.mode = RecordingMode::REGION;
    m_config.captureArea = area;
    
    std::cout << "[ScreenHandler] Set capture area to: [" << area.x << "," << area.y << "," 
              << area.width << "," << area.height << "]" << std::endl;
    
    if (m_isCapturing) {
        updateCaptureArea();
    }
    
    return true;
}

bool ScreenHandler::setCaptureWindow(uintptr_t windowHandle) {
    if (windowHandle == 0) {
        logError("Invalid window handle");
        return false;
    }
    
    m_config.mode = RecordingMode::WINDOW;
    m_config.targetWindow = windowHandle;
    
    std::cout << "[ScreenHandler] Set capture window to handle: " << windowHandle << std::endl;
    
    if (m_isCapturing) {
        updateCaptureArea();
    }
    
    return true;
}

bool ScreenHandler::setCaptureDisplay(int displayId) {
    auto displays = m_reader->getDisplays();
    
    bool validDisplay = false;
    for (const auto& display : displays) {
        if (display.displayId == displayId) {
            validDisplay = true;
            break;
        }
    }
    
    if (!validDisplay) {
        logError("Invalid display ID: " + std::to_string(displayId));
        return false;
    }
    
    m_config.mode = RecordingMode::FULLSCREEN;
    m_config.targetDisplay = displayId;
    
    std::cout << "[ScreenHandler] Set capture display to: " << displayId << std::endl;
    
    if (m_isCapturing) {
        updateCaptureArea();
    }
    
    return true;
}

Utils::Rectangle ScreenHandler::getCurrentCaptureArea() const {
    return m_config.captureArea;
}

// Real-time drawing and annotation
bool ScreenHandler::startDrawingMode() {
    if (m_drawingMode) return true;
    
    std::cout << "[ScreenHandler] Starting drawing mode..." << std::endl;
    
    if (!m_writer || !m_writer->isInitialized()) {
        setError(ErrorCode::WRITER_ERROR, "Writer not available");
        return false;
    }
    
    if (!m_writer->startRealTimeDrawing()) {
        setError(ErrorCode::WRITER_ERROR, "Failed to start real-time drawing");
        return false;
    }
    
    m_drawingMode = true;
    notifyEvent(ScreenEvent::DRAWING_STARTED, "Drawing mode started");
    return true;
}

bool ScreenHandler::stopDrawingMode() {
    if (!m_drawingMode) return true;
    
    std::cout << "[ScreenHandler] Stopping drawing mode..." << std::endl;
    
    if (m_writer) {
        m_writer->stopRealTimeDrawing();
    }
    
    m_drawingMode = false;
    notifyEvent(ScreenEvent::DRAWING_FINISHED, "Drawing mode stopped");
    return true;
}

// Quick annotation features
bool ScreenHandler::enableQuickAnnotation() {
    if (!m_writer || !m_reader) return false;
    
    std::cout << "[ScreenHandler] Enabling quick annotation mode" << std::endl;
    
    // Set up mouse callback for click-and-drag annotations
    m_reader->setMouseCallback([this](const MouseState& state) {
        if (m_interactionMode >= InteractionMode::ANNOTATION && m_writer->isRealTimeDrawing()) {
            if (state.leftButton.pressed && state.isDragging()) {
                // Create annotation based on drag
                ScreenWriter::Annotation annotation;
                annotation.type = "drag";
                annotation.points = {state.leftButton.pressPosition, state.position};
                annotation.shapeProps.strokeColor = Color::BLUE;
                annotation.shapeProps.strokeWidth = 2.0f;
                
                m_writer->addAnnotation(annotation);
            }
        }
    });
    
    return true;
}

bool ScreenHandler::enableClickHighlight() {
    if (!m_config.highlightClicks) {
        m_config.highlightClicks = true;
        std::cout << "[ScreenHandler] Enabled click highlighting" << std::endl;
    }
    return true;
}

bool ScreenHandler::enableKeyboardOverlay() {
    if (!m_reader || !m_writer) return false;
    
    std::cout << "[ScreenHandler] Enabling keyboard overlay" << std::endl;
    
    m_reader->setKeyboardCallback([this](const KeyboardState& state) {
        if (m_writer->isRealTimeDrawing() && !state.lastTypedText.empty()) {
            // Show typed text as overlay
            auto mousePos = m_reader->getMousePosition();
            ScreenWriter::TextProperties textProps;
            textProps.color = Color::WHITE;
            textProps.font.size = 16.0f;
            
            m_writer->drawText(state.lastTypedText, 
                             Utils::Point(mousePos.x + 20, mousePos.y - 30), 
                             textProps);
        }
    });
    
    return true;
}

// OCR and content analysis
std::string ScreenHandler::extractTextFromScreen(const Utils::Rectangle& area) {
    if (!m_reader) return "";
    
    ScreenCapture capture;
    Utils::Rectangle captureArea = area.isEmpty() ? getCurrentCaptureArea() : area;
    
    if (m_reader->captureScreen(capture, captureArea)) {
        std::cout << "[ScreenHandler] Extracted text from area [" 
                  << captureArea.x << "," << captureArea.y << "," 
                  << captureArea.width << "," << captureArea.height << "]" << std::endl;
        
        // Placeholder for OCR implementation
        return "Sample extracted text";
    }
    
    return "";
}

std::vector<Utils::Rectangle> ScreenHandler::findTextRegions() {
    std::vector<Utils::Rectangle> regions;
    
    if (m_reader) {
        ScreenCapture capture;
        if (m_reader->captureScreen(capture, getCurrentCaptureArea())) {
            regions = capture.findTextRegions();
            std::cout << "[ScreenHandler] Found " << regions.size() << " text regions" << std::endl;
        }
    }
    
    return regions;
}

// Recording and playback
bool ScreenHandler::startActionRecording() {
    if (m_recordingActions) return true;
    
    std::cout << "[ScreenHandler] Starting action recording..." << std::endl;
    
    m_recordingActions = true;
    m_recordedActions.clear();
    
    return true;
}

bool ScreenHandler::stopActionRecording() {
    if (!m_recordingActions) return true;
    
    std::cout << "[ScreenHandler] Stopping action recording. Recorded " 
              << m_recordedActions.size() << " actions" << std::endl;
    
    m_recordingActions = false;
    return true;
}

std::vector<ScreenHandler::RecordedAction> ScreenHandler::getRecordedActions() const {
    return m_recordedActions;
}

// Statistics
CaptureStats ScreenHandler::getCaptureStats() const {
    return m_stats;
}

void ScreenHandler::resetCaptureStats() {
    m_stats = CaptureStats{};
    if (m_isCapturing) {
        m_stats.startTime = std::chrono::steady_clock::now();
        m_stats.targetFPS = m_config.fps;
    }
    std::cout << "[ScreenHandler] Reset capture statistics" << std::endl;
}

// Error handling
void ScreenHandler::clearError() {
    m_lastError = ErrorCode::SUCCESS;
    m_lastErrorMessage.clear();
}

// Private methods
bool ScreenHandler::initializeComponents() {
    // Initialize reader first
    if (!m_reader->initialize()) {
        logError("Failed to initialize ScreenReader");
        return false;
    }
    
    // Initialize writer
    if (!m_writer->initialize()) {
        logError("Failed to initialize ScreenWriter");
        return false;
    }
    
    std::cout << "[ScreenHandler] All components initialized successfully" << std::endl;
    return true;
}

void ScreenHandler::shutdownComponents() {
    if (m_threading) {
        m_threading->stopThreads();
    }
    
    if (m_writer) {
        m_writer->shutdown();
    }
    
    if (m_reader) {
        m_reader->shutdown();
    }
    
    std::cout << "[ScreenHandler] All components shut down" << std::endl;
}

bool ScreenHandler::validateConfig(const RecordingConfig& config) {
    if (config.fps <= 0 || config.fps > 120) {
        logError("Invalid FPS: " + std::to_string(config.fps));
        return false;
    }
    
    if (config.mode == RecordingMode::REGION && config.captureArea.isEmpty()) {
        logError("Region mode requires valid capture area");
        return false;
    }
    
    if (config.mode == RecordingMode::WINDOW && config.targetWindow == 0) {
        logError("Window mode requires valid window handle");
        return false;
    }
    
    return true;
}

void ScreenHandler::updateCaptureArea() {
    switch (m_config.mode) {
        case RecordingMode::FULLSCREEN:
            if (m_config.targetDisplay >= 0) {
                auto displays = m_reader->getDisplays();
                for (const auto& display : displays) {
                    if (display.displayId == m_config.targetDisplay) {
                        m_config.captureArea = display.bounds;
                        break;
                    }
                }
            } else {
                m_config.captureArea = m_reader->getPrimaryDisplay().bounds;
            }
            break;
            
        case RecordingMode::WINDOW:
            if (m_config.targetWindow != 0) {
                // Find window bounds
                auto windows = m_reader->getAllWindows();
                for (const auto& window : windows) {
                    if (window.windowHandle == m_config.targetWindow) {
                        m_config.captureArea = window.bounds;
                        break;
                    }
                }
            }
            break;
            
        case RecordingMode::REGION:
            // Area already set
            break;
            
        case RecordingMode::FOLLOW_CURSOR:
            // Update area around cursor
            auto mousePos = m_reader->getMousePosition();
            int halfWidth = 400;  // Default follow area
            int halfHeight = 300;
            m_config.captureArea = Utils::Rectangle(
                mousePos.x - halfWidth, mousePos.y - halfHeight,
                halfWidth * 2, halfHeight * 2
            );
            break;
    }
    
    std::cout << "[ScreenHandler] Updated capture area: [" 
              << m_config.captureArea.x << "," << m_config.captureArea.y << "," 
              << m_config.captureArea.width << "," << m_config.captureArea.height << "]" << std::endl;
}

void ScreenHandler::handleReaderEvents() {
    if (!m_isCapturing || m_isPaused) return;
    
    // Process frame if needed
    auto now = std::chrono::steady_clock::now();
    auto frameInterval = std::chrono::duration<float>(1.0f / m_config.fps);
    auto timeSinceLastFrame = now - m_stats.lastUpdate;
    
    if (timeSinceLastFrame >= frameInterval) {
        processFrame();
        m_stats.lastUpdate = now;
    }
}

void ScreenHandler::handleWriterEvents() {
    // Handle writer-specific events
    if (m_writer && m_writer->isRealTimeDrawing()) {
        // Update real-time drawing
    }
}

void ScreenHandler::processFrame() {
    if (!m_isCapturing || m_isPaused) return;
    
    m_currentFrame++;
    m_stats.totalFrames++;
    
    // Calculate actual FPS
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - m_stats.startTime);
    if (elapsed.count() > 0) {
        m_stats.actualFPS = static_cast<float>(m_stats.totalFrames) / elapsed.count();
    }
    
    notifyEvent(ScreenEvent::FRAME_CAPTURED, "Frame " + std::to_string(m_currentFrame));
}

void ScreenHandler::setError(ErrorCode code, const std::string& message) {
    m_lastError = code;
    m_lastErrorMessage = message;
    logError(message);
    
    notifyEvent(ScreenEvent::ERROR_OCCURRED, message);
}

void ScreenHandler::logError(const std::string& message) {
    std::cerr << "[ScreenHandler ERROR] " << message << std::endl;
}

void ScreenHandler::notifyEvent(ScreenEvent event, const std::string& details) {
    if (m_eventCallback) {
        m_eventCallback(event, details);
    }
}

void ScreenHandler::synchronizeComponents() {
    // Ensure reader and writer are synchronized
    if (m_reader && m_writer && m_isCapturing) {
        // Synchronize cursor position with annotations if needed
        if (m_config.includeCursor && m_writer->isRealTimeDrawing()) {
            auto cursorPos = m_reader->getMousePosition();
            // Update cursor overlay or annotation
        }
    }
}

}} // namespace Recordify::ScreenHandler
