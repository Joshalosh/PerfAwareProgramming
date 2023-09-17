
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdint.h>

typedef uint8_t u8;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int32_t b32;

typedef float f32;
typedef double f64;

#define ArrayCount(Array) (sizeof(Array)/sizeof((Array)[0]))

#include "platform_metrics.cpp"

int main(int arg_count, char **args) {
    InitialiseOSMetrics();

    if (arg_count == 2) {
        u64 page_size  = 4096;
        u64 page_count = atol(args[1]);
        u64 total_size = page_size*page_count;

        printf("Page Count, Touch Count, Fault Count, Extra Faults\n");

        for (u64 touch_count = 0; touch_count <= page_count; touch_count++) {
            u64 touch_size = page_size*touch_count;
            u8 *data = (u8 *)VirtualAlloc(0, total_size, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
            if (data) {
                u64 start_fault_count = ReadOSPageFaultCount();
                for (u64 index = 0; index < touch_size; index++) {
                    data[index] = (u8)index;
                }

                u64 end_fault_count = ReadOSPageFaultCount();

                u64 fault_count = end_fault_count - start_fault_count;

                printf("%llu, %llu, %llu, %lld\n", page_count, touch_count, fault_count, (fault_count - touch_count));

                VirtualFree(data, 0, MEM_RELEASE);
            } else {
                fprintf(stderr, "ERROR: Unable to allocate memory\n");
            }
        }
    } else {
        fprintf(stderr, "Usage: %s [# of 4k pages to allocate]\n", args[0]);
    }
}
