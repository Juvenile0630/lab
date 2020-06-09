#pragma once

class Point
{
public:
    Point() { Point(0, 0); }
    Point(int x, int y) { this->x = x; this->y = y; }
    virtual ~Point() {};

    Point& operator=(Point& point) { this->x = point.x; this->y = point.y; return *this; };
    Point& operator+=(Point& point) { this->x += point.x; this->y += point.y; return *this; };
    Point& operator-=(Point& point) { this->x -= point.x; this->y -= point.y; return *this; };
    Point& operator*=(Point& point) { this->x *= point.x; this->y *= point.y; return *this; };
    Point& operator/=(Point& point) { this->x /= point.x; this->y /= point.y; return *this; };

    Point operator+(Point point) { this->x += point.x; this->y += point.y; return *this; };
    Point operator-(Point& point) { this->x -= point.x; this->y -= point.y; return *this; };
    Point operator*(Point& point) { this->x *= point.x; this->y *= point.y; return *this; };
    Point operator/(Point& point) { this->x /= point.x; this->y /= point.y; return *this; };

    bool operator==(Point point) {};

    int getIndex(int width) { return (y * width + x); }

    static Point GetPoint(int x, int y) { Point point(x, y); return point; }

    int x, y;
};

