
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/stat.h>

#include "haversine_generate.h"
#include "haversine.h"
#include "platform_metrics.cpp"
#include "repetition_tester.cpp"
#include "read_overhead_test.cpp"

struct Test_Function {
    char const *name;
    read_overhead_test_func *func;
};
Test_Function test_functions[] = {
    {"fread", ReadViaFRead},
    {"_read", ReadViaRead},
    {"ReadFile", ReadViaReadFile},
};

#if 0
static File_Content AllocateBuffer(size_t count) {
    File_Content result = {};
    result.data = (char *)malloc(
}
#endif

int main(int arg_count, char **args) {
    u64 cpu_timer_freq = GetCPUFreq();

    if (arg_count == 2) {
        char *filename = args[1];

        struct __stat64 stat;
        _stat64(filename, &stat);

        Read_Parameters params = {};
        params.dest.data = (char *)malloc(stat.st_size);
        params.filename = filename;

        if (params.dest.size > 0) {
            Repetition_Tester testers[ArrayCount(test_functions)] = {};

            while (true) {
                for (u32 func_index = 0; func_index < ArrayCount(test_functions); ++func_index)
                {
                    Repetition_Tester *tester = testers + func_index;
                    Test_Function test_func = test_functions[func_index];

                    printf("\n--- %s ---\n", test_func.name);
                    NewTestWave(tester, params.dest.size, cpu_timer_freq);
                    test_func.func(tester, &params);
                }
            }

            free(params.dest.data);
        } else {
            fprintf(stderr, "ERROR: Test data size must be non-zero\n");
        }
    } else {
        fprintf(stderr, "Usage: %s [existing filename]\n", args[0]);
    }

    return 0;
}


