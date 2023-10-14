
#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <stdio.h>
#include <stdint.h>

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int32_t  b32;
typedef float    f32;
typedef double   f64;

#define ArrayCount(Array) (sizeof(array) / sizeof((array)[0]))

#include "pointer_play.cpp"

static void PrintBinaryBits(u64 value, u32 first_bit, u32 bit_count)
{
    for (u32 bit_index = 0; bit_index < bit_count; bit_index++) {
        u64 bit = (value >> ((bit_count - 1 - bit_index) + first_bit)) & 1;
        printf("%c", bit ? '1' : '0');
    }
}

int main()
{
    for (int pointer_index = 0; pointer_index < 16; pointer_index++) {
        void *pointer = (u8 *)VirtualAlloc(0, 1024*1024, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);

        u64 address = (u64)pointer;
        PrintBinaryBits(address, 48,  16);
        printf("|");
        PrintBinaryBits(address, 39,   9);
        printf("|");
        PrintBinaryBits(address, 30,   9);
        printf("|");
        PrintBinaryBits(address, 21,   9);
        printf("|");
        PrintBinaryBits(address, 12,   9);
        printf("|");
        PrintBinaryBits(address,  0,  12);
        printf("\n");

        PrintAsLine(" 4k paging: ", DecomposePointer4K (pointer));
        PrintAsLine("2mb paging: ", DecomposePointer2MB(pointer));
        PrintAsLine("1gb paging: ", DecomposePointer1GB(pointer));

        printf("\n");
    }
}

