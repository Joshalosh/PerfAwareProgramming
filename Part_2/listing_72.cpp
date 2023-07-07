
#include <stdint.h>
#include <stdio.h>

#include "haversine_generate.h"
#include "listing_70.cpp"

int main() {
    u64 os_freq = GetOSTimerFreq();
    printf("     OS Freq: %llu\n", os_freq);

    u64 cpu_start  = ReadCPUTimer();
    u64 os_start   = ReadOSTimer();
    u64 os_end     = 0;
    u64 os_elapsed = 0;
    while (os_elapsed < os_freq) {
        os_end     = ReadOSTimer();
        os_elapsed = os_end - os_start;
    }

    u64 cpu_end = ReadCPUTimer();
    u64 cpu_elapsed = cpu_end - cpu_start;

    printf("    OS Timer: %llu -> %llu = %llu elapsed\n", os_start, os_end, os_elapsed);
    printf("  OS Seconds: %.4f\n", (f64)os_elapsed/(f64)os_freq);

    printf("   CPU Timer: %llu -> %llu = %llu elapsed\n", cpu_start, cpu_end, cpu_elapsed);
}
