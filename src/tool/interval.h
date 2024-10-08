#ifndef INTERVAL_H
#define INTERVAL_H

#include "rtweekend.h"

/* 时间间隔 */
class interval
{
public:
    double min, max;

    interval() : min(+infinity), max(-infinity) {} // Default interval is empty

    interval(double min, double max) : min(min), max(max) {}
    interval(const interval &a, const interval &b)
    {
        // Create the interval tightly enclosing the two input intervals.
        min = a.min <= b.min ? a.min : b.min;
        max = a.max >= b.max ? a.max : b.max;
    }
    double size() const
    {
        return max - min;
    }

    bool contains(double x) const
    {
        return min <= x && x <= max;
    }

    bool surrounds(double x) const
    {
        return min < x && x < max;
    }
    double clamp(double x) const
    {
        if (x < min)
            return min;
        if (x > max)
            return max;
        return x;
    }
    interval expand(double delta) const /* 返回扩展后的区间，左右范围各自扩展一半 */
    {
        auto padding = delta / 2;
        return interval(min - padding, max + padding);
    }

    static const interval empty, universe;
};
interval operator+(const interval &ival, double displacement);

interval operator+(double displacement, const interval &ival);

#endif