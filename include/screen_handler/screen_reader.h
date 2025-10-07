#ifndef RECORDIFY_SCREEN_READER_H
#define RECORDIFY_SCREEN_READER_H

#include "utils/geometry.h"
#include <vector>
#include <string>
#include <functional>
#include <chrono>
#include <memory>
#include <map>

namespace Recordify {
namespace ScreenHandler {

// Mouse and input tracking
struct MouseState {
    Utils::Point position;
    Utils::Point previousPosition;
    float velocity; // pixels per second
    
    struct ButtonState {
        bool pressed = false;
        std::chrono::steady_clock::time_point pressTime;
        Utils::Point pressPosition;
        int clickCount = 0; // for double/triple clicks
    };
    
    ButtonState leftButton;
    ButtonState rightButton;
    ButtonState middleButton;
    ButtonState x1Button; // Additional mouse buttons
    ButtonState x2Button;
    
    int wheelDelta = 0;
    int horizontalWheelDelta = 0;
    
    std::chrono::steady_clock::time_point timestamp;
    
    // Helper methods
    bool isAnyButtonPressed() const;
    bool isDragging() const;
    float getDistance(const Utils::Point& point) const;
    float getDistanceFromPress(int button = 0) const; // 0=left, 1=right, 2=middle
};

// Keyboard state and input tracking
struct KeyState {
    int keyCode;
    bool pressed = false;
    bool wasPressed = false; // previous frame state
    std::chrono::steady_clock::time_point pressTime;
    std::chrono::steady_clock::time_point releaseTime;
    
    bool isJustPressed() const { return pressed && !wasPressed; }
    bool isJustReleased() const { return !pressed && wasPressed; }
    float getPressedDuration() const;
};

struct KeyboardState {
    std::map<int, KeyState> keys;
    std::string lastTypedText;
    std::string currentInputText; // for IME support
    
    std::vector<int> modifierKeys; // Ctrl, Alt, Shift, etc.
    
    std::chrono::steady_clock::time_point timestamp;
    
    // Helper methods
    bool isKeyPressed(int keyCode) const;
    bool isKeyJustPressed(int keyCode) const;
    bool isKeyJustReleased(int keyCode) const;
    bool isModifierPressed(int modifier) const; // VK_CONTROL, VK_SHIFT, etc.
    std::vector<int> getPressedKeys() const;
};

// Display and screen information
struct DisplayInfo {
    int displayId;
    std::string deviceName;
    std::string manufacturerName;
    Utils::Rectangle bounds; // Physical bounds
    Utils::Rectangle workArea; // Available work area (excluding taskbar, etc.)
    
    // Display properties
    int bitsPerPixel = 24;
    int refreshRate = 60;
    float dpiX = 96.0f;
    float dpiY = 96.0f;
    float scalingFactor = 1.0f;
    
    // Display capabilities
    bool isPrimary = false;
    bool isEnabled = true;
    bool supportsHDR = false;
    int orientation = 0; // 0=normal, 90=rotated, etc.
    
    // Color profile information
    std::string colorProfile;
    float gamma = 2.2f;
    
    // Physical properties (if available)
    float physicalWidth = 0.0f;  // in millimeters
    float physicalHeight = 0.0f; // in millimeters
};

// System metrics and capabilities
struct SystemMetrics {
    // Screen metrics
    Utils::Size primaryScreenSize;
    Utils::Size virtualScreenSize; // All screens combined
    int displayCount = 1;
    
    // DPI and scaling
    float systemDpiX = 96.0f;
    float systemDpiY = 96.0f;
    float defaultScalingFactor = 1.0f;
    bool isDpiAware = false;
    
    // Cursor metrics
    Utils::Size cursorSize;
    int doubleClickTime = 500; // milliseconds
    Utils::Size doubleClickSize; // tolerance area
    
    // System capabilities
    bool hasMultiTouch = false;
    int maxTouchPoints = 0;
    bool hasStylus = false;
    
    // Performance metrics
    float cpuUsage = 0.0f;
    float memoryUsage = 0.0f;
    float gpuUsage = 0.0f;
};

// Window information and management
struct WindowInfo {
    uintptr_t windowHandle = 0;
    std::string title;
    std::string className;
    std::string processName;
    unsigned long processId = 0;
    unsigned long threadId = 0;
    
    Utils::Rectangle bounds;
    Utils::Rectangle clientBounds; // Interior area
    
    // Window state
    bool isVisible = true;
    bool isMinimized = false;
    bool isMaximized = false;
    bool isTopmost = false;
    bool isFocused = false;
    bool isResizable = true;
    
    // Window properties
    int transparency = 255; // 0-255
    std::string iconPath;
    
    // Parent-child relationships
    uintptr_t parentWindow = 0;
    std::vector<uintptr_t> childWindows;
    
    // Helper methods
    bool containsPoint(const Utils::Point& point) const;
    bool intersects(const WindowInfo& other) const;
};

// Screen capture and content analysis
struct ScreenCapture {
    Utils::Rectangle area;
    std::vector<uint8_t> pixelData;
    int width, height;
    int bitsPerPixel;
    std::chrono::steady_clock::time_point timestamp;
    
