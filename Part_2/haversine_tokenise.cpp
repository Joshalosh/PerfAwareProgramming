
static void 
CopyData(char *dest, File_Content source, s32 size, s32 starting_index) {
    for (int i = 0; i < size; i++) {
        dest[i] = source.data[starting_index + i]; 
    }
}

static Token *
TokeniseString(Memory_Arena *arena, File_Content loaded_file, int *index) {
    s32 start = *index + 1; // This skips the first ".
    s32 end   = start;

    // Find the end of the string
    while (loaded_file.data[end] != '"' || (end > 0 && loaded_file.data[end - 1] == '\\')) {
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

static void
IgnoreString(File_Content loaded_file, int *index) {
    s32 start = *index + 1; // This skips the first ".
    s32 end   = start;

    // Find the end of the string
    while (loaded_file.data[end] != '"' || (end > 0 && loaded_file.data[end - 1] == '\\')) {
        end++;

        Assert(end <= loaded_file.size);
    }
    
    *index = end;
}

static bool 
IsDigit(char c) {
    return (c >= '0' && c<= '9');
}

static s64 
StringToS32(char *str) {
    s32 result = 0;
    s8 sign    = 1;

    if (*str == '-') {
        sign = -1;
        str++;
    }

    while(IsDigit(*str)) {
        result = result * 10 + (*str - '0');
        str++;
    }

    result *= sign;
    return result;
}

static float 
StringToFloat(char *str) {
    float result = 0.0f;
    float sign   = 1.0f;

    if (*str == '-') {
        sign = -1.0f;
        str++;
    }

    while (IsDigit(*str)) {
        // Calculate the number by subtracting the character away from '0'.
        // '0' == 48 and 9 == 57 in ascii value, so by subtracting the current character
        // from '0' in ascii we will get the correct number value. Then we add that answer
        // to a power of 10 calculation to get the correct number before the decimal. This
        // will run until the decimal point or null terminator is hit.
        result = result * 10.0f + (*str - '0');
        str++;
    }

    if (*str == '.') {
        float fraction = 1.0f;
        str++;
        while (IsDigit(*str)) {
            // After the decimal we basically do the same trick but the opposite.
            // This time dividing by 10 and turning the current number into
            // a fraction. This will run until the null terminator is hit.
            fraction /= 10.0f;
            result   += (*str - '0') * fraction;
            str++;
        }
    }

    result *= sign;
    return result;
}

static float 
StringToFloat(char *str, s32 string_size) {
    float result = 0.0f;
    float sign   = 1.0f;
    s32 count    = 0;

    if (*str == '-') {
        sign = -1.0f;
        str++;
        count++;
    }

    while (IsDigit(*str) && count < string_size) {
        // Calculate the number by subtracting the character away from '0'.
        // '0' == 48 and 9 == 57 in ascii value, so by subtracting the current character
        // from '0' in ascii we will get the correct number value. Then we add that answer
        // to a power of 10 calculation to get the correct number before the decimal. This
        // will run until the decimal point or null terminator is hit.
        result = result * 10.0f + (*str - '0');
        str++;
        count++;
    }

    if (*str == '.') {
        float fraction = 1.0f;
        str++;
        while (IsDigit(*str) && count < string_size) {
            // After the decimal we basically do the same trick but the opposite.
            // This time dividing by 10 and turning the current number into
            // a fraction. This will run until the null terminator is hit.
            fraction /= 10.0f;
            result   += (*str - '0') * fraction;
            str++;
            count++;
        }
    }

    result *= sign;
    return result;
}

static Token *
TokeniseNumber(Memory_Arena *arena, File_Content loaded_file, int *index) {
    s32 start = *index;
    s32 end = start;

    bool is_real = false;

    // Parse the number.
    while (end < loaded_file.size) {
        char c = loaded_file.data[end];
        if (IsDigit(c) || c == '-') {
            end++;
        } else if (c == '.' && !is_real) {
            end++;
            is_real = true;
        } else {
            break;
        }
    }

    // Allocate memory for the number string and copy it.
    s32 string_size = end - start;
    // TODO: Can have this as a stack based array instead of
    // sitting in the arena, but it doesn't seem to help when
    // it comes to 10 million haversine points.
    char num_string[20] = {};
    CopyData(num_string, loaded_file, string_size, start);
    //num_string[string_size] = '\0'; // Null-terminate the string.

    // Allocate and initialise a new Token.
    Token *token = (Token *)ArenaAlloc(arena, sizeof(Token));
    if (is_real) {
        token->type = TokenType_Real;
#if 0
        token->real_num = strtof(num_string, NULL); // Convert string to float.
#else 
        token->real_num = StringToFloat(num_string, string_size);
#endif
    } else {
        token->type = TokenType_Num;
#if 0
        token->num = strtol(num_string, NULL, 10); // Convert string to int.
#else
        printf("This wasn't meant to get hit");
        token->num = StringToS32(num_string);
#endif
    }
    token->next = NULL;
    token->prev = NULL;

    *index = end;

    return token;
}

static Token *
TokeniseNumber(File_Content loaded_file, int *index) {
    s32 start = *index;
    s32 end = start;

    bool is_real = false;

    // Parse the number.
    while (end < loaded_file.size) {
        char c = loaded_file.data[end];
        if (IsDigit(c) || c == '-') {
            end++;
        } else if (c == '.' && !is_real) {
            end++;
            is_real = true;
        } else {
            break;
        }
    }

    // Allocate memory for the number string and copy it.
    s32 string_size = end - start;
    // TODO: Can have this as a stack based array instead of
    // sitting in the arena, but it doesn't seem to help when
    // it comes to 10 million haversine points.
    char num_string[20] = {};
    CopyData(num_string, loaded_file, string_size, start);
    //num_string[string_size] = '\0'; // Null-terminate the string.

    // Allocate and initialise a new Token.
    Token *token = (Token *)calloc(1, sizeof(Token));
    if (is_real) {
        token->type = TokenType_Real;
#if 0
        token->real_num = strtof(num_string, NULL); // Convert string to float.
#else 
        token->real_num = StringToFloat(num_string, string_size);
#endif
    } else {
        token->type = TokenType_Num;
#if 0
        token->num = strtol(num_string, NULL, 10); // Convert string to int.
#else
        printf("This wasn't meant to get hit");
        token->num = StringToS32(num_string);
#endif
    }
    token->next = NULL;
    token->prev = NULL;

    *index = end;

    return token;
}

static void
FreeTokens (Token *sentinel) {
    Token *current_token = sentinel->next;
    while (current_token != sentinel) {
        Token *next_token = current_token->next;
        free(current_token);
        current_token = next_token;
    }
    free(sentinel);
}

