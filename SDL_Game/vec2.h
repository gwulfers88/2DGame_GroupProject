#if !defined(VEC2_H)
/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: George Wulfers $
   $Notice: (C) Copyright 2016 by 8Box Studios, LLC. All Rights Reserved. $
   ======================================================================== */

#include <math.h>

struct vec2
{
    r32 x;
    r32 y;
};

vec2 Vec2(r32 x, r32 y)
{
    vec2 result = {x, y};
    return result;
}

vec2 operator* (r32 a, vec2 b)
{
    vec2 result;
    result.x = a * b.x;
    result.y = a * b.y;
    return result;
}

vec2 operator* (vec2 a, r32 b)
{
    vec2 result = b * a;
    return result;
}

vec2& operator *= (vec2& a, r32 b)
{
    a = a * b;
    return a;
}

vec2 operator-(vec2 a)
{
    vec2 result;
    result.x = -a.x;
    result.y = -a.y;
    return result;
}

vec2 operator+(vec2 a, vec2 b)
{
    vec2 result;
    result.x = a.x + b.x;
    result.y = a.y + b.y;
    return result;
}

vec2& operator+=(vec2& a, vec2 b)
{
    a = a + b;
    return a;
}

vec2 operator-(vec2 a, vec2 b)
{
    vec2 result;
    result.x = a.x - b.x;
    result.y = a.y - b.y;
    return result;
}

r32 Square(r32 a)
{
    r32 result = a * a;
    return result;
}

// NOTE: DOT PRODUCT
r32 Dot(vec2 a, vec2 b)
{
    r32 result = a.x * b.x + a.y * b.y;
    return result;
}

// NOTE: Magnitude
r32 Magnitude(vec2 a)
{
    r32 result = sqrt(Dot(a, a));
    return result;
}

// NOTE: MagnitudeSquared
r32 Magnitude2(vec2 a)
{
    r32 result = Dot(a, a);
    return result;
}

#define VEC2_H
#endif
