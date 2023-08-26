
enum Test_Mode : u32 {
    TestMode_Uninitialised,
    TestMode_Testing,
    TestMode_Completed,
    TestMode_Error,
};

struct Repetition_Test_Results {
    u64 test_count;
    u64 total_time;
    u64 max_time;
    u64 min_time;
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
    u64 time_accumulated_on_this_test;
    u64 bytes_accumulated_on_this_test;

    Repetition_Test_Results results;
};

static f64 SecondsFromCPUTime(f64 cpu_time, u64 cpu_timer_freq) {
    f64 result = 0.0;
    if (cpu_timer_freq) {
        result = (cpu_time / (f64)cpu_timer_freq);
    }

    return result;
}

static void PrintTime(char const *label, f64 cpu_time, u64 cpu_timer_freq, u64 byte_count) {
    printf("%s: %.0f", label, cpu_time);
    if (cpu_timer_freq) {
        f64 seconds = SecondsFromCPUTime(cpu_time, cpu_timer_freq);
        printf(" (%fms)", 1000.0f*seconds);

        if (byte_count) {
            f64 best_bandwidth = byte_count / (GIGABYTE * seconds);
            printf(" %fgb/s", best_bandwidth);
        }
    }
}

static void PrintTime(char const *label, u64 cpu_time, u64 cpu_timer_freq, u64 byte_count) {
    PrintTime(label, (f64)cpu_time, cpu_timer_freq, byte_count);
}

static void PrintResults(Repetition_Test_Results results, u64 cpu_timer_freq, u64 byte_count) {
    PrintTime("Min", (f64)results.min_time, cpu_timer_freq, byte_count);
    printf("\n");

    if (results.test_count) {
        PrintTime("Avg", (f64)results.total_time / (f64)results.test_count, cpu_timer_freq, byte_count);
        printf("\n");
    }
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
        tester->print_new_minimums = true;
        tester->results.min_time = (u64)-1;
    } else if (tester->mode == TestMode_Completed) {
        tester->mode = TestMode_Testing;

        if(tester->target_processed_byte_count != target_processed_byte_count) {
            Error(tester, "target_processed_byte_count changed");
        }

        if (tester->cpu_timer_freq != cpu_timer_freq) {
            Error(tester, "CPU frequence changed");
        }
    }

    tester->try_for_time = seconds_to_try*cpu_timer_freq;
    tester->tests_started_at = ReadCPUTimer();
}

static void BeginTime(Repetition_Tester *tester) {
    ++tester->open_block_count;
    tester->time_accumulated_on_this_test -= ReadCPUTimer();
}

static void EndTime(Repetition_Tester *tester) {
    ++tester->close_block_count;
    tester->time_accumulated_on_this_test += ReadCPUTimer();
}

static void CountBytes(Repetition_Tester *tester, u64 byte_count) {
    tester->bytes_accumulated_on_this_test += byte_count;
}

static b32 IsTesting(Repetition_Tester *tester) {
    if (tester->mode == TestMode_Testing) {
        u64 current_time = ReadCPUTimer();

        // We don't count tests that had no timing blocks
        // we assume they took some other path
        if (tester->open_block_count) {
            if (tester->open_block_count != tester->close_block_count) {
                Error(tester, "Unbalanced BeginTime/EndTime");
            }

            if (tester->bytes_accumulated_on_this_test != tester->target_processed_byte_count) {
                Error(tester, "Processed byte count mismatch");
            }

            if(tester->mode == TestMode_Testing) {
                Repetition_Test_Results *results = &tester->results;
                u64 elapsed_time = tester->time_accumulated_on_this_test;
                results->test_count += 1;
                results->total_time += elapsed_time;
                if (results->max_time < elapsed_time) {
                    results->max_time = elapsed_time;
                }

                if (results->min_time > elapsed_time) {
                    results->min_time = elapsed_time;

                    // Whenever we get a new minimum time, we reset the clock to the full trial time
                    tester->tests_started_at = current_time;

                    if (tester->print_new_minimums) {
                        PrintTime("Min", results->min_time, tester->cpu_timer_freq, tester->bytes_accumulated_on_this_test);
                        printf("               \r");
                    }
                }

                tester->open_block_count = 0;
                tester->close_block_count = 0;
                tester->time_accumulated_on_this_test = 0;
                tester->bytes_accumulated_on_this_test = 0;
            }
        }

        if ((current_time - tester->tests_started_at) > tester->try_for_time)
        {
            tester->mode = TestMode_Completed;

            printf("                                                                 \r");
            PrintResults(tester->results, tester->cpu_timer_freq, tester->target_processed_byte_count);
        }
    }

    b32 result = (tester->mode == TestMode_Testing);

    return result;
}
