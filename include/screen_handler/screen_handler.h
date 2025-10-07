#ifndef RECORDIFY_SCREEN_HANDLER_H
#define RECORDIFY_SCREEN_HANDLER_H

#include "screen_handler/screen_writer.h"
#include "screen_handler/screen_reader.h"
#include "utils/geometry.h"
#include <memory>
#include <string>
#include <vector>
#include <functional>
#include <chrono>
#include <map>

namespace Recordify {
namespace ScreenHandler {

// Recording modes and configurations
enum class RecordingMode {
    FULLSCREEN,     // Record entire screen
    WINDOW,         // Record specific window
    REGION,         // Record custom region
    MULTI_DISPLAY,  // Record multiple displays
    FOLLOW_CURSOR   // Record area around cursor
};

enum class CaptureQuality {
    LOW = 1,        // 720p, 15fps, low bitrate
    MEDIUM = 2,     // 1080p, 30fps, medium bitrate
    HIGH = 3,       // 1080p, 60fps, high bitrate
    ULTRA = 4       // 4K, 60fps, maximum bitrate
};

// Recording configuration
struct RecordingConfig {
    RecordingMode mode = RecordingMode::FULLSCREEN;
    CaptureQuality quality = CaptureQuality::MEDIUM;
    
    // Area configuration
    Utils::Rectangle captureArea;
    uintptr_t targetWindow = 0;
    int targetDisplay = -1; // -1 for primary
    
    // Frame rate and timing
    float fps = 30.0f;
    float duration = 0.0f; // 0 = unlimited
    
    // Cursor settings
    bool includeCursor = true;
    bool highlightCursor = false;
    bool highlightClicks = true;
    
    // Annotation settings
    bool enableAnnotations = true;
    bool realTimeAnnotations = true;
    bool persistAnnotations = true;
    
    // Performance settings
    bool useHardwareAcceleration = true;
    bool adaptiveQuality = true;
    int bufferSize = 30; // frames
    
    // Output settings
    std::string outputFormat = "MP4";
    std::string outputPath = "";
    bool autoSave = true;
};

// Real-time capture statistics
struct CaptureStats {
    // Frame statistics
    int totalFrames = 0;
    int droppedFrames = 0;
    float actualFPS = 0.0f;
    float targetFPS = 30.0f;
    
    // Performance metrics
    float captureTime = 0.0f;    // ms per frame
    float processTime = 0.0f;    // ms per frame
    float encodeTime = 0.0f;     // ms per frame
    
    // Memory usage
    int memoryUsage = 0;         // MB
    int bufferUsage = 0;         // %
    
    // Quality metrics
    float compressionRatio = 0.0f;
    int bitrate = 0;             // kbps
    
    std::chrono::steady_clock::time_point startTime;
    std::chrono::steady_clock::time_point lastUpdate;
};

// Event types for the unified handler
enum class ScreenEvent {
    CAPTURE_STARTED,
    CAPTURE_STOPPED,
    CAPTURE_PAUSED,
    CAPTURE_RESUMED,
    FRAME_CAPTURED,
    ANNOTATION_ADDED,
    ANNOTATION_REMOVED,
    DRAWING_STARTED,
    DRAWING_FINISHED,
    ERROR_OCCURRED
};

// Advanced screen interaction modes
enum class InteractionMode {
    PASSIVE,        // Read-only monitoring
    ANNOTATION,     // Allow annotations only
    INTERACTIVE,    // Full interaction (drawing, clicking)
    PRESENTATION    // Presentation mode with highlights
};

// Main ScreenHandler class - coordinates Reader and Writer
class ScreenHandler {
public:
    ScreenHandler();
    ~ScreenHandler();
    
    // Initialization and lifecycle
    bool initialize();
    void shutdown();
    bool isInitialized() const { return m_initialized; }
    
