#include <stdio.h>
#include <stdlib.h>

#include "haversine_generate.h"
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

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Need to call exectuable with arguments: -- harversine_parser.exe (char *)<filename.json>\n");
    } else {
        Memory_Arena arena;
        InitArena(&arena, 1024*1024*1024);
        char *filename = argv[1];

        File_Content loaded_file = LoadFile(filename);

        Token *sentinel = (Token *)ArenaAlloc(&arena, sizeof(Token));
        ZeroSize(sizeof(*sentinel), sentinel);
        sentinel->next = sentinel;
        sentinel->prev = sentinel;

        for (int index = 0; index < loaded_file.size; index++) {
            Token *new_token = NULL;
            switch (loaded_file.data[index]) {
                case '"': {
                    new_token = TokeniseString(&arena, loaded_file, &index);
                } break;

                case '0': case '1': case '2': case '3': case '4':
                case '5': case '6': case '7': case '8': case '9': case '-': {
                    new_token = TokeniseNumber(&arena, loaded_file, &index);
                } break;
            }

            if (new_token) {
                new_token->prev       = sentinel->prev;
                new_token->next       = sentinel;
                new_token->prev->next = new_token;
                new_token->next->prev = new_token; 
            }
        }

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

        average_haversine /= pair_count;
        printf("The number of pairs are: %d\nThe Average sum is: %f\n", pair_count, average_haversine);

        FreeArena(&arena);
    }
}
