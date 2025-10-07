#ifndef RECORDIFY_SCREEN_WRITER_H
#define RECORDIFY_SCREEN_WRITER_H

#include "utils/geometry.h"
#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <chrono>

namespace Recordify {
namespace ScreenHandler {

// Forward declarations
class ScreenCanvas;
class DrawingLayer;

// Drawing primitives and styles
enum class ShapeType {
    RECTANGLE,
    CIRCLE,
    ELLIPSE,
    LINE,
    ARROW,
    POLYGON,
    FREEHAND,
    BEZIER_CURVE
};

enum class TextAlignment {
    LEFT,
    CENTER,
    RIGHT,
    JUSTIFY
};

enum class TextStyle {
    NORMAL = 0x00,
    BOLD = 0x01,
    ITALIC = 0x02,
    UNDERLINE = 0x04,
    STRIKETHROUGH = 0x08
};

// Color and gradient support
struct Color {
    uint8_t r, g, b, a;
    
    Color(uint8_t red = 0, uint8_t green = 0, uint8_t blue = 0, uint8_t alpha = 255)
        : r(red), g(green), b(blue), a(alpha) {}
    
    static Color fromHex(const std::string& hex);
    static Color fromHSV(float h, float s, float v, float a = 1.0f);
    std::string toHex() const;
    
    // Predefined colors
    static const Color BLACK;
    static const Color WHITE;
    static const Color RED;
    static const Color GREEN;
    static const Color BLUE;
    static const Color TRANSPARENT;
};

struct Gradient {
    enum Type { LINEAR, RADIAL, CONIC };
    
    Type type = LINEAR;
    std::vector<std::pair<float, Color>> stops; // position (0-1), color
    Utils::Point startPoint;
    Utils::Point endPoint;
    float angle = 0.0f; // for linear gradients
};

// Advanced text properties
struct FontProperties {
    std::string family = "Arial";
    float size = 12.0f;
    int style = static_cast<int>(TextStyle::NORMAL);
    float letterSpacing = 0.0f;
    float lineHeight = 1.2f;
    bool antiAliasing = true;
    
    bool isBold() const { return style & static_cast<int>(TextStyle::BOLD); }
    bool isItalic() const { return style & static_cast<int>(TextStyle::ITALIC); }
    bool isUnderlined() const { return style & static_cast<int>(TextStyle::UNDERLINE); }
};

struct TextProperties {
    FontProperties font;
    Color color = Color::BLACK;
    TextAlignment alignment = TextAlignment::LEFT;
    bool wordWrap = true;
    int maxWidth = -1; // -1 for no limit
    
    // Advanced features
    Color shadowColor = Color::TRANSPARENT;
    Utils::Point shadowOffset;
    float shadowBlur = 0.0f;
    Color outlineColor = Color::TRANSPARENT;
    float outlineWidth = 0.0f;
};

// Advanced shape properties
struct ShapeProperties {
    // Fill properties
    Color fillColor = Color::WHITE;
    std::unique_ptr<Gradient> fillGradient;
    bool filled = true;
    
    // Stroke properties
    Color strokeColor = Color::BLACK;
    float strokeWidth = 1.0f;
    std::vector<float> dashPattern; // empty for solid line
    float dashOffset = 0.0f;
    
    // Advanced effects
    Color shadowColor = Color::TRANSPARENT;
    Utils::Point shadowOffset;
    float shadowBlur = 0.0f;
    float opacity = 1.0f;
    
    // Corner radius for rectangles
    float cornerRadius = 0.0f;
    
    ShapeProperties() = default;
    ShapeProperties(const ShapeProperties& other);
    ShapeProperties& operator=(const ShapeProperties& other);
};

// Drawing context for advanced operations
struct DrawingContext {
    Utils::Rectangle clipRect;
    float globalOpacity = 1.0f;
    bool antiAliasing = true;
    
    // Transformation matrix support
    struct Transform {
        float scaleX = 1.0f, scaleY = 1.0f;
        float rotationAngle = 0.0f;
        Utils::Point translation;
        
        Utils::Point apply(const Utils::Point& point) const;
        Utils::Rectangle apply(const Utils::Rectangle& rect) const;
    } transform;
};

// Animation support
struct AnimationKeyframe {
    float time; // 0-1
    Utils::Point position;
    float opacity;
    float scale;
    float rotation;
};

class Animation {
public:
    Animation(float duration) : m_duration(duration) {}
    
    void addKeyframe(const AnimationKeyframe& keyframe);
    AnimationKeyframe interpolate(float time) const;
    bool isFinished(float currentTime) const;
    
private:
    float m_duration;
    std::vector<AnimationKeyframe> m_keyframes;
};

// Main ScreenWriter class
class ScreenWriter {
public:
    ScreenWriter();
    ~ScreenWriter();
    
    // Initialization
    bool initialize();
    void shutdown();
    bool isInitialized() const { return m_initialized; }
    
    // Canvas management
    bool createCanvas(const Utils::Size& size);
    bool resizeCanvas(const Utils::Size& newSize);
    void clearCanvas(const Color& backgroundColor = Color::TRANSPARENT);
    
    // Layer management for advanced compositing
    int createLayer(const std::string& name = "");
    bool removeLayer(int layerId);
    bool setActiveLayer(int layerId);
    bool setLayerOpacity(int layerId, float opacity);
    bool setLayerVisible(int layerId, bool visible);
    std::vector<int> getLayerIds() const;
    
    // Drawing context
    void pushContext();
    void popContext();
    void setClipRect(const Utils::Rectangle& rect);
    void clearClipRect();
    void setGlobalOpacity(float opacity);
    void setTransform(const DrawingContext::Transform& transform);
    void resetTransform();
    
