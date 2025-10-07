#include "screen_handler/screen_reader.h"
#include <iostream>
#include <algorithm>
#include <random>
#include <thread>
#include <cmath>

namespace Recordify {
namespace ScreenHandler {

// MouseState helper methods
bool MouseState::isAnyButtonPressed() const {
    return leftButton.pressed || rightButton.pressed || middleButton.pressed || 
           x1Button.pressed || x2Button.pressed;
}

bool MouseState::isDragging() const {
    return isAnyButtonPressed() && getDistance(previousPosition) > 5.0f;
}

float MouseState::getDistance(const Utils::Point& point) const {
    float dx = static_cast<float>(position.x - point.x);
    float dy = static_cast<float>(position.y - point.y);
    return std::sqrt(dx * dx + dy * dy);
}

float MouseState::getDistanceFromPress(int button) const {
    const ButtonState* buttonState = nullptr;
    switch (button) {
        case 0: buttonState = &leftButton; break;
        case 1: buttonState = &rightButton; break;
        case 2: buttonState = &middleButton; break;
        default: return 0.0f;
    }
    
    if (buttonState && buttonState->pressed) {
        return getDistance(buttonState->pressPosition);
    }
    return 0.0f;
}

// KeyState helper methods
float KeyState::getPressedDuration() const {
    if (!pressed) return 0.0f;
    
    auto now = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - pressTime);
    return duration.count() / 1000.0f; // Convert to seconds
}

// KeyboardState helper methods
bool KeyboardState::isKeyPressed(int keyCode) const {
    auto it = keys.find(keyCode);
    return it != keys.end() && it->second.pressed;
}

bool KeyboardState::isKeyJustPressed(int keyCode) const {
    auto it = keys.find(keyCode);
    return it != keys.end() && it->second.isJustPressed();
}

bool KeyboardState::isKeyJustReleased(int keyCode) const {
    auto it = keys.find(keyCode);
    return it != keys.end() && it->second.isJustReleased();
}

bool KeyboardState::isModifierPressed(int modifier) const {
    return std::find(modifierKeys.begin(), modifierKeys.end(), modifier) != modifierKeys.end();
}

std::vector<int> KeyboardState::getPressedKeys() const {
    std::vector<int> pressedKeys;
    for (const auto& pair : keys) {
        if (pair.second.pressed) {
            pressedKeys.push_back(pair.first);
        }
    }
    return pressedKeys;
}

// WindowInfo helper methods
bool WindowInfo::containsPoint(const Utils::Point& point) const {
    return bounds.contains(point);
}

bool WindowInfo::intersects(const WindowInfo& other) const {
    return bounds.intersects(other.bounds);
}

// ScreenCapture analysis methods
ScreenCapture::ColorStats ScreenCapture::analyzeColors() const {
    ColorStats stats;
    
    if (pixelData.empty() || width == 0 || height == 0) {
        return stats;
    }
    
    // Simplified color analysis
    uint64_t totalR = 0, totalG = 0, totalB = 0;
    int pixelCount = width * height;
    int bytesPerPixel = bitsPerPixel / 8;
    
    for (int i = 0; i < pixelCount && i * bytesPerPixel + 2 < pixelData.size(); ++i) {
        int offset = i * bytesPerPixel;
        uint8_t b = pixelData[offset];
        uint8_t g = pixelData[offset + 1];
        uint8_t r = pixelData[offset + 2];
        
        totalR += r;
        totalG += g;
        totalB += b;
    }
    
    if (pixelCount > 0) {
        stats.averageColor = Color(
            static_cast<uint8_t>(totalR / pixelCount),
            static_cast<uint8_t>(totalG / pixelCount),
            static_cast<uint8_t>(totalB / pixelCount)
        );
        
        // Simplified brightness calculation
        stats.brightness = (totalR + totalG + totalB) / (3.0f * 255.0f * pixelCount);
    }
    
    stats.dominantColor = stats.averageColor; // Simplified
    stats.contrast = 0.5f; // Placeholder
    stats.saturation = 0.5f; // Placeholder
    
    return stats;
}

bool ScreenCapture::hasMotion(const ScreenCapture& previous, float threshold) const {
    if (pixelData.size() != previous.pixelData.size()) {
        return true; // Different sizes = motion
    }
    
    uint64_t differences = 0;
    for (size_t i = 0; i < pixelData.size(); ++i) {
        if (std::abs(static_cast<int>(pixelData[i]) - static_cast<int>(previous.pixelData[i])) > 10) {
            differences++;
        }
    }
    
    float changeRatio = static_cast<float>(differences) / pixelData.size();
    return changeRatio > threshold;
}

// ScreenReader implementation
struct ScreenReader::Impl {
    // Current states
    MouseState currentMouseState;
    KeyboardState currentKeyboardState;
    std::vector<DisplayInfo> displays;
    SystemMetrics systemMetrics;
    std::vector<WindowInfo> windows;
    CursorInfo cursorInfo;
    
