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

Token *TokeniseString(Memory_Arena *arena, File_Content loaded_file, int *index) {
    s32 start = *index + 1; // This skips the first ".
    s32 end   = start;

    // Find the end of the string
    while (loaded_file.data[end] != '"' || (end > 0 && loaded_file.data[end - 1] == '\\')){
        end++;

        Assert(end <= loaded_file.size);
    }

    // Allocate memory for the string and copy it.
    s32 string_size = end - start;
    char *string = (char *)ArenaAlloc(arena, string_size + 1);
    for (int i = 0; i < string_size; i++) {
        string[i] = loaded_file.data[start + i]; 
    }
    string[string_size] = '\0'; // Null-terminate the string.


    // Allocate and initialise a new Token
    Token *token  = (Token *)ArenaAlloc(arena, sizeof(Token));
    token->type   = TokenType_String;
    token->string = string;
    token->next   = NULL;
    token->prev   = NULL;

    *index = end;

    return token;
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
        sentinel->prev = sentinel;

        Token *new_token = {};
        for(int index = 0; index < loaded_file.size; index++) {
            switch (loaded_file.data[index]){
                case '"': {
                    new_token = TokeniseString(&arena, loaded_file, &index);
                } break;
            }


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
