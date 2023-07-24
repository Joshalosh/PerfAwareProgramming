#include <stdio.h>
#include <stdlib.h>

#include "haversine_generate.h"
#include "listing_70.cpp"
#include "haversine.h"

#include "haversine_tokenise.cpp"
#include "haversine_calculate.cpp"

File_Content LoadFile(char* filename) {

    File_Content result = {};
    FILE* file = fopen(filename, "rb");
    if (!file) {
        printf("Failed to open file\n");
    }
    
    if (file) {
        // Get the file size
        fseek(file, 0, SEEK_END);
        s64 file_size = ftell(file);
        // Returning back to the beginning of the file
        // here is potentially a pointless operation if 
        // we're not going to need the contents of the file 
        // anymore.
        fseek(file, 0, SEEK_SET);
        
        // Allocate a buffer to hold the file contents
        char *buffer = (char*)malloc(file_size + 1);
        if (!buffer) {
            printf("Failed to allocate memory\n");
            fclose(file);
        }
        
        if (buffer) {
            // Read the file into the buffer
            size_t read_size = fread(buffer, 1, file_size, file);
            if (read_size != file_size) {
                printf("Failed to read file\n");
                free(buffer);
                fclose(file);
            }
            
            // Null-terminate the buffer
            buffer[file_size] = '\0';

            fclose(file);
            result.data = buffer;
            result.size = file_size;
        }
    }

    return result;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("Need to call exectuable with arguments: -- harversine_parser.exe (char *)<filename.json>\n");
    } else {
        // Set up profiler timing variables.
        u64 os_freq  = GetOSTimerFreq();
        OS_Timer total_time;
        StartTimer(&total_time);

        OS_Timer memory_init_time;
        StartTimer(&memory_init_time);
#if CUSTOM_MEMORY
        Memory_Arena arena;
        InitArena(&arena, 1024ULL*1024*1024*2);
#endif
        EndTimer(&memory_init_time);

        OS_Timer file_time;
        StartTimer(&file_time);
        char *filename = argv[1];

        File_Content loaded_file = LoadFile(filename);
        EndTimer(&file_time);

#if CUSTOM_MEMORY
        Token *sentinel = (Token *)ArenaAlloc(&arena, sizeof(Token));
        ZeroSize(sizeof(*sentinel), sentinel);
#else
        Token *sentinel = (Token *)calloc(1, sizeof(Token));
#endif
        sentinel->next = sentinel;
        sentinel->prev = sentinel;

        OS_Timer token_setup_time;
        StartTimer(&token_setup_time);
        for (int index = 0; index < loaded_file.size; index++) {
            Token *new_token = NULL;
            switch (loaded_file.data[index]) {
                case '"': {
                    IgnoreString(loaded_file, &index);
                } break;

                case '0': case '1': case '2': case '3': case '4':
                case '5': case '6': case '7': case '8': case '9': case '-': {
#if CUSTOM_MEMORY
                    new_token = TokeniseNumber(&arena, loaded_file, &index);
#else 
                    new_token = TokeniseNumber(loaded_file, &index);
#endif
                } break;
            }

            if (new_token) {
                new_token->prev       = sentinel->prev;
                new_token->next       = sentinel;
                new_token->prev->next = new_token;
                new_token->next->prev = new_token; 
            }
        }
        EndTimer(&token_setup_time);

        OS_Timer token_read_time;
        StartTimer(&token_read_time);
        Token *iter_token = sentinel->next;

        f64 average_haversine = 0;
        s32 pair_count        = 0;

        while (iter_token != sentinel) {
            s32 index           = 0;
            f32 point_buffer[4] = {};
            while (index < 4 && iter_token != sentinel) {
                if (iter_token->type == TokenType_Real) {
                    point_buffer[index] = iter_token->real_num;
                    index++;
                }

                iter_token = iter_token->next;
            }

            f32 x0 = point_buffer[0];
            f32 y0 = point_buffer[1];
            f32 x1 = point_buffer[2];
            f32 y1 = point_buffer[3];
            pair_count++;

            f32 haversine      = ReferenceHaversine(x0, y0, x1, y1, EARTH_RADIUS);
            average_haversine += haversine;
        }
        EndTimer(&token_read_time);

        average_haversine /= pair_count;
        printf("The number of pairs are: %d\nThe Average sum is: %f\n\n", pair_count, average_haversine);

        OS_Timer free_time;
        StartTimer(&free_time);
#if CUSTOM_MEMORY
        FreeArena(&arena);
#else 
        FreeTokens(sentinel);
#endif
        EndTimer(&free_time);


        EndTimer(&total_time);
        {
        TIMED_BLOCK("Timings");
        OS_Timer profile_print;
        StartTimer(&profile_print);
        printf("      Total Seconds: %.4f\n\n", (f64)total_time.elapsed/(f64)os_freq); 
        printf("Memory Init Seconds: %.4f\n", (f64)memory_init_time.elapsed/(f64)os_freq); 
        printf("  Load File Seconds: %.4f\n", (f64)file_time.elapsed/(f64)os_freq); 
        printf("Token Setup Seconds: %.4f\n", (f64)token_setup_time.elapsed/(f64)os_freq); 
        printf(" Token read Seconds: %.4f\n", (f64)token_read_time.elapsed/(f64)os_freq); 
        printf("Free Memory Seconds: %.4f\n", (f64)free_time.elapsed/(f64)os_freq); 
        EndTimer(&profile_print);
        printf("  Profiling Seconds: %.4f\n", (f64)profile_print.elapsed/(f64)os_freq);
        }
    }
}
