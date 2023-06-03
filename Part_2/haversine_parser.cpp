#include <stdio.h>
#include <stdlib.h>
#include "haversine_generate.h"

struct File_Content {
    char *data;
    size_t size;
};

enum Token_Type {
    TokenType_None,
    TokenType_String,
    TokenType_Real,
    TokenType_Num,

    TokenType_Count,
};
struct Token {
    Token_Type type;
    union {
        char *string;
        float real_num;
        int   num;
    };
    Token *next;
    Token *prev;
};

struct Memory_Arena {
    char *current;
    char *end;
};

void InitArena(Memory_Arena *arena, size_t size) {
    arena->current = malloc(size);
    arena->end     = arena->current + size;
}

void *ArenaAlloc(Memory_Arena *arena, size_t size) {
    void *result = NULL;
    if (!(arena->current + size > arena->end)) { // Not enough space left in the arena.
        result = arena->current;
        arena->current += size;
    }
    

    return result;
}

void FreeArena(Memory_Arena *arena) {
    free(arena->current);
}


File_Content LoadFile(char* filename) {

    File_Content result = {};
    FILE* file = fopen(filename, "rb");
    if (!file) {
        printf("Failed to open file\n");
    }
    
    if(file) {
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
        
        if(buffer) {
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
        char *filename = argv[1];

        File_Content loaded_file = LoadFile(filename);

        for(int i = 0; i < loaded_file.size; i++) {
            printf("%c", loaded_file.data[i]);
        }
        printf("\n END OF TEST \n");
    }
}
