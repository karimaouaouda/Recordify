#include "screen_handler/screen_writer.h"
#include <iostream>
#include <algorithm>
#include <cmath>
#include <sstream>
#include <iomanip>

namespace Recordify {
namespace ScreenHandler {

// Color static members
const Color Color::BLACK(0, 0, 0);
const Color Color::WHITE(255, 255, 255);
const Color Color::RED(255, 0, 0);
const Color Color::GREEN(0, 255, 0);
const Color Color::BLUE(0, 0, 255);
const Color Color::TRANSPARENT(0, 0, 0, 0);

// Color methods
Color Color::fromHex(const std::string& hex) {
    std::string cleanHex = hex;
    if (cleanHex[0] == '#') cleanHex = cleanHex.substr(1);
    
    if (cleanHex.length() == 6) {
        unsigned int value = std::stoul(cleanHex, nullptr, 16);
        return Color((value >> 16) & 0xFF, (value >> 8) & 0xFF, value & 0xFF);
    }
    return Color::BLACK;
}

Color Color::fromHSV(float h, float s, float v, float a) {
    float c = v * s;
    float x = c * (1 - std::abs(std::fmod(h / 60.0f, 2) - 1));
    float m = v - c;
    
    float r, g, b;
    if (h >= 0 && h < 60) { r = c; g = x; b = 0; }
    else if (h >= 60 && h < 120) { r = x; g = c; b = 0; }
    else if (h >= 120 && h < 180) { r = 0; g = c; b = x; }
    else if (h >= 180 && h < 240) { r = 0; g = x; b = c; }
    else if (h >= 240 && h < 300) { r = x; g = 0; b = c; }
    else { r = c; g = 0; b = x; }
    
    return Color(
        static_cast<uint8_t>((r + m) * 255),
        static_cast<uint8_t>((g + m) * 255),
        static_cast<uint8_t>((b + m) * 255),
        static_cast<uint8_t>(a * 255)
    );
}

std::string Color::toHex() const {
    std::stringstream ss;
    ss << "#" << std::hex << std::setfill('0') 
       << std::setw(2) << static_cast<int>(r)
       << std::setw(2) << static_cast<int>(g)
       << std::setw(2) << static_cast<int>(b);
    return ss.str();
}

// ShapeProperties copy operations
ShapeProperties::ShapeProperties(const ShapeProperties& other)
    : fillColor(other.fillColor)
    , filled(other.filled)
    , strokeColor(other.strokeColor)
    , strokeWidth(other.strokeWidth)
    , dashPattern(other.dashPattern)
    , dashOffset(other.dashOffset)
    , shadowColor(other.shadowColor)
    , shadowOffset(other.shadowOffset)
    , shadowBlur(other.shadowBlur)
    , opacity(other.opacity)
    , cornerRadius(other.cornerRadius) {
    
    if (other.fillGradient) {
        fillGradient = std::make_unique<Gradient>(*other.fillGradient);
    }
}

ShapeProperties& ShapeProperties::operator=(const ShapeProperties& other) {
    if (this != &other) {
        fillColor = other.fillColor;
        filled = other.filled;
        strokeColor = other.strokeColor;
        strokeWidth = other.strokeWidth;
        dashPattern = other.dashPattern;
        dashOffset = other.dashOffset;
        shadowColor = other.shadowColor;
        shadowOffset = other.shadowOffset;
        shadowBlur = other.shadowBlur;
        opacity = other.opacity;
        cornerRadius = other.cornerRadius;
        
        if (other.fillGradient) {
            fillGradient = std::make_unique<Gradient>(*other.fillGradient);
        } else {
            fillGradient.reset();
        }
    }
    return *this;
}

// Transform methods
Utils::Point DrawingContext::Transform::apply(const Utils::Point& point) const {
    // Apply scaling
    float x = point.x * scaleX;
    float y = point.y * scaleY;
    
    // Apply rotation
    if (rotationAngle != 0.0f) {
        float cos_a = std::cos(rotationAngle);
        float sin_a = std::sin(rotationAngle);
        float new_x = x * cos_a - y * sin_a;
        float new_y = x * sin_a + y * cos_a;
        x = new_x;
        y = new_y;
    }
    
    // Apply translation
    return Utils::Point(
        static_cast<int>(x + translation.x),
        static_cast<int>(y + translation.y)
    );
}

Utils::Rectangle DrawingContext::Transform::apply(const Utils::Rectangle& rect) const {
    Utils::Point topLeft = apply(Utils::Point(rect.x, rect.y));
    Utils::Point bottomRight = apply(Utils::Point(rect.x + rect.width, rect.y + rect.height));
    
    return Utils::Rectangle(
        topLeft.x,
        topLeft.y,
        bottomRight.x - topLeft.x,
        bottomRight.y - topLeft.y
    );
}

// Animation methods
void Animation::addKeyframe(const AnimationKeyframe& keyframe) {
    m_keyframes.push_back(keyframe);
    std::sort(m_keyframes.begin(), m_keyframes.end(),
              [](const AnimationKeyframe& a, const AnimationKeyframe& b) {
                  return a.time < b.time;
              });
}

AnimationKeyframe Animation::interpolate(float time) const {
    if (m_keyframes.empty()) {
        return AnimationKeyframe{};
    }
    
    if (m_keyframes.size() == 1 || time <= 0.0f) {
        return m_keyframes.front();
    }
    
    if (time >= 1.0f) {
        return m_keyframes.back();
    }
    
    // Find the two keyframes to interpolate between
    for (size_t i = 0; i < m_keyframes.size() - 1; ++i) {
        if (time >= m_keyframes[i].time && time <= m_keyframes[i + 1].time) {
            const auto& k1 = m_keyframes[i];
            const auto& k2 = m_keyframes[i + 1];
            
            float t = (time - k1.time) / (k2.time - k1.time);
            
            AnimationKeyframe result;
            result.time = time;
            result.position.x = static_cast<int>(k1.position.x + t * (k2.position.x - k1.position.x));
            result.position.y = static_cast<int>(k1.position.y + t * (k2.position.y - k1.position.y));
            result.opacity = k1.opacity + t * (k2.opacity - k1.opacity);
            result.scale = k1.scale + t * (k2.scale - k1.scale);
            result.rotation = k1.rotation + t * (k2.rotation - k1.rotation);
            
            return result;
        }
    }
    
    return m_keyframes.back();
}

bool Animation::isFinished(float currentTime) const {
    return currentTime >= m_duration;
}

// ScreenWriter implementation
struct ScreenWriter::Impl {
    // Canvas and rendering state
    Utils::Size canvasSize;
    bool hasCanvas = false;
    
