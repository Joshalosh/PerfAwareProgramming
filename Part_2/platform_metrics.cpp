#if _WIN32

#include <intrin.h>
#include <windows.h>
#include <psapi.h>

struct OS_Metrics {
    b32 initialised;
    HANDLE process_handle;
};
static OS_Metrics global_metrics;

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

static u64 ReadOSPageFaultCount() {
    PROCESS_MEMORY_COUNTERS_EX memory_counters = {};
    memory_counters.cb = sizeof(memory_counters);
    GetProcessMemoryInfo(global_metrics.process_handle, (PROCESS_MEMORY_COUNTERS *)&memory_counters, sizeof(memory_counters));

    u64 result = memory_counters.PageFaultCount;
    return result;
}

static void InitialiseOSMetrics() {
    if(!global_metrics.initialised) {
        global_metrics.initialised = true;
        global_metrics.process_handle = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, GetCurrentProcessId());
    }
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

static u64 ReadOSPageFaultCount() {
    return 0;
}

static void InitialiseOSMetrics() {
}

#endif

inline u64 ReadCPUTimer() {
    return __rdtsc();
}

static u64 GetCPUFreq() {

    u64 milliseconds_to_wait = 100;
    u64 os_freq      = GetOSTimerFreq();

    u64 cpu_start    = ReadCPUTimer();
    u64 os_start     = ReadOSTimer();
    u64 os_end       = 0;
    u64 os_elapsed   = 0;
    u64 os_wait_time = os_freq * milliseconds_to_wait / 1000;
    while (os_elapsed < os_wait_time) {
        os_end = ReadOSTimer();
        os_elapsed = os_end - os_start;
    }

    u64 cpu_end = ReadCPUTimer();
    u64 cpu_elapsed = cpu_end - cpu_start;

    u64 cpu_freq = 0;
    if (os_elapsed) {
        cpu_freq = os_freq * cpu_elapsed / os_elapsed;
    }

    return cpu_freq;
}
