#if !defined(PROFILER_H)

#include "listing_70.cpp"

#ifndef PROFILER
#define PROFILER 0
#endif

#if PROFILER

struct Profile_Anchor
{
    u64 tsc_elapsed_exclusive; // Does not include children
    u64 tsc_elapsed_inclusive; // Includes children
    u64 hit_count;
    char const *label;
};
static profile_anchor global_profiler_anchors[4096];
static u32 global_profiler_parent;

struct Profile_Block
{
    Profile_Block(char const *label_, u32 anchor_index_)
    {
        parent_index = global_profiler_parent;

        anchor_index = anchor_index_;
        label        = label_;

        Profile_Anchor *anchor    = global_profiler_anchors + anchor_index;
        old_tsc_elapsed_inclusive = anchor->tsc_elapsed_inclusive;

        global_profiler_parent = anchor_index;
        start_tsc              = ReadCPUTimer();
    }

    ~Profile_Block()
    {
        u64 elapsed = ReadCPUTimer() - start_tsc;
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
#define TimeBlock(name) Profile_Block NameConcat(block, __LINE__)(name, __COUNTER__ + 1);
#define ProfilerEndOfCompilationUnit static_assert(__COUNTER__ < ArrayCount(global_profiler__anchors), "Number of profile points exceeds size of Profiler::anchors array")

static void PrintTimeElapsed(u64 total_tsc_elapsed, Profile_Anchor *anchor) {
    f64 percent = 100.0 * ((f64)anchor->tsc_elapsed_exclusive / (f64)total_tsc_elapsed);
    printf("  %s[%llu]: %llu (%.2f%%", anchor->label, anchor->hit_count, anchor->tsc_elapsed_exclusive, percent);
    if (anchor->tsc_elapsed_inclusive != anchor->tsc_elapsed_exclusive) {
        f64 percent_with_children = 100.0 * ((f64)anchor->tsc_elapsed_inclusive / (f64)total_tsc_elapsed);
        printf(", %.2f%% w/children", percent_with_children);
    }
    printf(")\n");
}

static void PrintAnchorData(u64 total_cpu_elapsed) {
    for (u32 anchor_index = 0; anchor_index < ArrayCount(global_profiler_anchors); ++anchor_index) {
        profile_anchor *anchor = global_profiler_anchors + anchor_index;
        if (anchor->tsc_elapsed_inclusive) {
            PrintTimeElapsed(total_cpu_elapsed, anchor);
        }
    }
}

#else

#define TimeBlock(...)
#define PrintAnchorData(...)
#define ProfilerEndOfCompilationUnit

#endif

struct profiler {
    u64 start_tsc;
    u64 end_tsc;
};
static profiler global_profiler;

#define TimeFunction TimeBlock(__func__)

static void BeginProfile() {
    global_profiler.start_tsc = ReadCPUTimer();
}

static void EndAndPrintProfile() {
    global_profiler.end_tsc = ReadCPUTimer();
    u64 cpu_freq            = GetCPUFreq();

    u64 total_cpu_elapsed = global_profiler.end_tsc - global_profiler.start_tsc;

    if (cpu_freq) {
        printf("\nTotal time: %0.4fms (CPU freq %llu)\n", 
               1000.0 *(f64)total_cpu_elapsed / (f64)cpu_freq, cpu_freq);
    }

    PrintAnchorData(total_cpu_elapsed);
}

#define PROFILER_H
#endif
