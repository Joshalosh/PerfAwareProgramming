#if !defined(HARVERSINE_MATH_H)

static f32 Square(f64 a)
{
    f32 result = (a*a);
    return result;
}

static f32 DegreesToRadians(f32 degrees)
{
    // To convert from degrees to radians you need to multiply
    // the degrees by PI/180
    f64 result = 0.01745329251994329577f * degrees;

    return (f32)result;
}

#define HARVERSINE_MATH_H
#endif
