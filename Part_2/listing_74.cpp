
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "haversine_generate.h"
#include "platform_metrics.cpp"

int main(int arg_count, char **args) {
    u64 milliseconds_to_wait = 1000;

    if (arg_count == 2) {
        milliseconds_to_wait = atol(args[1]);
    }

    u64 os_freq = GetOSTimerFreq();
    printf("     OS Freq: %llu (reported)\n", os_freq);

    u64 cpu_start    = ReadCPUTimer();
    u64 os_start     = ReadOSTimer();
    u64 os_end       = 0;
    u64 os_elapsed   = 0;
    u64 os_wait_time = os_freq * milliseconds_to_wait / 1000;

    while (os_elapsed < os_wait_time) {
        os_end     = ReadOSTimer();
        os_elapsed = os_end - os_start;
    }

    u64 cpu_end     = ReadCPUTimer();
    u64 cpu_elapsed = cpu_end - cpu_start;
    u64 cpu_freq = GetCPUFreq();
    printf("    OS Timer: %llu -> %llu = %llu elapsed\n", os_start, os_end, os_elapsed);
    printf("  OS Seconds: %.4f\n", (f64)os_elapsed/(f64)os_freq);

    printf("   CPU Timer: %llu -> %llu = %llu elapsed\n", cpu_start, cpu_end, cpu_elapsed);
    printf("     CPU Feq: %llu (guessed)\n", cpu_freq);
}