    // Color analysis
    struct ColorStats {
        Color dominantColor;
        Color averageColor;
        std::vector<Color> colorPalette;
        float brightness = 0.0f;
        float contrast = 0.0f;
        float saturation = 0.0f;
    };
    
    ColorStats analyzeColors() const;
    bool hasMotion(const ScreenCapture& previous, float threshold = 0.1f) const;
    std::vector<Utils::Rectangle> findTextRegions() const;
    std::vector<Utils::Rectangle> findChangedRegions(const ScreenCapture& previous) const;
};

// Cursor and pointer information
struct CursorInfo {
    Utils::Point position;
    Utils::Point hotspot;
    Utils::Size size;
    
    enum Type {
        ARROW, HAND, TEXT, CROSS, WAIT, HELP, 
        RESIZE_NS, RESIZE_EW, RESIZE_NWSE, RESIZE_NESW,
        MOVE, NO_DROP, CUSTOM
    } type = ARROW;
    
    bool isVisible = true;
    bool isSystemCursor = true;
    std::string customCursorPath;
    
    // Animation state for animated cursors
    int frameIndex = 0;
    int frameCount = 1;
    
    std::chrono::steady_clock::time_point timestamp;
};

// Advanced screen reading capabilities
class ScreenReader {
public:
    ScreenReader();
    ~ScreenReader();
    
    // Initialization and lifecycle
    bool initialize();
    void shutdown();
    bool isInitialized() const { return m_initialized; }
    
    // Real-time monitoring control
    void startMonitoring();
    void stopMonitoring();
    bool isMonitoring() const { return m_isMonitoring; }
    void update(); // Call regularly to process events
    void setUpdateInterval(int milliseconds);
    
    // Mouse tracking
    MouseState getCurrentMouseState() const;
    Utils::Point getMousePosition() const;
    Utils::Point getMouseVelocity() const;
    bool isMouseButtonPressed(int button) const;
    bool wasMouseButtonJustPressed(int button) const;
    bool wasMouseButtonJustReleased(int button) const;
    float getMouseButtonPressDuration(int button) const;
    
    // Advanced mouse tracking
    std::vector<Utils::Point> getMouseTrail(int maxPoints = 50) const;
    void clearMouseTrail();
    bool isMouseIdle(float seconds = 5.0f) const;
    Utils::Rectangle getMouseMovementBounds() const; // Bounding box of recent movement
    
    // Keyboard tracking
    KeyboardState getCurrentKeyboardState() const;
    bool isKeyPressed(int keyCode) const;
    bool wasKeyJustPressed(int keyCode) const;
    bool wasKeyJustReleased(int keyCode) const;
    std::string getLastTypedText() const;
    std::string getCurrentInputText() const; // For IME
    
    // Advanced keyboard tracking
    std::vector<int> getKeySequence(int maxKeys = 20) const;
    float getTypingSpeed() const; // Words per minute
    bool detectKeyboardShortcut(const std::vector<int>& keys) const;
    
    // Display and screen information
    std::vector<DisplayInfo> getDisplays() const;
    DisplayInfo getPrimaryDisplay() const;
    DisplayInfo getDisplayAt(const Utils::Point& point) const;
    DisplayInfo getDisplayContaining(const Utils::Rectangle& rect) const;
    Utils::Size getVirtualScreenSize() const;
    Utils::Rectangle getVirtualScreenBounds() const;
    
    // System metrics
    SystemMetrics getSystemMetrics() const;
    float getDisplayScaling(int displayId = -1) const; // -1 for primary
    Utils::Size getDPI(int displayId = -1) const;
    void refreshSystemMetrics();
    
    // Screen capture capabilities
    bool captureScreen(ScreenCapture& capture, const Utils::Rectangle& area = Utils::Rectangle()) const;
    bool captureWindow(ScreenCapture& capture, uintptr_t windowHandle) const;
    bool captureRegion(ScreenCapture& capture, const Utils::Rectangle& region) const;
    
    // Advanced capture features
    bool startContinuousCapture(const Utils::Rectangle& area, float fps = 30.0f);
    bool stopContinuousCapture();
    bool isContinuousCapturing() const;
    std::vector<ScreenCapture> getRecentCaptures(int count = 10) const;
    
    // Screen content analysis
    bool detectMotion(const Utils::Rectangle& area, float threshold = 0.1f) const;
    std::vector<Utils::Rectangle> findChangedRegions(const Utils::Rectangle& area) const;
    std::vector<Utils::Rectangle> detectTextRegions(const Utils::Rectangle& area) const;
    std::string extractTextFromRegion(const Utils::Rectangle& region) const; // OCR
    
    // Window management and tracking
    std::vector<WindowInfo> getVisibleWindows() const;
    std::vector<WindowInfo> getAllWindows() const;
    WindowInfo getActiveWindow() const;
    WindowInfo getForegroundWindow() const;
    WindowInfo getWindowAt(const Utils::Point& point) const;
    WindowInfo getWindowByTitle(const std::string& title) const;
    WindowInfo getWindowByClassName(const std::string& className) const;
    WindowInfo getWindowByProcessId(unsigned long processId) const;
    