    // Text rendering (advanced)
    bool drawText(const std::string& text, const Utils::Point& position,
                  const TextProperties& properties = TextProperties());
    bool drawTextInRect(const std::string& text, const Utils::Rectangle& bounds,
                        const TextProperties& properties = TextProperties());
    bool drawFormattedText(const std::string& htmlText, const Utils::Rectangle& bounds,
                           const TextProperties& baseProperties = TextProperties());
    
    // Shape drawing (advanced)
    bool drawShape(ShapeType type, const std::vector<Utils::Point>& points,
                   const ShapeProperties& properties = ShapeProperties());
    bool drawRectangle(const Utils::Rectangle& rect,
                       const ShapeProperties& properties = ShapeProperties());
    bool drawRoundedRectangle(const Utils::Rectangle& rect, float radius,
                              const ShapeProperties& properties = ShapeProperties());
    bool drawCircle(const Utils::Point& center, float radius,
                    const ShapeProperties& properties = ShapeProperties());
    bool drawEllipse(const Utils::Point& center, float radiusX, float radiusY,
                     const ShapeProperties& properties = ShapeProperties());
    bool drawLine(const Utils::Point& start, const Utils::Point& end,
                  const ShapeProperties& properties = ShapeProperties());
    bool drawPolyline(const std::vector<Utils::Point>& points,
                      const ShapeProperties& properties = ShapeProperties());
    bool drawPolygon(const std::vector<Utils::Point>& points,
                     const ShapeProperties& properties = ShapeProperties());
    bool drawArrow(const Utils::Point& start, const Utils::Point& end,
                   float headSize = 10.0f,
                   const ShapeProperties& properties = ShapeProperties());
    bool drawBezierCurve(const Utils::Point& start, const Utils::Point& control1,
                         const Utils::Point& control2, const Utils::Point& end,
                         const ShapeProperties& properties = ShapeProperties());
    
    // Advanced drawing operations
    bool drawImage(const std::string& imagePath, const Utils::Point& position,
                   const Utils::Size& size = Utils::Size(), float opacity = 1.0f);
    bool drawPattern(const Utils::Rectangle& area, const std::string& patternType);
    bool applyEffect(const Utils::Rectangle& area, const std::string& effectType,
                     const std::map<std::string, float>& parameters = {});
    
    // Animation support
    void startAnimation(int objectId, const Animation& animation);
    void stopAnimation(int objectId);
    void updateAnimations(float deltaTime);
    
    // Screen overlay operations
    bool enableOverlay();
    bool disableOverlay();
    bool isOverlayEnabled() const { return m_overlayEnabled; }
    bool showOverlay();
    bool hideOverlay();
    
    // Real-time drawing on screen
    bool startRealTimeDrawing();
    bool stopRealTimeDrawing();
    bool isRealTimeDrawing() const { return m_realTimeDrawing; }
    
    // Annotation management
    struct Annotation {
        int id;
        std::string type;
        std::vector<Utils::Point> points;
        TextProperties textProps;
        ShapeProperties shapeProps;
        std::string text;
        std::chrono::steady_clock::time_point timestamp;
        bool visible = true;
    };
    
    int addAnnotation(const Annotation& annotation);
    bool removeAnnotation(int annotationId);
    bool updateAnnotation(int annotationId, const Annotation& newAnnotation);
    std::vector<Annotation> getAnnotations() const;
    void clearAnnotations();
    void undoLastAnnotation();
    void redoAnnotation();
    
    // Export operations
    bool exportToImage(const std::string& filePath, const std::string& format = "PNG");
    bool exportToVideo(const std::string& filePath, float duration = 5.0f, int fps = 30);
    bool exportAnnotations(const std::string& filePath, const std::string& format = "JSON");
    bool importAnnotations(const std::string& filePath);
    
    // Event callbacks
    using DrawingCallback = std::function<void(const Utils::Point&, const std::string&)>;
    using AnnotationCallback = std::function<void(const Annotation&)>;
    
    void setDrawingCallback(DrawingCallback callback) { m_drawingCallback = callback; }
    void setAnnotationCallback(AnnotationCallback callback) { m_annotationCallback = callback; }
    
    // Configuration
    void setDefaultTextProperties(const TextProperties& properties);
    void setDefaultShapeProperties(const ShapeProperties& properties);
    TextProperties getDefaultTextProperties() const { return m_defaultTextProps; }
    ShapeProperties getDefaultShapeProperties() const { return m_defaultShapeProps; }
    
    // Performance and quality settings
    void setRenderQuality(int quality); // 0-100
    void setAntiAliasing(bool enabled);
    void setHardwareAcceleration(bool enabled);
    
    // Debug and diagnostics
    struct RenderStats {
        int objectsRendered = 0;
        float renderTime = 0.0f;
        int memoryUsage = 0;
        int layerCount = 0;
    };
    
    RenderStats getRenderStats() const;
    void resetRenderStats();
    
private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
    
    bool m_initialized;
    bool m_overlayEnabled;
    bool m_realTimeDrawing;
    
    TextProperties m_defaultTextProps;
    ShapeProperties m_defaultShapeProps;
    
    DrawingCallback m_drawingCallback;
    AnnotationCallback m_annotationCallback;
    
    std::vector<DrawingContext> m_contextStack;
    DrawingContext m_currentContext;
    
    // Internal methods
    void notifyDrawing(const Utils::Point& position, const std::string& action);
    void notifyAnnotation(const Annotation& annotation);
    bool validateContext() const;
    void updateRenderStats();
};

}} // namespace Recordify::ScreenHandler

#endif // RECORDIFY_SCREEN_WRITER_H
