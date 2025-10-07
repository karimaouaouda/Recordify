#ifndef RECORDIFY_UTILS_GEOMETRY_H
#define RECORDIFY_UTILS_GEOMETRY_H

#include <algorithm>
#include <cmath>

namespace Recordify {
namespace Utils {

struct Point {
    int x, y;
    
    Point(int x = 0, int y = 0) : x(x), y(y) {}
    
    Point operator+(const Point& other) const {
        return Point(x + other.x, y + other.y);
    }
    
    Point operator-(const Point& other) const {
        return Point(x - other.x, y - other.y);
    }
    
    Point operator*(float scale) const {
        return Point(static_cast<int>(x * scale), static_cast<int>(y * scale));
    }
    
    bool operator==(const Point& other) const {
        return x == other.x && y == other.y;
    }
    
    bool operator!=(const Point& other) const {
        return !(*this == other);
    }
    
    float distanceTo(const Point& other) const {
        float dx = static_cast<float>(x - other.x);
        float dy = static_cast<float>(y - other.y);
        return std::sqrt(dx * dx + dy * dy);
    }
    
    Point midpoint(const Point& other) const {
        return Point((x + other.x) / 2, (y + other.y) / 2);
    }
};

struct Size {
    int width, height;
    
    Size(int w = 0, int h = 0) : width(w), height(h) {}
    
    bool isEmpty() const {
        return width <= 0 || height <= 0;
    }
    
    bool isValid() const {
        return width >= 0 && height >= 0;
    }
    
    int area() const {
        return width * height;
    }
    
    Size operator+(const Size& other) const {
        return Size(width + other.width, height + other.height);
    }
    
    Size operator-(const Size& other) const {
        return Size(width - other.width, height - other.height);
    }
    
    Size operator*(float scale) const {
        return Size(static_cast<int>(width * scale), static_cast<int>(height * scale));
    }
    
    bool operator==(const Size& other) const {
        return width == other.width && height == other.height;
    }
    
    float aspectRatio() const {
        return height != 0 ? static_cast<float>(width) / height : 0.0f;
    }
};

struct Rectangle {
    int x, y, width, height;
    
    Rectangle(int x = 0, int y = 0, int w = 0, int h = 0) 
        : x(x), y(y), width(w), height(h) {}
    
    Rectangle(const Point& topLeft, const Size& size)
        : x(topLeft.x), y(topLeft.y), width(size.width), height(size.height) {}
    
    Rectangle(const Point& topLeft, const Point& bottomRight)
        : x(topLeft.x), y(topLeft.y), width(bottomRight.x - topLeft.x), height(bottomRight.y - topLeft.y) {}
    
    // Corner and center points
    Point topLeft() const { return Point(x, y); }
    Point topRight() const { return Point(x + width, y); }
    Point bottomLeft() const { return Point(x, y + height); }
    Point bottomRight() const { return Point(x + width, y + height); }
    Point center() const { return Point(x + width/2, y + height/2); }
    
    // Size and boundaries
    Size size() const { return Size(width, height); }
    int left() const { return x; }
    int right() const { return x + width; }
    int top() const { return y; }
    int bottom() const { return y + height; }
    
    // Validation
    bool isEmpty() const {
        return width <= 0 || height <= 0;
    }
    
    bool isValid() const {
        return width >= 0 && height >= 0;
    }
    
    int area() const {
        return width * height;
    }
    
    // Point containment
    bool contains(const Point& point) const {
        return point.x >= x && point.x < x + width &&
               point.y >= y && point.y < y + height;
    }
    
    bool contains(const Rectangle& other) const {
        return other.x >= x && other.y >= y &&
               other.x + other.width <= x + width &&
               other.y + other.height <= y + height;
    }
    
    // Intersection and overlap
    bool intersects(const Rectangle& other) const {
        return !(x >= other.x + other.width || other.x >= x + width ||
                 y >= other.y + other.height || other.y >= y + height);
    }
    
