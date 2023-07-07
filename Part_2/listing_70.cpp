#if _WIN32

#include <intrin.h>
#include <windows.h>

static u64 GetOSTimerFreq() {
    LARGE_INTEGER freq;
    QueryPerformanceFrequency(&freq);
    return freq.QuadPart;
}

static u64 ReadOSTimer() {
    LARGE_INTEGER value;
    QueryPerformanceCounter(&value);
    return value.QuadPart;
}

#else 

#include <x86intrin.h>
#include <sys/time.h>

static u64 GetOSTimerFreq() {
    return 1000000;
}

static u64 ReadOsTimer() {
    timeval value;
    gettimeofday(&value, 0);

    u64 result = GetOSTimerFreq()*(u64)value.tv_sec + (u64)value.tv_usec;
    return result;
}

#endif

inline u64 ReadCPUTimer() {
    return __rdtsc();
}