    // Tracking data
    std::vector<Utils::Point> mouseTrail;
    std::vector<int> keySequence;
    std::vector<ScreenCapture> recentCaptures;
    
    // Statistics
    ScreenReader::ReaderStats stats;
    
    // Simulation helpers
    std::mt19937 rng{std::random_device{}()};
    std::chrono::steady_clock::time_point lastUpdate;
    std::chrono::steady_clock::time_point lastMouseMove;
    
    void logOperation(const std::string& operation) {
        std::cout << "[ScreenReader] " << operation << std::endl;
    }
    
    void initializeDisplays() {
        displays.clear();
        
        // Create primary display
        DisplayInfo primary;
        primary.displayId = 0;
        primary.deviceName = "Primary Display";
        primary.manufacturerName = "Generic";
        primary.bounds = Utils::Rectangle(0, 0, 1920, 1080);
        primary.workArea = Utils::Rectangle(0, 0, 1920, 1040); // Account for taskbar
        primary.bitsPerPixel = 24;
        primary.refreshRate = 60;
        primary.dpiX = 96.0f;
        primary.dpiY = 96.0f;
        primary.scalingFactor = 1.0f;
        primary.isPrimary = true;
        primary.isEnabled = true;
        primary.colorProfile = "sRGB";
        primary.gamma = 2.2f;
        
        displays.push_back(primary);
        
        logOperation("Initialized " + std::to_string(displays.size()) + " display(s)");
    }
    
    void initializeSystemMetrics() {
        systemMetrics.primaryScreenSize = Utils::Size(1920, 1080);
        systemMetrics.virtualScreenSize = Utils::Size(1920, 1080);
        systemMetrics.displayCount = 1;
        systemMetrics.systemDpiX = 96.0f;
        systemMetrics.systemDpiY = 96.0f;
        systemMetrics.defaultScalingFactor = 1.0f;
        systemMetrics.isDpiAware = true;
        systemMetrics.cursorSize = Utils::Size(32, 32);
        systemMetrics.doubleClickTime = 500;
        systemMetrics.doubleClickSize = Utils::Size(4, 4);
        systemMetrics.hasMultiTouch = false;
        systemMetrics.maxTouchPoints = 0;
        systemMetrics.hasStylus = false;
        
        logOperation("Initialized system metrics");
    }
    
    void simulateMouseMovement() {
        // Simulate realistic mouse movement
        std::uniform_int_distribution<int> xDist(0, 1919);
        std::uniform_int_distribution<int> yDist(0, 1079);
        
        Utils::Point newPos(xDist(rng), yDist(rng));
        
        // Calculate velocity
        auto now = std::chrono::steady_clock::now();
        if (lastMouseMove != std::chrono::steady_clock::time_point{}) {
            auto timeDiff = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastMouseMove);
            if (timeDiff.count() > 0) {
                float distance = currentMouseState.getDistance(newPos);
                currentMouseState.velocity = distance / (timeDiff.count() / 1000.0f);
            }
        }
        
        currentMouseState.previousPosition = currentMouseState.position;
        currentMouseState.position = newPos;
        currentMouseState.timestamp = now;
        lastMouseMove = now;
        
        // Add to trail
        mouseTrail.push_back(newPos);
        if (mouseTrail.size() > 100) {
            mouseTrail.erase(mouseTrail.begin());
        }
    }
    
    void updateWindows() {
        windows.clear();
        
        // Simulate some windows
        WindowInfo desktop;
        desktop.windowHandle = 1;
        desktop.title = "Desktop";
        desktop.className = "Progman";
        desktop.processName = "explorer.exe";
        desktop.processId = 1000;
        desktop.bounds = Utils::Rectangle(0, 0, 1920, 1080);
        desktop.clientBounds = desktop.bounds;
        desktop.isVisible = true;
        desktop.isFocused = false;
        windows.push_back(desktop);
        
        WindowInfo notepad;
        notepad.windowHandle = 2;
        notepad.title = "Untitled - Notepad";
        notepad.className = "Notepad";
        notepad.processName = "notepad.exe";
        notepad.processId = 2000;
        notepad.bounds = Utils::Rectangle(100, 100, 800, 600);
        notepad.clientBounds = Utils::Rectangle(108, 130, 784, 570);
        notepad.isVisible = true;
        notepad.isFocused = true;
        windows.push_back(notepad);
    }
    
