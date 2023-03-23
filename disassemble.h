#if !defined(GAME_ENTITY_H)

#define ARRAY_COUNT(array) (sizeof(array) / ((array)[0]))

typedef uint8_t  u8;
typedef uint16_t u16;

typedef int8_t  s8;
typedef int16_t s16;

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
    bool has_second_instruction_byte;
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

    InstructionType_Count,
};

struct Instruction {
    char *op_name;
    u8 op_mask;
    u8 op_bits;
    u8 d_mask;
    u8 w_mask;
    u8 mod_mask;
    u8 reg_mask;
    u8 mid_bits_mask;
    u8 mid_bits;
    u8 rm_mask;

    bool reg_on_first_byte;
    bool is_immediate;
    bool has_second_instruction_byte;
};

Instruction instruction_table[InstructionType_Count] = {
    {"MOV", 0b1111'1100, 0b1000'1000, 2, 1, 0b11'000'000, 0b00'111'000, NULL, NULL, 0b00'000'111, false, false, true},
    {"MOV", 0b1111'1110, 0b1100'0110, NULL, 1, 0b11'000'000, 0, 0b00'000'000, 0, 0b00'000'111, false, true, true},
    {"MOV", 0b1111'0000, 0b1011'0000, NULL, 0b0000'1000, NULL, 0b0000'0111, NULL, NULL, NULL, true, true, false},
    {"MOV", 0b1111'1110, 0b1010'0000, NULL, 1, NULL, NULL, NULL, NULL, NULL, false, false, false},
    {"MOV", 0b1111'1110, 0b1010'0010, NULL, 1, NULL, NULL, NULL, NULL, NULL, false, false, false},
    {"MOV", 0b1111'1111, 0b1000'1110, NULL, NULL, 0b11'000'000, NULL, NULL, NULL, 0b00'000'111, false, false, true},
    {"MOV", 0b1111'1111, 0b1000'1100, NULL, NULL, 0b11'000'000, NULL, NULL, NULL, 0b00'000'111, false, false, true}};

#define GAME_ENTITY_H
#endif
