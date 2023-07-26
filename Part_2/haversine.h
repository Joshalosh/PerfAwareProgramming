#if !defined(HAVERSINE_H)

struct File_Content {
    char *data;
    size_t size;
};

enum Token_Type {
    TokenType_None,
    TokenType_String,
    TokenType_Real,
    TokenType_Num,

    TokenType_Count,
};
struct Token {
    Token_Type type;
    union {
        char *string;
        float real_num;
        s32   num;
    };
    Token *next;
    Token *prev;
};


struct Memory_Arena {
    char *start;
    char *current;
    char *end;
};

struct Timings {
    u64 start;
    u64 end;
    u64 elapsed;
};

void StartTimer(Timings *timer) {
    timer->start = ReadCPUTimer();
}

void EndTimer(Timings *timer) {
    timer->end = ReadCPUTimer();
    timer->elapsed = timer->end - timer->start;
}

void InitArena(Memory_Arena *arena, size_t size) {
    arena->start   = (char *)malloc(size);
    arena->current = arena->start;
    arena->end     = arena->current + size;
}

void *ArenaAlloc(Memory_Arena *arena, size_t size) {
    void *result = NULL;
    if (!(arena->current + size > arena->end)) { // Not enough space left in the arena.
        result = arena->current;
        arena->current += size;
    } else {
        printf("Not enough room in arena");
        Assert(1 == 0);
    }
    

    return result;
}

void FreeArena(Memory_Arena *arena) {
    free(arena->start);
}

#define ZeroStruct(instance) ZeroSize(sizeof(instance), &(instance))
#define ZeroArray(count, pointer) ZeroSize(count*sizeof((pointer)[0]), pointer)
void ZeroSize(size_t size, void *ptr) {
    u8 *byte = (u8 *)ptr;
    while(size--) {
        *byte++ = 0;
    }
}

struct Profile_Anchor
{
    u64 tsc_elapsed;
    u64 hit_count;
    char const *label;
};

struct Profiler 
{
    Profile_Anchor anchors[4096];

    u64 start_tsc;
    u64 end_tsc;
};
static Profiler global_profiler;

struct Profile_Block
{
    Profile_Block(char const *label_, u32 anchor_index_)
    {
        anchor_index = anchor_index_;
        label = label_;
        start_tsc = ReadCPUTimer();
    }

    ~Profile_Block()
    {
        u64 elapsed = ReadCPUTimer() - start_tsc;

        Profile_Anchor *anchor = global_profiler.anchors + anchor_index;
        anchor->tsc_elapsed   += elapsed;
        ++anchor->hit_count;

        // This write happens every time solely because there is no 
        // straightforward way in C++ to have the same ease-of-use.
        // In a better programming language, it would be simple to have 
        // the anchor points gathered and labeled at compile time, and 
        // this repetative write would be eliminated.
        anchor->label = label;
    }

    char const *label;
    u64 start_tsc;
    u32 anchor_index;
};

#define NameConcat2(a, b) a##b 
#define NameConcat(a, b) NameConcat2(a, b)
#define TimeBlock(name) Profile_Block NameConcat(block, __LINE__)(name, __COUNTER__ + 1);
#define TimeFunction TimeBlock(__func__)

static void PrintTimeElapsed(u64 total_tsc_elapsed, Profile_Anchor *anchor) {
    u64 elapsed = anchor->tsc_elapsed;
    f64 percent = 100.0 * ((f64)elapsed / (f64)total_tsc_elapsed);
    printf("  %s[%llu]: %llu (%.2f%%)\n", anchor->label, anchor->hit_count, elapsed, percent);
}

static void BeginProfile() {
    global_profiler.start_tsc = ReadCPUTimer();
}

static void EndAndPrintProfile() {
    global_profiler.end_tsc = ReadCPUTimer();
    u64 cpu_freq = GetCPUFreq();

    u64 total_cpu_elapsed = global_profiler.end_tsc - global_profiler.start_tsc;

    if (cpu_freq) {
        printf("\nTotal time: %0.4fms (CPU freq %llu)\n", 
               1000.0 * (f64)total_cpu_elapsed / (f64)cpu_freq, cpu_freq);
    }

    for (u32 anchor_index = 0; anchor_index < ArrayCount(global_profiler.anchors); ++anchor_index)
    {
        Profile_Anchor *anchor = global_profiler.anchors + anchor_index;
        if (anchor->tsc_elapsed) {
            PrintTimeElapsed(total_cpu_elapsed, anchor);
        }
    }
}

#if 0
#define TIMED_BLOCK(BlockName) \
    timed_block TimedBlock_##__LINE__(__rdtsc(), BlockName)

struct timed_block
{
    u64 start_time;
    char *block_name;

    timed_block(u64 start, char *name)
        : start_time(start), block_name(name)
    {
    }

    ~timed_block()
    {
        u64 end_time = __rdtsc();
        u64 elapsed_cycles = end_time - start_time;
        u64 cpu_frequency = GetCPUFreq(start_time, end_time, elapsed_cycles);
        f64 elapsed_seconds = (f64)elapsed_cycles / (f64)cpu_frequency;
        printf("Block '%s' took %f seconds\n", block_name, elapsed_seconds);
    }
};
#endif

#define HAVERSINE_H
#endif
