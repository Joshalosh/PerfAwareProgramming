#if !defined(GAME_ENTITY_H)

#define ARRAY_COUNT(array) (sizeof(array) / sizeof((array)[0]))

typedef uint8_t  u8;
typedef uint16_t u16;

typedef int8_t  s8;
typedef int16_t s16;

union Flags {
    u8 flag_array[9];
    struct {
        u8 sign;
        u8 zero;
        u8 aux_carry;
        u8 parity;
        u8 carry;
        u8 overflow;
        u8 directional;
        u8 interrupt;
        u8 trap;
    };
};

enum Mod_Type : u8 {
    Mod_MemModeNoDisp = 0b00,
    Mod_MemModeDisp8  = 0b01,
    Mod_MemModeDisp16 = 0b10,
    Mod_RegMode       = 0b11,

    Mod_Count,
};

enum Instruction_Type : u8 {
    InstructionType_MovRegOrMem,
    InstructionType_MovImmediateRegOrMem,
    InstructionType_MovImmediateReg,
    InstructionType_MovMemToAccum,
    InstructionType_MovAccumToMem,
    InstructionType_MovRegOrMemToSegmentReg,
    InstructionType_MovSegmentRegToRegOrMem,

    InstructionType_AddRegOrMem,
    InstructionType_AddImmediateRegOrMem,
    InstructionType_AddImmediateToAccum,

    InstructionType_SubRegOrMem,
    InstructionType_SubImmediateRegOrMem,
    InstructionType_SubImmediateFromAccum,

    InstructionType_CmpRegOrMem,
    InstructionType_CmpImmediateRegOrMem,
    InstructionType_CmpImmediateWithAccum,
    InstructionType_CmpAsciiForSubtract,
    InstructionType_CmpDecimalForSubstract,
    InstructionType_CmpUMultiply,
    InstructionType_CmpSMultiply,
    InstructionType_CmpAsciiForMultiply,
    InstructionType_CmpUDivide,
    InstructionType_CmpSDivide,
    InstructionType_CmpAsciiForDivide,
    InstructionType_CmpConvertByteToWord,
    InstructionType_CmpConvertByteToDWord,

    InstructionType_JmpJE,
    InstructionType_JmpJL,
    InstructionType_JmpJLE,
    InstructionType_JmpJB,
    InstructionType_JmpJBE,
    InstructionType_JmpJP,
    InstructionType_JmpJO,
    InstructionType_JmpJS,
    InstructionType_JmpJNE,
    InstructionType_JmpJNL,
    InstructionType_JmpJNLE,
    InstructionType_JmpJNB,
    InstructionType_JmpJNBE,
    InstructionType_JmpJNP,
    InstructionType_JmpJNO,
    InstructionType_JmpJNS,
    InstructionType_Loop,
    InstructionType_LoopZ,
    InstructionType_LoopNZ,
    InstructionType_JmpJCXZ,

    InstructionType_Count,
};

struct Type_Bucket {
    Instruction_Type array[InstructionType_Count];
    int size;
};

enum Arithmetic_Type : u8 {
    ArithType_None,
    ArithType_Add,
    ArithType_Sub,
    ArithType_Cmp,

    ArithType_Count,
};

struct Instruction_Info {
    u8 opcode;
    u8 d_bit;
    u8 w_bit;
    u8 s_bit;
    u8 mod;
    u8 reg;
    u8 rm; 
    u8 mid_bits;

    char *op_name;
    bool is_immediate;
    bool has_second_instruction_byte;
    bool is_arithmetic;

    u8 arithmetic_type;
};

struct Instruction {
    char *op_name;
    u8 op_mask;
    u8 op_bits;
    u8 d_mask;
    u8 s_mask;
    u8 w_mask;
    u8 mod_mask;
    u8 reg_mask;
    u8 mid_bits;
    u8 rm_mask;

    bool reg_on_first_byte;
    bool is_immediate;
    bool has_second_instruction_byte;
    bool is_arithmetic;

    u8 arithmetic_type;
};