    // Component access
    ScreenWriter* getWriter() { return m_writer.get(); }
    ScreenReader* getReader() { return m_reader.get(); }
    const ScreenWriter* getWriter() const { return m_writer.get(); }
    const ScreenReader* getReader() const { return m_reader.get(); }
    
    // Recording control
    bool startCapture(const RecordingConfig& config = RecordingConfig());
    bool stopCapture();
    bool pauseCapture();
    bool resumeCapture();
    bool isCapturing() const { return m_isCapturing; }
    bool isPaused() const { return m_isPaused; }
    
    // Configuration management
    void setRecordingConfig(const RecordingConfig& config);
    RecordingConfig getRecordingConfig() const { return m_config; }
    void setInteractionMode(InteractionMode mode);
    InteractionMode getInteractionMode() const { return m_interactionMode; }
    
    // Capture area management
    bool setCaptureArea(const Utils::Rectangle& area);
    bool setCaptureWindow(uintptr_t windowHandle);
    bool setCaptureDisplay(int displayId);
    bool setFollowCursor(bool enabled, const Utils::Size& size = Utils::Size(800, 600));
    Utils::Rectangle getCurrentCaptureArea() const;
    
    // Advanced capture features
    bool enableMotionDetection(bool enabled, float sensitivity = 0.1f);
    bool enableSmartCropping(bool enabled); // Auto-crop to content
    bool enableStabilization(bool enabled); // Reduce camera shake
    bool enableAutoFocus(bool enabled);     // Focus on active areas
    
    // Real-time drawing and annotation
    bool startDrawingMode();
    bool stopDrawingMode();
    bool isDrawingMode() const { return m_drawingMode; }
    
    // Quick drawing methods (combines reader input with writer output)
    bool enableQuickAnnotation(); // Click and drag to annotate
    bool enableClickHighlight();  // Highlight mouse clicks
    bool enableKeyboardOverlay(); // Show pressed keys
    bool enableCursorTrail();     // Show cursor movement trail
    
    // Presentation features
    bool enableSpotlight(float radius = 100.0f);        // Highlight area around cursor
    bool enableMagnifier(float zoom = 2.0f);            // Magnify area around cursor
    bool enablePointer(const Color& color = Color::RED); // Virtual pointer
    bool enableRuler();                                  // Measurement tools
    
    // Screen interaction simulation
    bool simulateMouseClick(const Utils::Point& position, int button = 0);
    bool simulateMouseDrag(const Utils::Point& start, const Utils::Point& end);
    bool simulateKeyPress(int keyCode);
    bool simulateKeySequence(const std::string& text);
    bool simulateScroll(const Utils::Point& position, int delta);
    
    // OCR and content analysis
    std::string extractTextFromScreen(const Utils::Rectangle& area = Utils::Rectangle());
    std::vector<Utils::Rectangle> findTextRegions();
    std::vector<Utils::Rectangle> findButtons();
    std::vector<Utils::Rectangle> findImages();
    bool findAndClick(const std::string& text, float confidence = 0.8f);
    
    // Screen comparison and analysis
    bool compareScreens(const ScreenCapture& capture1, const ScreenCapture& capture2,
                       float& similarity, std::vector<Utils::Rectangle>& differences);
    bool waitForScreenChange(const Utils::Rectangle& area, float timeout = 10.0f);
    bool waitForImage(const std::string& imagePath, float timeout = 10.0f);
    bool waitForText(const std::string& text, float timeout = 10.0f);
    
    // Recording and playback
    struct RecordedAction {
        enum Type { MOUSE_MOVE, MOUSE_CLICK, KEY_PRESS, ANNOTATION, WAIT } type;
        std::chrono::steady_clock::time_point timestamp;
        Utils::Point position;
        int button = 0;
        int keyCode = 0;
        std::string text;
        ScreenWriter::Annotation annotation;
        float waitTime = 0.0f;
    };
    