    Rectangle intersection(const Rectangle& other) const {
        int left = std::max(x, other.x);
        int top = std::max(y, other.y);
        int right = std::min(x + width, other.x + other.width);
        int bottom = std::min(y + height, other.y + other.height);
        
        if (left < right && top < bottom) {
            return Rectangle(left, top, right - left, bottom - top);
        }
        return Rectangle(); // Empty rectangle
    }
    
    Rectangle united(const Rectangle& other) const {
        if (isEmpty()) return other;
        if (other.isEmpty()) return *this;
        
        int left = std::min(x, other.x);
        int top = std::min(y, other.y);
        int right = std::max(x + width, other.x + other.width);
        int bottom = std::max(y + height, other.y + other.height);
        
        return Rectangle(left, top, right - left, bottom - top);
    }
    
    // Transformations
    Rectangle translated(const Point& offset) const {
        return Rectangle(x + offset.x, y + offset.y, width, height);
    }
    
    Rectangle scaled(float scale) const {
        return Rectangle(
            static_cast<int>(x * scale),
            static_cast<int>(y * scale),
            static_cast<int>(width * scale),
            static_cast<int>(height * scale)
        );
    }
    
    Rectangle expanded(int margin) const {
        return Rectangle(x - margin, y - margin, width + 2 * margin, height + 2 * margin);
    }
    
    Rectangle adjusted(int dx1, int dy1, int dx2, int dy2) const {
        return Rectangle(x + dx1, y + dy1, width + dx2 - dx1, height + dy2 - dy1);
    }
    
    // Operators
    bool operator==(const Rectangle& other) const {
        return x == other.x && y == other.y && width == other.width && height == other.height;
    }
    
    bool operator!=(const Rectangle& other) const {
        return !(*this == other);
    }
    
    Rectangle operator+(const Point& offset) const {
        return translated(offset);
    }
    
    Rectangle operator-(const Point& offset) const {
        return translated(Point(-offset.x, -offset.y));
    }
    
    // Utility methods
    float distanceToPoint(const Point& point) const {
        if (contains(point)) return 0.0f;
        
        int dx = std::max({x - point.x, 0, point.x - (x + width)});
        int dy = std::max({y - point.y, 0, point.y - (y + height)});
        
        return std::sqrt(static_cast<float>(dx * dx + dy * dy));
    }
    
    Point closestPointTo(const Point& point) const {
        return Point(
            std::clamp(point.x, x, x + width),
            std::clamp(point.y, y, y + height)
        );
    }
    
    // Aspect ratio maintenance
    Rectangle fitInside(const Rectangle& container, bool maintainAspectRatio = true) const {
        if (!maintainAspectRatio) {
            return container;
        }
        
        float scaleX = static_cast<float>(container.width) / width;
        float scaleY = static_cast<float>(container.height) / height;
        float scale = std::min(scaleX, scaleY);
        
        int newWidth = static_cast<int>(width * scale);
        int newHeight = static_cast<int>(height * scale);
        
        int offsetX = (container.width - newWidth) / 2;
        int offsetY = (container.height - newHeight) / 2;
        
        return Rectangle(container.x + offsetX, container.y + offsetY, newWidth, newHeight);
    }
    
    // Subdivision
    std::vector<Rectangle> subdivide(int rows, int cols) const {
        std::vector<Rectangle> rects;
        
        if (rows <= 0 || cols <= 0) return rects;
        
        int cellWidth = width / cols;
        int cellHeight = height / rows;
        
        for (int row = 0; row < rows; ++row) {
            for (int col = 0; col < cols; ++col) {
                int cellX = x + col * cellWidth;
                int cellY = y + row * cellHeight;
                
                // Adjust last column/row to account for rounding
                int actualWidth = (col == cols - 1) ? width - col * cellWidth : cellWidth;
                int actualHeight = (row == rows - 1) ? height - row * cellHeight : cellHeight;
                
                rects.emplace_back(cellX, cellY, actualWidth, actualHeight);
            }
        }
        
        return rects;
    }
};

// Geometric calculations
namespace Geometry {
    
