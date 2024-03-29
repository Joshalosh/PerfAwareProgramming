#if !defined(PROFILER_H)

#include "platform_metrics.cpp"

#ifndef PROFILER
#define PROFILER 0
#endif

#ifndef READ_BLOCK_TIMER
#define READ_BLOCK_TIMER ReadCPUTimer
#endif

#if PROFILER

struct Profile_Anchor
{
    u64 tsc_elapsed_exclusive; // Does not include children
    u64 tsc_elapsed_inclusive; // Includes children
    u64 hit_count;
    u64 processed_byte_count;
    char const *label;
};
static Profile_Anchor global_profiler_anchors[4096];
static u32 global_profiler_parent;

struct Profile_Block
{
    Profile_Block(char const *label_, u32 anchor_index_, u64 byte_count)
    {
        parent_index = global_profiler_parent;

        anchor_index = anchor_index_;
        label        = label_;

        Profile_Anchor *anchor        = global_profiler_anchors + anchor_index;
        old_tsc_elapsed_inclusive     = anchor->tsc_elapsed_inclusive;
        anchor->processed_byte_count += byte_count;

        global_profiler_parent = anchor_index;
        start_tsc              = READ_BLOCK_TIMER();
    }

    ~Profile_Block()
    {
        u64 elapsed            = READ_BLOCK_TIMER() - start_tsc;
        global_profiler_parent = parent_index;

        Profile_Anchor *parent = global_profiler_anchors + parent_index;
        Profile_Anchor *anchor = global_profiler_anchors + anchor_index;

        parent->tsc_elapsed_exclusive -= elapsed;
        anchor->tsc_elapsed_exclusive += elapsed;
        anchor->tsc_elapsed_inclusive  = old_tsc_elapsed_inclusive + elapsed;
        ++anchor->hit_count;

        // This write happens every time solely because there is no 
        // straightforward way in C++ to have the same ease-of-use.
        // In a better programming language, it would be simple to have 
        // the anchor points gathered and labeled at compile time, and 
        // this repetative write would be eliminated.
        anchor->label = label;
    }

    char const *label;
    u64 old_tsc_elapsed_inclusive;
    u64 start_tsc;
    u32 parent_index;
    u32 anchor_index;
};

#define NameConcat2(a, b) a##b 
#define NameConcat(a, b) NameConcat2(a, b)
#define TimeBandwidth(name, byte_count) Profile_Block NameConcat(block, __LINE__)(name, __COUNTER__ + 1, byte_count);
#define ProfilerEndOfCompilationUnit static_assert(__COUNTER__ < ArrayCount(global_profiler_anchors), "Number of profile points exceeds size of Profiler::anchors array")

static void PrintTimeElapsed(u64 total_tsc_elapsed, u64 timer_freq, Profile_Anchor *anchor) {
    f64 percent = 100.0 * ((f64)anchor->tsc_elapsed_exclusive / (f64)total_tsc_elapsed);
    printf("  %s[%llu]: %llu (%.2f%%", anchor->label, anchor->hit_count, anchor->tsc_elapsed_exclusive, percent);
    if (anchor->tsc_elapsed_inclusive != anchor->tsc_elapsed_exclusive) {
        f64 percent_with_children = 100.0 * ((f64)anchor->tsc_elapsed_inclusive / (f64)total_tsc_elapsed);
        printf(", %.2f%% w/children", percent_with_children);
    }
    printf(")");

    if(anchor->processed_byte_count) {
        f64 seconds              = (f64)anchor->tsc_elapsed_inclusive / (f64)timer_freq;
        f64 bytes_per_second     = (f64)anchor->processed_byte_count / seconds;
        f64 megabytes            = (f64)anchor->processed_byte_count / (f64)MEGABYTE;
        f64 gigabytes_per_second = bytes_per_second / GIGABYTE;

        printf("  %.3fmb at %.2fgb/s", megabytes, gigabytes_per_second);
    }

    printf("\n");
}

static void PrintAnchorData(u64 total_cpu_elapsed, u64 timer_freq) {
    for (u32 anchor_index = 0; anchor_index < ArrayCount(global_profiler_anchors); ++anchor_index) {
        Profile_Anchor *anchor = global_profiler_anchors + anchor_index;
        if (anchor->tsc_elapsed_inclusive) {
            PrintTimeElapsed(total_cpu_elapsed, timer_freq, anchor);
        }
    }
}

#else

#define TimeBandwidth(...)
#define PrintAnchorData(...)
#define ProfilerEndOfCompilationUnit

#endif

struct profiler {
    u64 start_tsc;
    u64 end_tsc;
};
static profiler global_profiler;

#define TimeBlock(name) TimeBandwidth(name, 0)
#define TimeFunction TimeBlock(__func__)

static u64 EstimateBlockTimerFreq()
{
    (void)&GetCPUFreq; // This has to be voided here to prevent compilers 
                                 // from warning us that it is not used
    u64 milliseconds_to_wait = 100;
    u64 os_freq              = GetOSTimerFreq();

    u64 block_start  = READ_BLOCK_TIMER();
    u64 os_start     = ReadOSTimer();
    u64 os_end       = 0;
    u64 os_elapsed   = 0;
    u64 os_wait_time = os_freq * milliseconds_to_wait / 1000;
    while (os_elapsed < os_wait_time) {
        os_end =  ReadOSTimer();
        os_elapsed = os_end - os_start;
    }

    u64 block_end = READ_BLOCK_TIMER();
    u64 block_elapsed = block_end - block_start;

    u64 block_freq = 0;
    if (os_elapsed) {
        block_freq = os_freq * block_elapsed / os_elapsed;
    }

    return block_freq;
}

static void BeginProfile() {
    global_profiler.start_tsc = READ_BLOCK_TIMER();
}

static void EndAndPrintProfile() {
    global_profiler.end_tsc = READ_BLOCK_TIMER();
    u64 timer_freq            = EstimateBlockTimerFreq();

    u64 total_tsc_elapsed = global_profiler.end_tsc - global_profiler.start_tsc;

    if (timer_freq) {
        printf("\nTotal time: %0.4fms (timer freq %llu)\n", 
               1000.0 *(f64)total_tsc_elapsed / (f64)timer_freq, timer_freq);
    }

    PrintAnchorData(total_tsc_elapsed, timer_freq);
}

#define PROFILER_H
#endif
