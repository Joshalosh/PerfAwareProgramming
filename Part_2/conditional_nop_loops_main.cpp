
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
    }
}