    void updateCursor() {
        cursorInfo.position = currentMouseState.position;
        cursorInfo.type = CursorInfo::ARROW;
        cursorInfo.isVisible = true;
        cursorInfo.isSystemCursor = true;
        cursorInfo.size = Utils::Size(32, 32);
        cursorInfo.hotspot = Utils::Point(0, 0);
        cursorInfo.timestamp = std::chrono::steady_clock::now();
    }
};

ScreenReader::ScreenReader()
    : m_impl(std::make_unique<Impl>())
    , m_initialized(false)
    , m_isMonitoring(false)
    , m_mouseTrackingEnabled(true)
    , m_keyboardTrackingEnabled(true)
    , m_windowTrackingEnabled(true)
    , m_displayTrackingEnabled(true)
    , m_cursorTrackingEnabled(true)
    , m_captureTrackingEnabled(true) {
    
    m_impl->lastUpdate = std::chrono::steady_clock::now();
    std::cout << "[ScreenReader] Created" << std::endl;
}

ScreenReader::~ScreenReader() {
    shutdown();
    std::cout << "[ScreenReader] Destroyed" << std::endl;
}

bool ScreenReader::initialize() {
    if (m_initialized) {
        m_impl->logOperation("Already initialized");
        return true;
    }
    
    m_impl->logOperation("Initializing...");
    
    m_impl->initializeDisplays();
    m_impl->initializeSystemMetrics();
    m_impl->updateWindows();
    
    // Initialize mouse state
    m_impl->currentMouseState.position = Utils::Point(960, 540); // Center of screen
    m_impl->currentMouseState.previousPosition = m_impl->currentMouseState.position;
    m_impl->currentMouseState.timestamp = std::chrono::steady_clock::now();
    
    // Initialize keyboard state
    m_impl->currentKeyboardState.timestamp = std::chrono::steady_clock::now();
    
    m_initialized = true;
    m_impl->logOperation("Initialization completed successfully");
    return true;
}

void ScreenReader::shutdown() {
    if (!m_initialized) return;
    
    m_impl->logOperation("Shutting down...");
    
    stopMonitoring();
    
    m_initialized = false;
    m_impl->logOperation("Shutdown completed");
}

void ScreenReader::startMonitoring() {
    if (m_isMonitoring) return;
    
    m_impl->logOperation("Starting monitoring");
    m_isMonitoring = true;
    m_impl->stats = ReaderStats{}; // Reset stats
}

void ScreenReader::stopMonitoring() {
    if (!m_isMonitoring) return;
    
    m_impl->logOperation("Stopping monitoring");
    m_isMonitoring = false;
}

void ScreenReader::update() {
    if (!m_isMonitoring || !m_initialized) return;
    
    auto now = std::chrono::steady_clock::now();
    auto timeSinceLastUpdate = std::chrono::duration_cast<std::chrono::milliseconds>(now - m_impl->lastUpdate);
    
    // Update at most 60 FPS
    if (timeSinceLastUpdate.count() < 16) return;
    
    m_impl->lastUpdate = now;
    
    // Process events for enabled features
    if (m_mouseTrackingEnabled) {
        processMouseEvents();
    }
    
    if (m_keyboardTrackingEnabled) {
        processKeyboardEvents();
    }
    
    if (m_windowTrackingEnabled) {
        processWindowEvents();
    }
    
    if (m_cursorTrackingEnabled) {
        processCursorEvents();
    }
    
    updateStats();
}

// Mouse tracking
MouseState ScreenReader::getCurrentMouseState() const {
    return m_impl->currentMouseState;
}

Utils::Point ScreenReader::getMousePosition() const {
    return m_impl->currentMouseState.position;
}

Utils::Point ScreenReader::getMouseVelocity() const {
    float vx = m_impl->currentMouseState.velocity * 
               (m_impl->currentMouseState.position.x - m_impl->currentMouseState.previousPosition.x);
    float vy = m_impl->currentMouseState.velocity * 
               (m_impl->currentMouseState.position.y - m_impl->currentMouseState.previousPosition.y);
    return Utils::Point(static_cast<int>(vx), static_cast<int>(vy));
}

