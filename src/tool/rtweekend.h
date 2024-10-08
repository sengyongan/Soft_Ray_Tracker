#ifndef RTWEEKEND_H
#define RTWEEKEND_H

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <memory>
#include <random>

// C++ Std Usings

using std::make_shared;
using std::shared_ptr;

// Constants

extern const double infinity;

extern const double pi;

// Utility Functions

double degrees_to_radians(double degrees);

double random_double();

double random_double(double min, double max);
// Common Headers
inline int random_int(int min, int max)
{
    // Returns a random integer in [min,max].
    return int(random_double(min, max + 1));
}

#endif