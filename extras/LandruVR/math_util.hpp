#ifndef math_util_h
#define math_util_h

#include "util.hpp"
#include "linalg_util.hpp"
#include <algorithm>

namespace avl
{

    inline float to_radians(float degrees) { return degrees * float(ANVIL_PI) / 180.0f; }
    inline float to_degrees(float radians) { return radians * 180.0f / float(ANVIL_PI); }
    
    template<typename T> 
    T min(const T& x, const T& y) 
    {
        return ((x) < (y) ? (x) : (y));
    }

    template<typename T> 
    T max(const T& x, const T& y) 
    {
        return ((x) > (y) ? (x) : (y));
    }
    
    template <typename T>
    T min(const T& a, const T& b, const T& c)
    {
        return std::min(a, std::min(b, c));
    }
    
    template <typename T>
    T max(const T& a, const T& b, const T& c)
    {
        return std::max(a, std::max(b, c));
    }
    
    template<typename T> 
    T clamp(const T& val, const T& min, const T& max) 
    {
        return std::min(std::max(val, min), max);
    }

    template<typename T> 
    T sign(const T& a, const T& b) 
    {
        return ((b) >= 0.0 ? std::abs(a) : -std::abs(a));
    }

    template<typename T> 
    T sign(const T& a) 
    {
        return (a == 0) ? T(0) : ((a > 0.0) ? T(1) : T(-1));
    }

    inline float normalize(float value, float min, float max)
    {
        return clamp<float>((value - min) / (max - min), 0.0f, 1.0f);
    }

    template <typename T>
    inline bool in_range(T val, T min, T max)
    {
        return (val >= min && val <= max);
    }

    template <typename T>
    inline T remap(T value, T inputMin, T inputMax, T outputMin, T outputMax, bool clamp)
    {
        T outVal = ((value - inputMin) / (inputMax - inputMin) * (outputMax - outputMin) + outputMin);
        if (clamp)
        {
            if (outputMax < outputMin)
            {
                if (outVal < outputMax) outVal = outputMax;
                else if (outVal > outputMin) outVal = outputMin;
            }
            else
            {
                if (outVal > outputMax) outVal = outputMax;
                else if (outVal < outputMin) outVal = outputMin;
            }
        }
        return outVal;
    }
    
    // In radians
    inline float3 spherical(float theta, float phi)
    {
        return safe_normalize(float3(cos(phi) * sin(theta), sin(phi) * sin(theta), cos(theta)));
    }
    
    template <typename T>
    bool quadratic(T a, T b, T c, T & r1, T & r2)
    {
        T q = b * b - 4 * a * c;
        if (q < 0) return false;
        T sq = sqrt(q);
        T d = 1 / (2 * a);
        r1 = (-b + sq) * d;
        r2 = (-b - sq) * d;
        return true;
    }

    inline float damped_spring(float target, float current, float & velocity, float delta, const float spring_constant)
    {
        float currentToTarget = target - current;
        float springForce = currentToTarget * spring_constant;
        float dampingForce = -velocity * 2 * sqrt(spring_constant);
        float force = springForce + dampingForce;
        velocity += force * delta;
        float displacement = velocity * delta;
        return current + displacement;
    }

}

#endif // end math_util_h
