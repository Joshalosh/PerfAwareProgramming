#include <stdio.h>
#include <stdlib.h>

#include "haversine_generate.h"
#include "haversine.h"

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

Token *SearchForTokens(Memory_Arena *arena, File_Content loaded_file, int *index) {
    if (loaded_file.data[index] == '"') {
        index++;
        while (loaded_file.data[index] != '"'){
            // Get the size of the string
        }
        // initialise the array to the known size.
        // add all the elements to the array up to the size
        // set the token.string to point to the array

    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Need to call exectuable with arguments: -- harversine_parser.exe (char *)<filename.json>\n");
    } else {
        Memory_Arena arena;
        InitArena(&arena, 1024*1024);
        char *filename = argv[1];

        File_Content loaded_file = LoadFile(filename);
        Token *sentinel = (Token *)ArenaAlloc(&arena, sizeof(Token));
        sentinel = {};
        sentinel->next = sentinel;
        senitinel->prev = senitnel;

        for(int i = 0; i < loaded_file.size; i++) {
            Token *new_token = SearchForTokens(&arena, loaded_file, &i);

            if (new_token) {
                new_token->prev = sentinel->prev;
                new_token->next = sentinel;
                sentinel->prev->next = new_token;
                sentinel->prev = new_token;
            }
        }

        printf("\n Size of Token: %zu\n", sizeof(struct Token));
        FreeArena(&arena);
    }
}
