#include <stdio.h>
#include <stdlib.h>
#include "haversine_generate.h"

char *LoadFile(char* filename) {

    char *result = 0;
    FILE* file = fopen(filename, "r");
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
                return NULL;
            }
            
            // Null-terminate the buffer
            buffer[file_size] = '\0';

            fclose(file);
            result = buffer;
        }
    }

    return result;
}

int main(int argc, char *argv[])
{
    if (argc < 2) {
        printf("Need to call exectuable with arguments: -- harversine_parser.exe (char *)<filename.json>\n");
    } else {
        char *filename = argv[1];

        char *loaded_file = LoadFile(filename);

        for(int i = 0; i < 40; i++) {
            printf("%c", loaded_file[i]);
        }
        printf("\n END OF TEST \n");
    }
}
