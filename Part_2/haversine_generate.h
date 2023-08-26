#if !defined(HARVERSINE_GENERATE_H)
#include <stdint.h>

typedef float f32;
typedef double f64;

typedef int32_t b32;

typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

#define EARTH_RADIUS 6372.8f

#define CUSTOM_MEMORY 1
#define PROFILER 1

#define MEGABYTE 1024.f * 1024.f
#define GIGABYTE MEGABYTE * 1024.f

#if 0
#define READ_BLOCK_TIMER ReadOSTimer
#endif

#define ArrayCount(array) (sizeof(array) / sizeof((array)[0]))
#define Assert(expression) if(!(expression)) {*(int *)0 = 0;}

#define HARVERSINE_GENERATE_H
#endif