    bool startActionRecording();
    bool stopActionRecording();
    std::vector<RecordedAction> getRecordedActions() const;
    bool playbackActions(const std::vector<RecordedAction>& actions, float speed = 1.0f);
    bool saveActions(const std::string& filePath) const;
    bool loadActions(const std::string& filePath);
    
    // Statistics and monitoring
    CaptureStats getCaptureStats() const;
    void resetCaptureStats();
    
    // Performance optimization
    void setPerformanceMode(bool enabled); // Optimize for performance over quality
    void setPreviewEnabled(bool enabled);  // Show real-time preview
    void setBufferSize(int frames);
    void setThreadCount(int count);        // Parallel processing threads
    
    // Error handling and diagnostics
    enum class ErrorCode {
        SUCCESS = 0,
        INITIALIZATION_FAILED,
        CAPTURE_FAILED,
        WRITER_ERROR,
        READER_ERROR,
        INVALID_CONFIG,
        INSUFFICIENT_RESOURCES,
        PERMISSION_DENIED
    };
    
    ErrorCode getLastError() const { return m_lastError; }
    std::string getLastErrorMessage() const { return m_lastErrorMessage; }
    bool hasError() const { return m_lastError != ErrorCode::SUCCESS; }
    void clearError();
    
    // Event system
    using EventCallback = std::function<void(ScreenEvent, const std::string&)>;
    void setEventCallback(EventCallback callback) { m_eventCallback = callback; }
    
    // Advanced features
    bool enableGestureRecognition(bool enabled);
    bool detectGesture(const std::string& gestureName, float confidence = 0.8f);
    bool enableVoiceControl(bool enabled);
    bool executeVoiceCommand(const std::string& command);
    
    // Integration helpers
    bool exportToClipboard();
    bool importFromClipboard();
    bool shareToApplication(const std::string& appName);
    bool uploadToCloud(const std::string& service);
    
    // Template and preset management
    bool savePreset(const std::string& name, const RecordingConfig& config);
    bool loadPreset(const std::string& name);
    std::vector<std::string> getAvailablePresets() const;
    bool deletePreset(const std::string& name);
    
private:
    // Core components
    std::unique_ptr<ScreenWriter> m_writer;
    std::unique_ptr<ScreenReader> m_reader;
    
    // State management
    bool m_initialized;
    bool m_isCapturing;
    bool m_isPaused;
    bool m_drawingMode;
    
    // Configuration
    RecordingConfig m_config;
    InteractionMode m_interactionMode;
    
    // Statistics
    CaptureStats m_stats;
    
    // Error handling
    ErrorCode m_lastError;
    std::string m_lastErrorMessage;
    
    // Callbacks
    EventCallback m_eventCallback;
    
    // Internal capture state
    std::chrono::steady_clock::time_point m_captureStartTime;
    std::vector<ScreenCapture> m_captureBuffer;
    int m_currentFrame;
    
    // Action recording
    bool m_recordingActions;
    std::vector<RecordedAction> m_recordedActions;
    
    // Internal methods
    bool initializeComponents();
    void shutdownComponents();
    bool validateConfig(const RecordingConfig& config);
    void updateCaptureArea();
    void processFrame();
    void handleReaderEvents();
    void handleWriterEvents();
    
    // Error management
    void setError(ErrorCode code, const std::string& message);
    void logError(const std::string& message);
    
    // Event notifications
    void notifyEvent(ScreenEvent event, const std::string& details = "");
    
    // Performance monitoring
    void updateStats();
    void optimizePerformance();
    
    // Coordination between reader and writer
    void synchronizeComponents();
    void handleCrossComponentEvents();
    
    // Advanced processing
    void processMotionDetection();
    void processSmartCropping();
    void processStabilization();
    void processAutoFocus();
    
    // Threading and synchronization
    struct ThreadingImpl;
    std::unique_ptr<ThreadingImpl> m_threading;
};

}} // namespace Recordify::ScreenHandler

#endif // RECORDIFY_SCREEN_HANDLER_H