    // Layer management
    struct Layer {
        int id;
        std::string name;
        float opacity = 1.0f;
        bool visible = true;
        std::vector<int> objects;
    };
    
    std::vector<Layer> layers;
    int activeLayerId = 0;
    int nextLayerId = 1;
    
    // Annotation management
    std::vector<ScreenWriter::Annotation> annotations;
    int nextAnnotationId = 1;
    std::vector<ScreenWriter::Annotation> undoStack;
    std::vector<ScreenWriter::Annotation> redoStack;
    
    // Animation management
    std::map<int, Animation> activeAnimations;
    std::chrono::steady_clock::time_point lastAnimationUpdate;
    
    // Render statistics
    ScreenWriter::RenderStats renderStats;
    std::chrono::steady_clock::time_point lastStatsReset;
    
    void logOperation(const std::string& operation) {
        std::cout << "[ScreenWriter] " << operation << std::endl;
    }
    
    Layer* findLayer(int layerId) {
        auto it = std::find_if(layers.begin(), layers.end(),
                              [layerId](const Layer& layer) { return layer.id == layerId; });
        return (it != layers.end()) ? &(*it) : nullptr;
    }
};

ScreenWriter::ScreenWriter()
    : m_impl(std::make_unique<Impl>())
    , m_initialized(false)
    , m_overlayEnabled(false)
    , m_realTimeDrawing(false) {
    
    // Initialize default layer
    m_impl->layers.push_back({0, "Default", 1.0f, true, {}});
    m_impl->lastAnimationUpdate = std::chrono::steady_clock::now();
    m_impl->lastStatsReset = std::chrono::steady_clock::now();
    
    std::cout << "[ScreenWriter] Created" << std::endl;
}

ScreenWriter::~ScreenWriter() {
    shutdown();
    std::cout << "[ScreenWriter] Destroyed" << std::endl;
}

bool ScreenWriter::initialize() {
    if (m_initialized) {
        m_impl->logOperation("Already initialized");
        return true;
    }
    
    m_impl->logOperation("Initializing...");
    
    // Initialize platform-specific rendering context
    // This would include setting up DirectX, OpenGL, or software rendering
    
    m_initialized = true;
    m_impl->logOperation("Initialization completed successfully");
    return true;
}

void ScreenWriter::shutdown() {
    if (!m_initialized) return;
    
    m_impl->logOperation("Shutting down...");
    
    stopRealTimeDrawing();
    disableOverlay();
    clearAnnotations();
    
    m_initialized = false;
    m_impl->logOperation("Shutdown completed");
}

bool ScreenWriter::createCanvas(const Utils::Size& size) {
    m_impl->logOperation("Creating canvas: " + std::to_string(size.width) + "x" + std::to_string(size.height));
    
    if (size.isEmpty()) {
        m_impl->logOperation("Invalid canvas size");
        return false;
    }
    
    m_impl->canvasSize = size;
    m_impl->hasCanvas = true;
    
    // Initialize rendering surface
    // Platform-specific implementation would go here
    
    clearCanvas();
    return true;
}

bool ScreenWriter::resizeCanvas(const Utils::Size& newSize) {
    if (!m_impl->hasCanvas) {
        return createCanvas(newSize);
    }
    
    m_impl->logOperation("Resizing canvas from " + std::to_string(m_impl->canvasSize.width) + "x" + 
                        std::to_string(m_impl->canvasSize.height) + " to " + 
                        std::to_string(newSize.width) + "x" + std::to_string(newSize.height));
    
    m_impl->canvasSize = newSize;
    return true;
}

void ScreenWriter::clearCanvas(const Color& backgroundColor) {
    m_impl->logOperation("Clearing canvas with color " + backgroundColor.toHex());
    
    if (!m_impl->hasCanvas) {
        m_impl->logOperation("No canvas to clear");
        return;
    }
    
    // Clear all layers and annotations
    for (auto& layer : m_impl->layers) {
        layer.objects.clear();
    }
    
    // Platform-specific canvas clearing would go here
}

// Layer management
int ScreenWriter::createLayer(const std::string& name) {
    int layerId = m_impl->nextLayerId++;
    std::string layerName = name.empty() ? ("Layer " + std::to_string(layerId)) : name;
    
    m_impl->logOperation("Creating layer: " + layerName + " (ID: " + std::to_string(layerId) + ")");
    
    m_impl->layers.push_back({layerId, layerName, 1.0f, true, {}});
    return layerId;
}

bool ScreenWriter::removeLayer(int layerId) {
    if (layerId == 0) {
        m_impl->logOperation("Cannot remove default layer");
        return false;
    }
    
    auto it = std::find_if(m_impl->layers.begin(), m_impl->layers.end(),
                          [layerId](const Impl::Layer& layer) { return layer.id == layerId; });
    
    if (it != m_impl->layers.end()) {
        m_impl->logOperation("Removing layer ID: " + std::to_string(layerId));
        m_impl->layers.erase(it);
        
        if (m_impl->activeLayerId == layerId) {
            m_impl->activeLayerId = 0; // Reset to default layer
        }
        return true;
    }
    
    m_impl->logOperation("Layer not found: " + std::to_string(layerId));
    return false;
}

bool ScreenWriter::setActiveLayer(int layerId) {
    auto* layer = m_impl->findLayer(layerId);
    if (layer) {
        m_impl->logOperation("Setting active layer to: " + std::to_string(layerId));
        m_impl->activeLayerId = layerId;
        return true;
    }
    
    m_impl->logOperation("Layer not found: " + std::to_string(layerId));
    return false;
}

bool ScreenWriter::setLayerOpacity(int layerId, float opacity) {
    auto* layer = m_impl->findLayer(layerId);
    if (layer) {
        opacity = std::clamp(opacity, 0.0f, 1.0f);
        m_impl->logOperation("Setting layer " + std::to_string(layerId) + " opacity to: " + std::to_string(opacity));
        layer->opacity = opacity;
        return true;
    }
    return false;
}

bool ScreenWriter::setLayerVisible(int layerId, bool visible) {
    auto* layer = m_impl->findLayer(layerId);
    if (layer) {
        m_impl->logOperation("Setting layer " + std::to_string(layerId) + " visibility to: " + (visible ? "true" : "false"));
        layer->visible = visible;
        return true;
    }
    return false;
}

std::vector<int> ScreenWriter::getLayerIds() const {
    std::vector<int> ids;
    for (const auto& layer : m_impl->layers) {
        ids.push_back(layer.id);
    }
    return ids;
}

// Drawing context management
void ScreenWriter::pushContext() {
    m_contextStack.push_back(m_currentContext);
    m_impl->logOperation("Pushed drawing context (stack size: " + std::to_string(m_contextStack.size()) + ")");
}

void ScreenWriter::popContext() {
    if (!m_contextStack.empty()) {
        m_currentContext = m_contextStack.back();
        m_contextStack.pop_back();
        m_impl->logOperation("Popped drawing context (stack size: " + std::to_string(m_contextStack.size()) + ")");
    } else {
        m_impl->logOperation("Context stack is empty, cannot pop");
    }
}

void ScreenWriter::setClipRect(const Utils::Rectangle& rect) {
    m_currentContext.clipRect = rect;
    m_impl->logOperation("Set clip rect: [" + std::to_string(rect.x) + "," + std::to_string(rect.y) + "," +
                        std::to_string(rect.width) + "," + std::to_string(rect.height) + "]");
}

void ScreenWriter::clearClipRect() {
    m_currentContext.clipRect = Utils::Rectangle();
    m_impl->logOperation("Cleared clip rect");
}

void ScreenWriter::setGlobalOpacity(float opacity) {
    m_currentContext.globalOpacity = std::clamp(opacity, 0.0f, 1.0f);
    m_impl->logOperation("Set global opacity: " + std::to_string(m_currentContext.globalOpacity));
}

void ScreenWriter::setTransform(const DrawingContext::Transform& transform) {
    m_currentContext.transform = transform;
    m_impl->logOperation("Set transform: scale(" + std::to_string(transform.scaleX) + "," + 
                        std::to_string(transform.scaleY) + ") rotate(" + std::to_string(transform.rotationAngle) + 
                        ") translate(" + std::to_string(transform.translation.x) + "," + 
                        std::to_string(transform.translation.y) + ")");
}

void ScreenWriter::resetTransform() {
    m_currentContext.transform = DrawingContext::Transform();
    m_impl->logOperation("Reset transform to identity");
}

// Text rendering
bool ScreenWriter::drawText(const std::string& text, const Utils::Point& position, const TextProperties& properties) {
    if (!m_initialized || text.empty()) return false;
    
    m_impl->logOperation("Drawing text: '" + text + "' at (" + std::to_string(position.x) + "," + std::to_string(position.y) + ")");
    
    // Apply transform
    Utils::Point transformedPos = m_currentContext.transform.apply(position);
    
    // Check clipping
    if (!m_currentContext.clipRect.isEmpty() && !m_currentContext.clipRect.contains(transformedPos)) {
        m_impl->logOperation("Text clipped, skipping");
        return false;
    }
    
    // Platform-specific text rendering would go here
    notifyDrawing(transformedPos, "text");
    updateRenderStats();
    
    return true;
}

bool ScreenWriter::drawTextInRect(const std::string& text, const Utils::Rectangle& bounds, const TextProperties& properties) {
    if (!m_initialized || text.empty()) return false;
    
    m_impl->logOperation("Drawing text in rect: '" + text + "' in bounds [" + 
                        std::to_string(bounds.x) + "," + std::to_string(bounds.y) + "," +
                        std::to_string(bounds.width) + "," + std::to_string(bounds.height) + "]");
    
    // Apply transform
    Utils::Rectangle transformedBounds = m_currentContext.transform.apply(bounds);
    
    // Platform-specific bounded text rendering would go here
    // This would handle word wrapping, alignment, etc.
    
    updateRenderStats();
    return true;
}

// Shape drawing
bool ScreenWriter::drawRectangle(const Utils::Rectangle& rect, const ShapeProperties& properties) {
    if (!m_initialized) return false;
    
    m_impl->logOperation("Drawing rectangle: [" + std::to_string(rect.x) + "," + std::to_string(rect.y) + "," +
                        std::to_string(rect.width) + "," + std::to_string(rect.height) + "]");
    
    Utils::Rectangle transformedRect = m_currentContext.transform.apply(rect);
    
    // Platform-specific rectangle rendering would go here
    notifyDrawing(transformedRect.topLeft(), "rectangle");
    updateRenderStats();
    
    return true;
}

bool ScreenWriter::drawCircle(const Utils::Point& center, float radius, const ShapeProperties& properties) {
    if (!m_initialized || radius <= 0) return false;
    
    m_impl->logOperation("Drawing circle: center(" + std::to_string(center.x) + "," + std::to_string(center.y) + 
                        ") radius(" + std::to_string(radius) + ")");
    
    Utils::Point transformedCenter = m_currentContext.transform.apply(center);
    
    // Platform-specific circle rendering would go here
    notifyDrawing(transformedCenter, "circle");
    updateRenderStats();
    
    return true;
}

bool ScreenWriter::drawLine(const Utils::Point& start, const Utils::Point& end, const ShapeProperties& properties) {
    if (!m_initialized) return false;
    
    m_impl->logOperation("Drawing line: (" + std::to_string(start.x) + "," + std::to_string(start.y) + 
                        ") to (" + std::to_string(end.x) + "," + std::to_string(end.y) + ")");
    
    Utils::Point transformedStart = m_currentContext.transform.apply(start);
    Utils::Point transformedEnd = m_currentContext.transform.apply(end);
    
    // Platform-specific line rendering would go here
    notifyDrawing(transformedStart, "line");
    updateRenderStats();
    
    return true;
}

// Overlay operations
bool ScreenWriter::enableOverlay() {
    if (m_overlayEnabled) return true;
    
    m_impl->logOperation("Enabling screen overlay");
    
    // Platform-specific overlay initialization would go here
    // This would create a transparent window that covers the screen
    
    m_overlayEnabled = true;
    return true;
}

bool ScreenWriter::disableOverlay() {
    if (!m_overlayEnabled) return true;
    
    m_impl->logOperation("Disabling screen overlay");
    
    // Platform-specific overlay cleanup would go here
    
    m_overlayEnabled = false;
    return true;
}

bool ScreenWriter::startRealTimeDrawing() {
    if (m_realTimeDrawing) return true;
    
    m_impl->logOperation("Starting real-time drawing mode");
    
    if (!enableOverlay()) {
        m_impl->logOperation("Failed to enable overlay for real-time drawing");
        return false;
    }
    
    m_realTimeDrawing = true;
    return true;
}

bool ScreenWriter::stopRealTimeDrawing() {
    if (!m_realTimeDrawing) return true;
    
    m_impl->logOperation("Stopping real-time drawing mode");
    
    m_realTimeDrawing = false;
    return true;
}

// Annotation management
int ScreenWriter::addAnnotation(const Annotation& annotation) {
    Annotation newAnnotation = annotation;
    newAnnotation.id = m_impl->nextAnnotationId++;
    newAnnotation.timestamp = std::chrono::steady_clock::now();
    
    m_impl->annotations.push_back(newAnnotation);
    m_impl->undoStack.push_back(newAnnotation);
    m_impl->redoStack.clear(); // Clear redo stack on new action
    
    m_impl->logOperation("Added annotation ID: " + std::to_string(newAnnotation.id));
    notifyAnnotation(newAnnotation);
    
    return newAnnotation.id;
}

bool ScreenWriter::removeAnnotation(int annotationId) {
    auto it = std::find_if(m_impl->annotations.begin(), m_impl->annotations.end(),
                          [annotationId](const Annotation& ann) { return ann.id == annotationId; });
    
    if (it != m_impl->annotations.end()) {
        m_impl->logOperation("Removing annotation ID: " + std::to_string(annotationId));
        m_impl->annotations.erase(it);
        return true;
    }
    
    m_impl->logOperation("Annotation not found: " + std::to_string(annotationId));
    return false;
}

std::vector<ScreenWriter::Annotation> ScreenWriter::getAnnotations() const {
    return m_impl->annotations;
}

void ScreenWriter::clearAnnotations() {
    m_impl->logOperation("Clearing all annotations (" + std::to_string(m_impl->annotations.size()) + " items)");
    m_impl->annotations.clear();
    m_impl->undoStack.clear();
    m_impl->redoStack.clear();
}

void ScreenWriter::undoLastAnnotation() {
    if (!m_impl->undoStack.empty()) {
        auto annotation = m_impl->undoStack.back();
        m_impl->undoStack.pop_back();
        m_impl->redoStack.push_back(annotation);
        
        removeAnnotation(annotation.id);
        m_impl->logOperation("Undid annotation ID: " + std::to_string(annotation.id));
    } else {
        m_impl->logOperation("Nothing to undo");
    }
}

void ScreenWriter::redoAnnotation() {
    if (!m_impl->redoStack.empty()) {
        auto annotation = m_impl->redoStack.back();
        m_impl->redoStack.pop_back();
        
        addAnnotation(annotation);
        m_impl->logOperation("Redid annotation");
    } else {
        m_impl->logOperation("Nothing to redo");
    }
}

// Configuration
void ScreenWriter::setDefaultTextProperties(const TextProperties& properties) {
    m_defaultTextProps = properties;
    m_impl->logOperation("Updated default text properties");
}

void ScreenWriter::setDefaultShapeProperties(const ShapeProperties& properties) {
    m_defaultShapeProps = properties;
    m_impl->logOperation("Updated default shape properties");
}

// Statistics
ScreenWriter::RenderStats ScreenWriter::getRenderStats() const {
    return m_impl->renderStats;
}

void ScreenWriter::resetRenderStats() {
    m_impl->renderStats = RenderStats();
    m_impl->lastStatsReset = std::chrono::steady_clock::now();
    m_impl->logOperation("Reset render statistics");
}

// Private methods
void ScreenWriter::notifyDrawing(const Utils::Point& position, const std::string& action) {
    if (m_drawingCallback) {
        m_drawingCallback(position, action);
    }
}

void ScreenWriter::notifyAnnotation(const Annotation& annotation) {
    if (m_annotationCallback) {
        m_annotationCallback(annotation);
    }
}

bool ScreenWriter::validateContext() const {
    return m_initialized && m_impl->hasCanvas;
}

void ScreenWriter::updateRenderStats() {
    m_impl->renderStats.objectsRendered++;
    
    auto now = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - m_impl->lastStatsReset);
    m_impl->renderStats.renderTime = duration.count();
}

}} // namespace Recordify::ScreenHandler
