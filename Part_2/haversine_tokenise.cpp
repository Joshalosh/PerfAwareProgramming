
void CopyData(char *dest, File_Content source, s32 size, s32 starting_index) {
    for (int i = 0; i < size; i++) {
        dest[i] = source.data[starting_index + i]; 
    }
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
    CopyData(string, loaded_file, string_size, start); 
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

Token *TokeniseNumber(Memory_Arena *arena, File_Content loaded_file, int *index) {
    s32 start = *index;
    s32 end = start;

    bool is_real = false;

    // Parse the number.
    while (end < loaded_file.size) {
        char c = loaded_file.data[end];
        if (IsDigit(c)) {
            end ++;
        } else if (c == '.' && !is_real) {
            end ++;
            is_real = true;
        } else {
            break;
        }
    }

    // Allocate memory for the number string and copy it.
    s32 string_size = end - start;
    char *num_string = (char *)ArenaAlloc(arena, string_size + 1);
    CopyData(num_string, loaded_file, string_size, start);
    num_string[string_size] = '\0'; // Null-terminate the string.

    // Allocate and initialise a new Token.
    Token *token = (Token *)ArenaAlloc(arena, sizeof(Token));
    if (is_real) {
        token->type = TokenType_Real;
        token->real_num = strtof(num_string, NULL); // Convert string to float.
    } else {
        token->type = TokenType_Num;
        token->num = strtol(num_string, NULL, 10); // Convert string to int.
    }
    token->next = NULL;
    token->prev = NULL;

    *index = end;

    return token;
}