    // Distance calculations
    inline float distance(const Point& p1, const Point& p2) {
        return p1.distanceTo(p2);
    }
    
    inline float distanceSquared(const Point& p1, const Point& p2) {
        float dx = static_cast<float>(p1.x - p2.x);
        float dy = static_cast<float>(p1.y - p2.y);
        return dx * dx + dy * dy;
    }
    
    // Angle calculations (in radians)
    inline float angle(const Point& from, const Point& to) {
        return std::atan2(static_cast<float>(to.y - from.y), static_cast<float>(to.x - from.x));
    }
    
    inline float angleDegrees(const Point& from, const Point& to) {
        return angle(from, to) * 180.0f / static_cast<float>(M_PI);
    }
    
    // Line operations
    inline Point lineIntersection(const Point& p1, const Point& p2, const Point& p3, const Point& p4) {
        float denom = static_cast<float>((p1.x - p2.x) * (p3.y - p4.y) - (p1.y - p2.y) * (p3.x - p4.x));
        
        if (std::abs(denom) < 1e-6f) {
            return Point(); // Lines are parallel
        }
        
        float t = static_cast<float>(((p1.x - p3.x) * (p3.y - p4.y) - (p1.y - p3.y) * (p3.x - p4.x))) / denom;
        
        return Point(
            static_cast<int>(p1.x + t * (p2.x - p1.x)),
            static_cast<int>(p1.y + t * (p2.y - p1.y))
        );
    }
    
    inline float distanceToLine(const Point& point, const Point& lineStart, const Point& lineEnd) {
        float A = static_cast<float>(point.x - lineStart.x);
        float B = static_cast<float>(point.y - lineStart.y);
        float C = static_cast<float>(lineEnd.x - lineStart.x);
        float D = static_cast<float>(lineEnd.y - lineStart.y);
        
        float dot = A * C + B * D;
        float lenSq = C * C + D * D;
        
        if (lenSq < 1e-6f) {
            return distance(point, lineStart);
        }
        
        float param = dot / lenSq;
        
        Point closest;
        if (param < 0) {
            closest = lineStart;
        } else if (param > 1) {
            closest = lineEnd;
        } else {
            closest = Point(
                static_cast<int>(lineStart.x + param * C),
                static_cast<int>(lineStart.y + param * D)
            );
        }
        
        return distance(point, closest);
    }
    
    // Polygon operations
    inline bool pointInPolygon(const Point& point, const std::vector<Point>& polygon) {
        if (polygon.size() < 3) return false;
        
        bool inside = false;
        int j = static_cast<int>(polygon.size()) - 1;
        
        for (int i = 0; i < static_cast<int>(polygon.size()); i++) {
            if (((polygon[i].y > point.y) != (polygon[j].y > point.y)) &&
                (point.x < (polygon[j].x - polygon[i].x) * (point.y - polygon[i].y) / (polygon[j].y - polygon[i].y) + polygon[i].x)) {
                inside = !inside;
            }
            j = i;
        }
        
        return inside;
    }
    
    // Bounding rectangle of points
    inline Rectangle boundingRect(const std::vector<Point>& points) {
        if (points.empty()) return Rectangle();
        
        int minX = points[0].x, maxX = points[0].x;
        int minY = points[0].y, maxY = points[0].y;
        
        for (const auto& point : points) {
            minX = std::min(minX, point.x);
            maxX = std::max(maxX, point.x);
            minY = std::min(minY, point.y);
            maxY = std::max(maxY, point.y);
        }
        
        return Rectangle(minX, minY, maxX - minX, maxY - minY);
    }
    
} // namespace Geometry

}} // namespace Recordify::Utils

#endif // RECORDIFY_UTILS_GEOMETRY_H
