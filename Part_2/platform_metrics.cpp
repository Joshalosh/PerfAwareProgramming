
static u64 GetCPUFreq();

#if _WIN32

#include <intrin.h>
#include <windows.h>
#include <psapi.h>

#pragma comment (lib, "advapi32.lib")

struct OS_Metrics {
    b32 initialised;
    u64 large_page_size;
    HANDLE process_handle;
    u64 cpu_timer_freq;
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

static u64 GetMaxOSRandomCount() {
    return 0xffffffff;
}

static b32 ReadOSRandomBytes(u64 count, void *dest) {
    b32 result = false;
    if (count < GetMaxOSRandomCount()) {
        result = (BCryptGenRandom(0, (BYTE *)dest, (u32)count, BCRYPT_USE_SYSTEM_PREFERRED_RNG) != 0);
    }

    return result;
}

static u64 TryToEnableLargePages() {
    u64 result = 0;
    
    HANDLE token_handle;
    if (OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &token_handle)) {
        TOKEN_PRIVILEGES privs         = {};
        privs.PrivilegeCount           = 1;
        privs.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
        if (LookupPrivilegeValue(0, SE_LOCK_MEMORY_NAME, &privs.Privileges[0].Luid)) {
            AdjustTokenPrivileges(token_handle, FALSE, &privs, 0, 0, 0);

            if (GetLastError() == ERROR_SUCCESS) {
                result = GetLargePageMinimum();
            }
        }
        CloseHandle(token_handle);
    }

    return result;
}

static void InitialiseOSMetrics() {
    if(!global_metrics.initialised) {
        global_metrics.initialised    = true;
        global_metrics.large_page_size = TryToEnableLargePages();
        global_metrics.process_handle = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, GetCurrentProcessId());
        global_metrics.cpu_timer_freq = GetCPUFreq();
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

inline u64 GetCPUTimerFreq() {
    u64 result = global_metrics.cpu_timer_freq;
    return result;
}

inline u64 GetLargePageSize() {
    u64 result = global_metrics.large_page_size;
    return result;
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

inline void FillWithRandomBytes(File_Content buffer) {
    u64 max_rand_count = GetMaxOSRandomCount();
    u64 at_offset      = 0;
    while (at_offset < dest.size) {
        u64 read_count = dest.size - at_offset;
        if (read_count > max_rand_count) {
            read_count = max_rand_count;
        }

        ReadOSRandomBytes(read_count, dest.data + at_offset);
        at_offset += read_count;
    }
}
