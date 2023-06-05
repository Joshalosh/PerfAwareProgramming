#if !defined(HAVERSINE_H)

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
        s32   num;
    };
    Token *next;
    Token *prev;
};


struct Memory_Arena {
    char *current;
    char *end;
};

void InitArena(Memory_Arena *arena, size_t size) {
    arena->current = (char *)malloc(size);
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

#define ZeroStruct(instance) ZeroSize(sizeof(instance), &(instance))
#define ZeroArray(count, pointer) ZeroSize(count*sizeof((pointer)[0]), pointer)
void ZeroSize(size_t size, void *ptr) {
    u8 *byte = (u8 *)ptr;
    while(size--)
    {
        *byte++ = 0;
    }
}

#define HAVERSINE_H
#endif
