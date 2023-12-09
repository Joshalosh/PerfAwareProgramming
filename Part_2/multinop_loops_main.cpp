
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

extern "C" void NOP3x1AllBytes(u64 Count, char *data);
extern "C" void NOP1x3AllBytes(u64 Count, char *data);
extern "C" void NOP1x9AllBytes(u64 Count, char *data);
#pragma comment (lib, "multinop")

struct Test_Function {
    char const *name;
    ASMFunction *func;
};
Test_Function test_functions[] = {
    {"NOP3x1AllBytes", NOP3x1AllBytes},
    {"NOP1x3AllBytes", NOP1x3AllBytes},
    {"NOP1x9AllBytes", NOP1x9AllBytes},
};

int main(){
    InitialiseOSMetrics();

   File_Content buffer = AllocateBuffer(1*1024*1024*1024); 
   if(IsValid(buffer)) {
       Repetition_Tester testers[ArrayCount(test_functions)] = {};

       for (;;) {
           for (u32 func_index = 0; func_index < ArrayCount(test_functions); func_index++)
           {
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