Instruction instruction_table[InstructionType_Count] = {
    {"mov", 0b1111'1100, 0b1000'1000, 2, NULL, 1, 0b11'000'000, 0b00'111'000, NULL, 0b00'000'111, false, false, true, false, ArithType_None},
    {"mov", 0b1111'1110, 0b1100'0110, NULL, NULL, 1, 0b11'000'000, 0, 0b00'000'000, 0b00'000'111, false, true, true, false, ArithType_None},
    {"mov", 0b1111'0000, 0b1011'0000, NULL, NULL, 0b0000'1000, NULL, 0b0000'0111, NULL, NULL, true, true, false, false, ArithType_None},
    {"mov", 0b1111'1110, 0b1010'0000, NULL, NULL, 1, NULL, NULL, NULL, NULL, false, false, false, false, ArithType_None},
    {"mov", 0b1111'1110, 0b1010'0010, NULL, NULL, 1, NULL, NULL, NULL, NULL, false, false, false, false, ArithType_None},
    {"mov", 0b1111'1111, 0b1000'1110, NULL, NULL, NULL, 0b11'000'000, NULL, NULL, 0b00'000'111, false, false, true, false, ArithType_None},
    {"mov", 0b1111'1111, 0b1000'1100, NULL, NULL, NULL, 0b11'000'000, NULL, NULL, 0b00'000'111, false, false, true, false, ArithType_None},

    {"add", 0b1111'1100, 0b0, 2, NULL, 1, 0b11'000'000, 0b00'111'000, NULL, 0b00'000'111, false, false, true, true, ArithType_Add},
    {"add", 0b1111'1100, 0b1000'0000, NULL, 2, 1, 0b11'000'000, NULL, 0b0, 0b00'000'111, false, true, true, true, ArithType_Add},
    {"add", 0b1111'1110, 0b0000'0100, NULL, NULL, 1, NULL, NULL, NULL, NULL, NULL, true, false, true, ArithType_Add},

    {"sub", 0b1111'1100, 0b0010'1000, 2, NULL, 1, 0b11'000'000, 0b00'111'000, NULL, 0b00'000'111, false, false, true, true, ArithType_Sub},
    {"sub", 0b1111'1100, 0b1000'0000, NULL, 2, 1, 0b11'000'000, NULL, 0b00'101'000, 0b00'000'111, false, true, true, true, ArithType_Sub},
    {"sub", 0b1111'1110, 0b0010'1100, NULL, NULL, 1, NULL, NULL, NULL, NULL, NULL, true, false, true, ArithType_Sub},

    {"cmp", 0b1111'1100, 0b0011'1000, 2, NULL, 1, 0b11'000'000, 0b00'111'000, NULL, 0b00'000'111, false, false, true, true, ArithType_Cmp},
    {"cmp", 0b1111'1100, 0b1000'0000, NULL, 2, 1, 0b11'000'000, NULL, 0b00'111'000, 0b00'000'111, false, true, true, true, ArithType_Cmp},
    {"cmp", 0b1111'1110, 0b0011'1100, NULL, NULL, 1, NULL, NULL, NULL, NULL, false, true, false, true, ArithType_Cmp},
    {"cmp", 0xFF, 0b0011'1111, NULL, NULL, NULL, NULL, NULL, NULL, NULL, false, false, false, true, ArithType_Cmp},
    {"cmp", 0xFF, 0b0010'1111, NULL, NULL, NULL, NULL, NULL, NULL, NULL, false, false, false, true, ArithType_Cmp},
    {"cmp", 0b1111'1110, 0b1111'0110, NULL, NULL, 1, 0b11'000'000, NULL, 0b00'100'000, 0b00'000'111, false, false, true, true, ArithType_Cmp},
    {"cmp", 0b1111'1110, 0b1111'0110, NULL, NULL, 1, 0b11'000'000, NULL, 0b00'101'000, 0b00'000'111, false, false, true, true, ArithType_Cmp},
    {"cmp", 0xFF, 0b1101'0100, NULL, NULL, NULL, NULL, NULL, NULL, NULL, false, false, true, true, ArithType_Cmp},
    {"cmp", 0b1111'1110, 0b1111'0110, NULL, NULL, 1, 0b11'000'000, NULL, 0b00'110'000, 0b00'000'111, false, false, true, true, ArithType_Cmp},
    {"cmp", 0b1111'1110, 0b1111'0110, NULL, NULL, 1, 0b11'000'000, NULL, 0b00'111'000, 0b00'000'111, false, false, true, true, ArithType_Cmp},
    {"cmp", 0xFF, 0b1101'0101, NULL, NULL, NULL, NULL, NULL, NULL, NULL, false, false, true, true, ArithType_Cmp},
    {"cmp", 0xFF, 0b1001'1000, NULL, NULL, NULL, NULL, NULL, NULL, NULL, false, false, false, true, ArithType_Cmp},
    {"cmp", 0xFF, 0b1001'1001, NULL, NULL, NULL, NULL, NULL, NULL, NULL, false, false, false, true, ArithType_Cmp},

    {"je", 0xFF, 0b0111'0100, NULL, NULL, NULL, NULL, NULL, NULL, NULL, false, false, false, false, ArithType_None},
    {"jl", 0xFF, 0b0111'1100, NULL, NULL, NULL, NULL, NULL, NULL, NULL, false, false, false, false, ArithType_None},
    {"jle", 0xFF, 0b0111'1110, NULL, NULL, NULL, NULL, NULL, NULL, NULL, false, false, false, false, ArithType_None},
    {"jb", 0xFF, 0b0111'0010, NULL, NULL, NULL, NULL, NULL, NULL, NULL, false, false, false, false, ArithType_None},
    {"jbe", 0xFF, 0b0111'0110, NULL, NULL, NULL, NULL, NULL, NULL, NULL, false, false, false, false, ArithType_None},
    {"jp", 0xFF, 0b0111'1010, NULL, NULL, NULL, NULL, NULL, NULL, NULL, false, false, false, false, ArithType_None},
    {"jo", 0xFF, 0b0111'0000, NULL, NULL, NULL, NULL, NULL, NULL, NULL, false, false, false, false, ArithType_None},
    {"js", 0xFF, 0b0111'1000, NULL, NULL, NULL, NULL, NULL, NULL, NULL, false, false, false, false, ArithType_None},
    {"jne", 0xFF, 0b0111'0101, NULL, NULL, NULL, NULL, NULL, NULL, NULL, false, false, false, false, ArithType_None},
    {"jnl", 0xFF, 0b0111'1101, NULL, NULL, NULL, NULL, NULL, NULL, NULL, false, false, false, false, ArithType_None},
    {"jnle", 0xFF, 0b0111'1111, NULL, NULL, NULL, NULL, NULL, NULL, NULL, false, false, false, false, ArithType_None},
    {"jnb", 0xFF, 0b0111'0011, NULL, NULL, NULL, NULL, NULL, NULL, NULL, false, false, false, false, ArithType_None},
    {"jnbe", 0xFF, 0b0111'0111, NULL, NULL, NULL, NULL, NULL, NULL, NULL, false, false, false, false, ArithType_None},
    {"jnp", 0xFF, 0b0111'1011, NULL, NULL, NULL, NULL, NULL, NULL, NULL, false, false, false, false, ArithType_None},
    {"jno", 0xFF, 0b0111'0001, NULL, NULL, NULL, NULL, NULL, NULL, NULL, false, false, false, false, ArithType_None},
    {"jns", 0xFF, 0b0111'1001, NULL, NULL, NULL, NULL, NULL, NULL, NULL, false, false, false, false, ArithType_None},
    {"loop", 0xFF, 0b1110'0010, NULL, NULL, NULL, NULL, NULL, NULL, NULL, false, false, false, false, ArithType_None},
    {"loopz", 0xFF, 0b1110'0001, NULL, NULL, NULL, NULL, NULL, NULL, NULL, false, false, false, false, ArithType_None},
    {"loopnz", 0xFF, 0b1110'0000, NULL, NULL, NULL, NULL, NULL, NULL, NULL, false, false, false, false, ArithType_None},
    {"jcxz", 0xFF, 0b1110'0011, NULL, NULL, NULL, NULL, NULL, NULL, NULL, false, false, false, false, ArithType_None}};

#define GAME_ENTITY_H
#endif