bool ScreenReader::isMouseButtonPressed(int button) const {
    switch (button) {
        case 0: return m_impl->currentMouseState.leftButton.pressed;
        case 1: return m_impl->currentMouseState.rightButton.pressed;
        case 2: return m_impl->currentMouseState.middleButton.pressed;
        case 3: return m_impl->currentMouseState.x1Button.pressed;
        case 4: return m_impl->currentMouseState.x2Button.pressed;
        default: return false;
    }
}

std::vector<Utils::Point> ScreenReader::getMouseTrail(int maxPoints) const {
    std::vector<Utils::Point> trail = m_impl->mouseTrail;
    if (trail.size() > static_cast<size_t>(maxPoints)) {
        trail.erase(trail.begin(), trail.end() - maxPoints);
    }
    return trail;
}

void ScreenReader::clearMouseTrail() {
    m_impl->mouseTrail.clear();
    m_impl->logOperation("Cleared mouse trail");
}

bool ScreenReader::isMouseIdle(float seconds) const {
    auto now = std::chrono::steady_clock::now();
    auto idleDuration = std::chrono::duration_cast<std::chrono::milliseconds>(now - m_impl->lastMouseMove);
    return idleDuration.count() / 1000.0f > seconds;
}

// Keyboard tracking
KeyboardState ScreenReader::getCurrentKeyboardState() const {
    return m_impl->currentKeyboardState;
}

bool ScreenReader::isKeyPressed(int keyCode) const {
    return m_impl->currentKeyboardState.isKeyPressed(keyCode);
}

std::string ScreenReader::getLastTypedText() const {
    return m_impl->currentKeyboardState.lastTypedText;
}

// Display information
std::vector<DisplayInfo> ScreenReader::getDisplays() const {
    return m_impl->displays;
}

DisplayInfo ScreenReader::getPrimaryDisplay() const {
    for (const auto& display : m_impl->displays) {
        if (display.isPrimary) {
            return display;
        }
    }
    return m_impl->displays.empty() ? DisplayInfo{} : m_impl->displays[0];
}

DisplayInfo ScreenReader::getDisplayAt(const Utils::Point& point) const {
    for (const auto& display : m_impl->displays) {
        if (display.bounds.contains(point)) {
            return display;
        }
    }
    return getPrimaryDisplay();
}

Utils::Size ScreenReader::getVirtualScreenSize() const {
    return m_impl->systemMetrics.virtualScreenSize;
}

Utils::Rectangle ScreenReader::getVirtualScreenBounds() const {
    if (m_impl->displays.empty()) {
        return Utils::Rectangle(0, 0, 1920, 1080);
    }
    
    int minX = m_impl->displays[0].bounds.x;
    int minY = m_impl->displays[0].bounds.y;
    int maxX = m_impl->displays[0].bounds.x + m_impl->displays[0].bounds.width;
    int maxY = m_impl->displays[0].bounds.y + m_impl->displays[0].bounds.height;
    
    for (const auto& display : m_impl->displays) {
        minX = std::min(minX, display.bounds.x);
        minY = std::min(minY, display.bounds.y);
        maxX = std::max(maxX, display.bounds.x + display.bounds.width);
        maxY = std::max(maxY, display.bounds.y + display.bounds.height);
    }
    
    return Utils::Rectangle(minX, minY, maxX - minX, maxY - minY);
}

// System metrics
SystemMetrics ScreenReader::getSystemMetrics() const {
    return m_impl->systemMetrics;
}

float ScreenReader::getDisplayScaling(int displayId) const {
    if (displayId == -1) {
        return getPrimaryDisplay().scalingFactor;
    }
    
    for (const auto& display : m_impl->displays) {
        if (display.displayId == displayId) {
            return display.scalingFactor;
        }
    }
    
    return 1.0f;
}

Utils::Size ScreenReader::getDPI(int displayId) const {
    DisplayInfo display = (displayId == -1) ? getPrimaryDisplay() : 
                         (displayId < m_impl->displays.size() ? m_impl->displays[displayId] : getPrimaryDisplay());
    
    return Utils::Size(static_cast<int>(display.dpiX), static_cast<int>(display.dpiY));
}

