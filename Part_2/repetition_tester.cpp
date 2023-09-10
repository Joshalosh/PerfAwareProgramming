
enum Test_Mode : u32 {
    TestMode_Uninitialised,
    TestMode_Testing,
    TestMode_Completed,
    TestMode_Error,
};

enum RepetitionValue_Type {
    RepValue_TestCount,

    RepValue_CPUTimer,
    RepValue_MemPageFaults,
    RepValue_ByteCount,

    RepValue_Count,
};

struct Repetition_Value {
    u64 e[RepValue_Count];
};

struct Repetition_Test_Results {
    Repetition_Value total;
    Repetition_Value min;
    Repetition_Value max;
};

struct Repetition_Tester {
    u64 target_processed_byte_count;
    u64 cpu_timer_freq;
    u64 try_for_time;
    u64 tests_started_at;

    Test_Mode mode;
    b32 print_new_minimums;
    u32 open_block_count;
    u32 close_block_count;

    Repetition_Value accumulated_on_this_test;
    Repetition_Test_Results results;
};

static f64 SecondsFromCPUTime(f64 cpu_time, u64 cpu_timer_freq) {
    f64 result = 0.0;
    if (cpu_timer_freq) {
        result = (cpu_time / (f64)cpu_timer_freq);
    }

    return result;
}

static void PrintValue(char const *label, Repetition_Value value, u64 cpu_timer_freq) {
    u64 test_count = value.e[RepValue_TestCount];
    f64 divisor = test_count ? (f64)test_count : 1;

    f64 e[RepValue_Count];
    for (u32 e_index = 0; e_index < ArrayCount(e); ++e_index) {
        e[e_index] = (f64)value.e[e_index] / divisor;
    }

    printf("%s: %.0f", label, e[RepValue_CPUTimer]);
    if (cpu_timer_freq) {
        f64 seconds = SecondsFromCPUTime(e[RepValue_CPUTimer], cpu_timer_freq);
        printf(" (%fms)", 1000.0f*seconds);

        if (e[RepValue_ByteCount] > 0) {
            f64 gigabyte = (1024.0f * 1024.0f * 1024.0f);
            f64 bandwidth = e[RepValue_ByteCount] / (gigabyte * seconds);
            printf(" %fgb/s", bandwidth);
        }
    }

    if (e[RepValue_MemPageFaults] > 0) {
        printf(" PF: %0.4f (%0.4fk/fault)", e[RepValue_MemPageFaults], e[RepValue_ByteCount] / (e[RepValue_MemPageFaults] * 1024.0));
    }
}

static void PrintResults(Repetition_Test_Results results, u64 cpu_timer_freq) {
    PrintValue("Min", results.min, cpu_timer_freq);
    printf("\n");
    PrintValue("Max", results.max, cpu_timer_freq);
    printf("\n");
    PrintValue("Avg", results.total, cpu_timer_freq);
    printf("\n");
}

static void Error(Repetition_Tester *tester, char const *message) {
    tester->mode = TestMode_Error;
    fprintf(stderr, "ERROR:%s\n", message);
}

static void NewTestWave(Repetition_Tester *tester, u64 target_processed_byte_count, 
                        u64 cpu_timer_freq, u32 seconds_to_try = 10) {
    if (tester->mode == TestMode_Uninitialised) {
        tester->mode = TestMode_Testing;
        tester->target_processed_byte_count = target_processed_byte_count;
        tester->cpu_timer_freq = cpu_timer_freq;
        tester->print_new_minimums = true;
        tester->results.min.e[RepValue_CPUTimer] = (u64)-1;

    } else if (tester->mode == TestMode_Completed) {
        tester->mode = TestMode_Testing;

        if(tester->target_processed_byte_count != target_processed_byte_count) {
            Error(tester, "target_processed_byte_count changed");
        }

        if (tester->cpu_timer_freq != cpu_timer_freq) {
            Error(tester, "CPU frequence changed");
        }
    }

    tester->try_for_time     = seconds_to_try*cpu_timer_freq;
    tester->tests_started_at = ReadCPUTimer();
}

static void BeginTime(Repetition_Tester *tester) {
    ++tester->open_block_count;

    Repetition_Value *accum           = &tester->accumulated_on_this_test;
    accum->e[RepValue_MemPageFaults] -= ReadOSPageFaultCount();
    accum->e[RepValue_CPUTimer]      -= ReadCPUTimer();
}

static void EndTime(Repetition_Tester *tester) {
    Repetition_Value *accum           = &tester->accumulated_on_this_test;
    accum->e[RepValue_CPUTimer]      += ReadCPUTimer();
    accum->e[RepValue_MemPageFaults] += ReadOSPageFaultCount();

    ++tester->close_block_count;
}

static void CountBytes(Repetition_Tester *tester, u64 byte_count) {
    Repetition_Value *accum       = &tester->accumulated_on_this_test;
    accum->e[RepValue_ByteCount] += byte_count;
}

static b32 IsTesting(Repetition_Tester *tester) {
    if (tester->mode == TestMode_Testing) {
        Repetition_Value accum = tester->accumulated_on_this_test;
        u64 current_time        = ReadCPUTimer();

        // We don't count tests that had no timing blocks
        // we assume they took some other path
        if (tester->open_block_count) {
            if (tester->open_block_count != tester->close_block_count) {
                Error(tester, "Unbalanced BeginTime/EndTime");
            }

            if (accum.e[RepValue_ByteCount] != tester->target_processed_byte_count) {
                Error(tester, "Processed byte count mismatch");
            }

            if(tester->mode == TestMode_Testing) {
                Repetition_Test_Results *results = &tester->results;

                accum.e[RepValue_TestCount] = 1;
                for (u32 e_index = 0; e_index < ArrayCount(accum.e); ++e_index) {
                    results->total.e[e_index] += accum.e[e_index];
                }

                if (results->max.e[RepValue_CPUTimer] < accum.e[RepValue_CPUTimer]) {
                    results->max = accum;
                }

                if (results->min.e[RepValue_CPUTimer] > accum.e[RepValue_CPUTimer]) {
                    results->min = accum;

                    // Whenever we get a new minimum time, we reset the clock to the full trial time
                    tester->tests_started_at = current_time;

                    if (tester->print_new_minimums) {
                        PrintValue("Min", results->min, tester->cpu_timer_freq);
                        printf("                                   \r");
                    }
                }

                tester->open_block_count = 0;
                tester->close_block_count = 0;
                tester->accumulated_on_this_test = {};
            }
        }

        if ((current_time - tester->tests_started_at) > tester->try_for_time)
        {
            tester->mode = TestMode_Completed;

            printf("                                                                 \r");
            PrintResults(tester->results, tester->cpu_timer_freq);
        }
    }

    b32 result = (tester->mode == TestMode_Testing);

    return result;
}
