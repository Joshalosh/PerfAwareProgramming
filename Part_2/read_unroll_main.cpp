
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

typedef void ASMFunction(u64 count, char *data);

extern "C" void read_x1(u64 count, char *data);
extern "C" void read_x2(u64 count, char *data);
extern "C" void read_x3(u64 count, char *data);
extern "C" void read_x4(u64 count, char *data);
#pragma comment (lib, "read_unroll")


struct Test_Function {
    char const *name;
    ASMFunction *func;
};
Test_Function test_functions[] = {
    {"read_x1", read_x1},
    {"read_x2", read_x2},
    {"read_x3", read_x3},
    {"read_x4", read_x4},
};

int main(){
    InitialiseOSMetrics();

    u64 repeat_count = 1024*1024*1024ull;
    File_Content buffer = AllocateBuffer(4096);
    if (IsValid(buffer)) {
        Repetition_Tester testers[ArrayCount(test_functions)] = {};

        for (;;) {
            for (u32 func_index = 0; func_index < ArrayCount(test_functions); func_index++) {
                Repetition_Tester *tester = &testers[func_index];
                Test_Function test_func   = test_functions[func_index];

                printf("\n--- %s ---\n", test_func.name);
                NewTestWave(tester, buffer.size, GetCPUTimerFreq());

                while (IsTesting(tester)) {
                    BeginTime(tester);
                    test_func.func(buffer.size, buffer.data);
                    EndTime(tester);
                    CountBytes(tester, buffer.size);
                }
            }
        }
    } else {
       fprintf(stderr, "Unable to allocate memory buffer for testing");
    }
    FreeBuffer(&buffer);
}
