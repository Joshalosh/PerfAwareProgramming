struct Decomposed_Virtual_Address
{
    u16 pml_4_index;
    u16 directory_ptr_index;
    u16 directory_index;
    u16 table_index;
    u32 offset;
};

static void Print(Decomposed_Virtual_Address address)
{
    printf("|%3u|%3u|%3u|%3u|%10u|",
           address.pml_4_index, address.directory_ptr_index,
           address.directory_index, address.table_index,
           address.offset);
}

static void PrintAsLine(char const *label, Decomposed_Virtual_Address address)
{
    printf("%s", label);
    Print(address);
    printf("\n");
}

static Decomposed_Virtual_Address DecomposePointer4K(void *ptr)
{
    Decomposed_Virtual_Address result = {};

    u64 address = (u64)ptr;
    result.pml_4_index         = ((address >> 39) & 0x1ff);
    result.directory_ptr_index = ((address >> 30) & 0x1ff);
    result.directory_index     = ((address >> 21) & 0x1ff);
    result.table_index         = ((address >> 12) & 0x1ff);
    result.offset              = ((address >>  0) & 0x1ff);

    return result;
}

static Decomposed_Virtual_Address DecomposePointer2MB(void *ptr)
{
    Decomposed_Virtual_Address result = {};

    u64 address = (u64)ptr;
    result.pml_4_index         = ((address >> 39) & 0x1ff);
    result.directory_ptr_index = ((address >> 30) & 0x1ff);
    result.directory_index     = ((address >> 21) & 0x1ff);
    result.offset              = ((address >>  0) & 0x1ff);

    return result;
}

static Decomposed_Virtual_Address DecomposePointer1GB(void *ptr)
{
    Decomposed_Virtual_Address result = {};

    u64 address = (u64)ptr;
    result.pml_4_index         = ((address >> 39) & 0x1ff);
    result.directory_ptr_index = ((address >> 30) & 0x1ff);
    result.offset              = ((address >>  0) & 0x1ff);

    return result;
}
