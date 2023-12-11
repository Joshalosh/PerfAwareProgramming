
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
//#include "pagefault_overhead_test.cpp"

typedef void ASMFunction(u64 count, char *data);

extern "C" void ConditionalNOP(u64 count, char *data);
#pragma comment (lib, "conditional_nop")

struct Test_Function {
    char const *name;
    ASMFunction *func;
};
Test_Function test_functions[] = {
    {"ConditionalNOP", ConditionalNOP},
};

enum Branch_Pattern {
    BranchPattern_NeverTaken,
    BranchPattern_AlwaysTaken,
    BranchPattern_Every2,
    BranchPattern_Every3,
    BranchPattern_Every4,
    BranchPattern_CRTRandom,
    BranchPattern_OSRandom,

    BranchPattern_Count,
};

static char const *FillWithBranchPattern(Branch_Pattern pattern, File_Content buffer) {
    char const *pattern_name = "UNKNOWN";

    if (pattern == BranchPattern_OSRandom) {
        pattern_name = "OSRandom";
        FillWithRandomBytes(buffer);
    } else {
        for (int index = 0; index < buffer.size; index++) {
            u8 value = 0;

            switch (pattern) {
                case BranchPattern_NeverTaken: {
                    pattern_name = "Never Taken";
                    value = 0;
                } break;

                case BranchPattern_AlwaysTaken: {
                    pattern_name = "Always Taken";
                    value = 1;
                } break;

                case BranchPattern_Every2: {
                    pattern_name = "Every 2";
                    value = ((index % 2) == 0);
                } break;

                case BranchPattern_Every3: {
                    pattern_name = "Every 3";
                    value = ((index % 3) == 0);
                } break;

                case BranchPattern_Every4: {
                    pattern_name = "Every 4";
                    value = ((index % 4) == 0);
                } break;

                case BranchPattern_CRTRandom: {
                    pattern_name = "CRTRandom";
                    // NOTE: rand() actually isn't really all that random 
                    // in the future we will look at better ways to get entropy for
                    // testing purposes
                    value = (u8)rand();
                } break;

                default: {
                    fprintf(stderr, "Unrecognised branch pattern. \n");
                } break;
            }

            buffer.data[index] = value;
        }
    }

    return pattern_name;
}

int main() {
    InitialiseOSMetrics();

    File_Content buffer = AllocateBuffer(1*1024*1024*1024);

    if (isValid(buffer)) {
        Repetition_Tester testers[BranchPattern_Count][ArrayCount(test_functions)] = {};
        for (;;) {
            for (u32 pattern = 0; pattern < BranchPattern_Count; pattern++) {
                char const *pattern_name = FillWithBranchPattern((branch_pattern)pattern, buffer);

                for (u32 func_index = 0; func_index < ArrayCount(test_functions); func_index++) {
                    Repetition_Tester *tester = &testers[pattern][func_index];
                    Test_Function test_func   = test_functions[func_index];

                    printf("\n--- %s, %s ---\n", test_func.name, pattern_name);
                    NewTestWave(tester, params.dest.size, GetCPUTimerFreq());

                    while (IsTesting(tester)) {
                        BeginTime(tester);
                        test_func.func(buffer.size, buffer.data);
                        EndTime(tester);
                        CountBytes(tester, buffer.size);
                    }
                }
            }
        }
    } else {
        fprintf(stderr, "Unable to allocate memory buffer for testing");
    }

    FreeBuffer(&buffer);
}
