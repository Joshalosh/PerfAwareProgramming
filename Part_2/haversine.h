#if !defined(HAVERSINE_H)

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

#define HAVERSINE_H
#endif