// Screen capture
bool ScreenReader::captureScreen(ScreenCapture& capture, const Utils::Rectangle& area) const {
    Utils::Rectangle captureArea = area.isEmpty() ? getPrimaryDisplay().bounds : area;
    
    m_impl->logOperation("Capturing screen area: [" + std::to_string(captureArea.x) + "," + 
                        std::to_string(captureArea.y) + "," + std::to_string(captureArea.width) + "," + 
                        std::to_string(captureArea.height) + "]");
    
    capture.area = captureArea;
    capture.width = captureArea.width;
    capture.height = captureArea.height;
    capture.bitsPerPixel = 24;
    capture.timestamp = std::chrono::steady_clock::now();
    
    // Simulate pixel data
    int pixelCount = capture.width * capture.height;
    int bytesPerPixel = capture.bitsPerPixel / 8;
    capture.pixelData.resize(pixelCount * bytesPerPixel);
    
    // Fill with simulated data
    std::fill(capture.pixelData.begin(), capture.pixelData.end(), 128);
    
    return true;
}

// Window management
std::vector<WindowInfo> ScreenReader::getVisibleWindows() const {
    std::vector<WindowInfo> visibleWindows;
    std::copy_if(m_impl->windows.begin(), m_impl->windows.end(),
                 std::back_inserter(visibleWindows),
                 [](const WindowInfo& window) { return window.isVisible; });
    return visibleWindows;
}

std::vector<WindowInfo> ScreenReader::getAllWindows() const {
    return m_impl->windows;
}

WindowInfo ScreenReader::getActiveWindow() const {
    for (const auto& window : m_impl->windows) {
        if (window.isFocused) {
            return window;
        }
    }
    return m_impl->windows.empty() ? WindowInfo{} : m_impl->windows[0];
}

WindowInfo ScreenReader::getWindowAt(const Utils::Point& point) const {
    for (const auto& window : m_impl->windows) {
        if (window.isVisible && window.containsPoint(point)) {
            return window;
        }
    }
    return WindowInfo{};
}

// Cursor information
CursorInfo ScreenReader::getCurrentCursor() const {
    return m_impl->cursorInfo;
}

bool ScreenReader::isCursorVisible() const {
    return m_impl->cursorInfo.isVisible;
}

// Statistics
ScreenReader::ReaderStats ScreenReader::getReaderStats() const {
    return m_impl->stats;
}

void ScreenReader::resetReaderStats() {
    m_impl->stats = ReaderStats{};
    m_impl->logOperation("Reset reader statistics");
}

// Event callbacks
void ScreenReader::setMouseCallback(MouseCallback callback) {
    m_mouseCallback = callback;
}

void ScreenReader::setKeyboardCallback(KeyboardCallback callback) {
    m_keyboardCallback = callback;
}

void ScreenReader::setWindowCallback(WindowCallback callback) {
    m_windowCallback = callback;
}

// Feature toggles
void ScreenReader::setMouseTrackingEnabled(bool enabled) {
    m_mouseTrackingEnabled = enabled;
    m_impl->logOperation("Mouse tracking " + std::string(enabled ? "enabled" : "disabled"));
}

void ScreenReader::setKeyboardTrackingEnabled(bool enabled) {
    m_keyboardTrackingEnabled = enabled;
    m_impl->logOperation("Keyboard tracking " + std::string(enabled ? "enabled" : "disabled"));
}

// Private methods
void ScreenReader::processMouseEvents() {
    m_impl->simulateMouseMovement();
    m_impl->stats.mouseEvents++;
    
    if (m_mouseCallback) {
        m_mouseCallback(m_impl->currentMouseState);
    }
}

void ScreenReader::processKeyboardEvents() {
    // Update keyboard state
    m_impl->currentKeyboardState.timestamp = std::chrono::steady_clock::now();
    m_impl->stats.keyboardEvents++;
    
    if (m_keyboardCallback) {
        m_keyboardCallback(m_impl->currentKeyboardState);
    }
}

void ScreenReader::processWindowEvents() {
    m_impl->updateWindows();
    m_impl->stats.windowEvents++;
    
    if (m_windowCallback) {
        for (const auto& window : m_impl->windows) {
            m_windowCallback(window, "update");
        }
    }
}

void ScreenReader::processCursorEvents() {
    m_impl->updateCursor();
    
    if (m_cursorCallback) {
        m_cursorCallback(m_impl->cursorInfo);
    }
}

void ScreenReader::updateStats() {
    m_impl->stats.eventsProcessed++;
    
    auto now = std::chrono::steady_clock::now();
    auto updateTime = std::chrono::duration_cast<std::chrono::microseconds>(now - m_impl->lastUpdate);
    m_impl->stats.averageUpdateTime = updateTime.count() / 1000.0f; // Convert to milliseconds
}

}} // namespace Recordify::ScreenHandler