    // Advanced window operations
    bool setWindowFocus(uintptr_t windowHandle);
    bool minimizeWindow(uintptr_t windowHandle);
    bool maximizeWindow(uintptr_t windowHandle);
    bool restoreWindow(uintptr_t windowHandle);
    bool closeWindow(uintptr_t windowHandle);
    bool moveWindow(uintptr_t windowHandle, const Utils::Point& position);
    bool resizeWindow(uintptr_t windowHandle, const Utils::Size& size);
    
    // Cursor information
    CursorInfo getCurrentCursor() const;
    bool isCursorVisible() const;
    Utils::Point getCursorHotspot() const;
    
    // Advanced cursor tracking
    void startCursorTracking();
    void stopCursorTracking();
    std::vector<Utils::Point> getCursorPath(int maxPoints = 100) const;
    bool isCursorOverWindow(uintptr_t windowHandle) const;
    
    // Event callbacks and notifications
    using MouseCallback = std::function<void(const MouseState&)>;
    using KeyboardCallback = std::function<void(const KeyboardState&)>;
    using WindowCallback = std::function<void(const WindowInfo&, const std::string& event)>;
    using DisplayCallback = std::function<void(const DisplayInfo&, const std::string& event)>;
    using CursorCallback = std::function<void(const CursorInfo&)>;
    using CaptureCallback = std::function<void(const ScreenCapture&)>;
    
    void setMouseCallback(MouseCallback callback);
    void setKeyboardCallback(KeyboardCallback callback);
    void setWindowCallback(WindowCallback callback);
    void setDisplayCallback(DisplayCallback callback);
    void setCursorCallback(CursorCallback callback);
    void setCaptureCallback(CaptureCallback callback);
    
    // Feature enable/disable
    void setMouseTrackingEnabled(bool enabled);
    void setKeyboardTrackingEnabled(bool enabled);
    void setWindowTrackingEnabled(bool enabled);
    void setDisplayTrackingEnabled(bool enabled);
    void setCursorTrackingEnabled(bool enabled);
    void setCaptureTrackingEnabled(bool enabled);
    
    bool isMouseTrackingEnabled() const { return m_mouseTrackingEnabled; }
    bool isKeyboardTrackingEnabled() const { return m_keyboardTrackingEnabled; }
    bool isWindowTrackingEnabled() const { return m_windowTrackingEnabled; }
    bool isDisplayTrackingEnabled() const { return m_displayTrackingEnabled; }
    bool isCursorTrackingEnabled() const { return m_cursorTrackingEnabled; }
    bool isCaptureTrackingEnabled() const { return m_captureTrackingEnabled; }
    
    // Performance and configuration
    void setHighPrecisionMouse(bool enabled);
    void setRawInput(bool enabled);
    void setUpdateRate(float hz); // Updates per second
    void setCaptureQuality(int quality); // 0-100
    
    // Debugging and diagnostics
    struct ReaderStats {
        int eventsProcessed = 0;
        float averageUpdateTime = 0.0f;
        int memoryUsage = 0;
        float cpuUsage = 0.0f;
        
        // Tracking stats
        int mouseEvents = 0;
        int keyboardEvents = 0;
        int windowEvents = 0;
        int captureEvents = 0;
    };
    
    ReaderStats getReaderStats() const;
    void resetReaderStats();
    
    // Save/load settings
    bool saveSettings(const std::string& filePath) const;
    bool loadSettings(const std::string& filePath);
    
private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
    
    bool m_initialized;
    bool m_isMonitoring;
    
    // Feature flags
    bool m_mouseTrackingEnabled;
    bool m_keyboardTrackingEnabled;
    bool m_windowTrackingEnabled;
    bool m_displayTrackingEnabled;
    bool m_cursorTrackingEnabled;
    bool m_captureTrackingEnabled;
    
    // Callbacks
    MouseCallback m_mouseCallback;
    KeyboardCallback m_keyboardCallback;
    WindowCallback m_windowCallback;
    DisplayCallback m_displayCallback;
    CursorCallback m_cursorCallback;
    CaptureCallback m_captureCallback;
    
    // Internal methods
    void processMouseEvents();
    void processKeyboardEvents();
    void processWindowEvents();
    void processDisplayEvents();
    void processCursorEvents();
    void processCaptureEvents();
    
    void notifyMouseChange(const MouseState& state);
    void notifyKeyboardChange(const KeyboardState& state);
    void notifyWindowChange(const WindowInfo& window, const std::string& event);
    void notifyDisplayChange(const DisplayInfo& display, const std::string& event);
    void notifyCursorChange(const CursorInfo& cursor);
    void notifyCapture(const ScreenCapture& capture);
    
    bool validateInitialization() const;
    void updateStats();
};

}} // namespace Recordify::ScreenHandler

#endif // RECORDIFY_SCREEN_READER_H
