#include "disassemble.h"

#define MAX_BUFFER_SIZE 256
#define MAX_REGISTERS 16
#define REGISTER_CHAR_LENGTH 3

void InitInstructionInfo(Instruction_Info *info, char *ch, int instruction_index,
                         Instruction *instruction_table, Instruction_Type instruction_type)
{
    info->opcode       = ch[instruction_index] & instruction_table[instruction_type].op_mask;
    info->d_bit        = ch[instruction_index] & instruction_table[instruction_type].d_mask;
    info->s_bit        = ch[instruction_index] & instruction_table[instruction_type].s_mask;
    info->w_bit        = ch[instruction_index] & instruction_table[instruction_type].w_mask;
    info->mod          = (ch[instruction_index+1] & instruction_table[instruction_type].mod_mask) >> 6;
    info->rm           = ch[instruction_index+1] & instruction_table[instruction_type].rm_mask;
    info->mid_bits     = instruction_table[instruction_type].mid_bits;
    info->op_name      = instruction_table[instruction_type].op_name;
    info->is_immediate = instruction_table[instruction_type].is_immediate;

    info->reg = (instruction_table[instruction_type].reg_on_first_byte) ? 
        ch[instruction_index] & instruction_table[instruction_type].reg_mask :
        (ch[instruction_index+1] & instruction_table[instruction_type].reg_mask) >> 3;

    info->has_second_instruction_byte = instruction_table[instruction_type].has_second_instruction_byte;
}

void PrintInstructionType(Instruction_Info instruction_info)
{
    printf("%s ", instruction_info.op_name);
}

char *reg_registers[2][8] = {{"al", "cl", "dl", "bl", "ah", "ch", "dh", "bh"}, 
                             {"ax", "cx", "dx", "bx", "sp", "bp", "si", "di"}};

char *mod_registers[8] = {"bx + si", "bx + di", "bp + si", "bp + di", "si", "di", "bp", "bx"};


Mod_Type CheckMod(Instruction_Info instruction_info)
{
    Mod_Type result = Mod_Count;
    switch (instruction_info.mod) {
        case Mod_MemModeNoDisp: {
            result = Mod_MemModeNoDisp;
        } break;

        case Mod_MemModeDisp8: {
            result = Mod_MemModeDisp8;
        } break;

        case Mod_MemModeDisp16: {
            result = Mod_MemModeDisp16;
        } break;

        case Mod_RegMode: {
            result = Mod_RegMode;
        } break;
        default : {
          printf("Something went real bad");
        } break;
    }

    return result;
}

void PrintRegister(Instruction_Info instruction_info, char *registers[2][8])
{
    printf("%s", registers[instruction_info.w_bit][instruction_info.reg]);
}

void PrintRM(Instruction_Info instruction_info, char *registers[8])
{
    printf("%s", registers[instruction_info.rm]);
}
void PrintRM(Instruction_Info instruction_info, char *registers[2][8])
{
    printf("%s", registers[instruction_info.w_bit][instruction_info.rm]);
}


void PrintDisplacement(s16 value)
{
    if (value == 0) {
        // Do nothing.

    } else {
        s16 sign_mask      = 0b10000000'00000000;
        s16 negative_check = value & sign_mask;

        char sign = (negative_check) ? '-' : '+';
        if(negative_check) {
            value *= -1;
        }

        printf(" %c %d", sign, value);
    }
}

s16 CalculateWord(char *ch, int instruction_index, int offset)
{
    s16 result;
    result = ((ch[instruction_index + offset + 1] & 0xFF) << 8) | (ch[instruction_index + offset] & 0xFF);

    return result;
}

void PrintMemModeOperations(Instruction_Info instruction_info, char *reg_registers[2][8], 
                            char *mod_registers[8], s16 displacement)
{
    if (instruction_info.d_bit) {
        PrintRegister(instruction_info, reg_registers);
        printf(", [");
        PrintRM(instruction_info, mod_registers);
        PrintDisplacement(displacement);
        printf("]");

    } else {
        printf("[");
        PrintRM(instruction_info, mod_registers);
        PrintDisplacement(displacement);
        printf("], ");
        PrintRegister(instruction_info, reg_registers);
    }
}

void PrintImmediateMemModeOperations(Instruction_Info instruction_info, char *ch, char *mod_registers[8],
                                     int instruction_index, s16 bytes_to_displacement, 
                                     int *bytes_to_next_instruction)
{
    printf("[");
    PrintRM(instruction_info, mod_registers);
    if (bytes_to_displacement) {
        s16 displacement = CalculateWord(ch, instruction_index, bytes_to_displacement);
        PrintDisplacement(displacement);
    }
    printf("], ");

    if (instruction_info.w_bit && !instruction_info.s_bit) {
        printf("word ");

        int value_offset = bytes_to_displacement + 2;
        s16 value        = CalculateWord(ch, instruction_index, value_offset);
        printf("%d", value);

        *bytes_to_next_instruction = 4 + bytes_to_displacement;

    } else {
        char *string = (instruction_info.w_bit) ? "word" : "byte";
        printf("%s %d", string, ch[instruction_index + 2 + bytes_to_displacement]);

        *bytes_to_next_instruction = 3 + bytes_to_displacement;
    }
}

void PrintImmediateRegModeOperations(Instruction_Info instruction_info, char *ch, char *reg_registers[2][8],
                                     int instruction_index, int *bytes_to_next_instruction)
{
    PrintRM(instruction_info, reg_registers);
    printf(", ");

    if(instruction_info.w_bit && !instruction_info.s_bit)
    {
        int value_offset = 2;
        s16 value        = CalculateWord(ch, instruction_index, value_offset);
        printf("%d", value);

        *bytes_to_next_instruction = 4;

    } else {
        printf("%d", ch[instruction_index + 2]);

        *bytes_to_next_instruction = 3;
    }
}

void DecodeInstruction(Instruction_Info instruction_info, Instruction_Type instruction_type, 
                       char *ch, int instruction_index, int *bytes_to_next_instruction)
{
    if (instruction_info.has_second_instruction_byte) {
        Mod_Type mod_type = CheckMod(instruction_info);
        switch (mod_type) { 

            case Mod_MemModeNoDisp: {
                // For some reason there is a funny exeption in this mode where
                // if the RM bits equal 110 then there is a 16 bit value that goes
                // directly into the register.
                if (instruction_info.rm == 0b110) {
                    if (instruction_info.is_immediate) {
                        PrintImmediateMemModeOperations(instruction_info, ch, mod_registers, 
                                                        instruction_index, 2, 
                                                        bytes_to_next_instruction);

                    } else {
                        int bytes_to_value = 2;
                        s16 value = CalculateWord(ch, instruction_index, bytes_to_value);
                        PrintRegister(instruction_info, reg_registers);
                        printf(", [");
                        printf("%d", value);
                        printf("]");

                        *bytes_to_next_instruction = 4;
                    }

                } else if (instruction_info.is_immediate) {
                    PrintImmediateMemModeOperations(instruction_info, ch, mod_registers, 
                                                    instruction_index, 0, 
                                                    bytes_to_next_instruction);

                } else {
                    PrintMemModeOperations(instruction_info, reg_registers, mod_registers, 0);

                    *bytes_to_next_instruction = 2;
                }
            } break;

            case Mod_MemModeDisp8: {
                int bytes_to_displacement = 2;
                s16 displacement = ch[instruction_index + bytes_to_displacement];
                PrintMemModeOperations(instruction_info, reg_registers, mod_registers, displacement);

                *bytes_to_next_instruction = 3;
            } break; 

            case Mod_MemModeDisp16: {
                int bytes_to_displacement = 2;

                if (instruction_info.is_immediate) {
                    PrintImmediateMemModeOperations(instruction_info, ch, mod_registers, 
                                                    instruction_index, bytes_to_displacement,
                                                    bytes_to_next_instruction);

                } else {

                    s16 displacement = CalculateWord(ch, instruction_index, bytes_to_displacement);
                    PrintMemModeOperations(instruction_info, reg_registers, mod_registers, displacement);

                    *bytes_to_next_instruction = 4;
                }
            } break;

            case Mod_RegMode: {
                if (!instruction_info.is_immediate) {
                    if (instruction_info.d_bit) {
                        PrintRegister(instruction_info, reg_registers);
                        printf(", ");
                        PrintRM(instruction_info, reg_registers);

                    } else {
                        PrintRM(instruction_info, reg_registers);
                        printf(", ");
                        if(!instruction_info.is_immediate) {
                            PrintRegister(instruction_info, reg_registers);
                        }
                    }

                    *bytes_to_next_instruction = 2;

                } else {
                    PrintImmediateRegModeOperations(instruction_info, ch, reg_registers,
                                                    instruction_index, bytes_to_next_instruction);
                }
            } break;  

            default: {
                printf("Something went really very truly bad");
            }
        }

    } else {
        switch (instruction_type) 
        {
            case InstructionType_MovImmediateReg: 
            {
                if (instruction_type == InstructionType_MovImmediateReg) {
                    instruction_info.w_bit >>= 3;
                }
                PrintRegister(instruction_info, reg_registers);
                printf(", ");

                s16 data = 0;
                if (instruction_info.w_bit) {
                    int data_offset = 1;
                    data = CalculateWord(ch, instruction_index, data_offset);
                    *bytes_to_next_instruction = 3;

                } else {
                    data = ch[instruction_index + 1];
                    *bytes_to_next_instruction = 2;
                }

                printf("%d", data);
            } break;

            case InstructionType_CmpImmediateWithAccum:
            case InstructionType_SubImmediateFromAccum:
            case InstructionType_AddImmediateToAccum:
            case InstructionType_MovMemToAccum: 
            {
                PrintRegister(instruction_info, reg_registers);
                printf(", ");

                s16 data = 0;

                if (instruction_info.w_bit) {
                    int data_offset = 1;
                    data = CalculateWord(ch, instruction_index, data_offset);
                    *bytes_to_next_instruction = 3;

                } else {
                    data = ch[instruction_index + 1];
                    *bytes_to_next_instruction = 2;
                }

                if (!instruction_info.is_immediate) {
                    printf("[%d]", data);

                } else {
                    printf("%d", data);
                }
            } break;

            case InstructionType_MovAccumToMem:
            {
                s16 data = 0;
                if (instruction_info.w_bit) {
                    int data_offset = 1;
                    data = CalculateWord(ch, instruction_index, data_offset);
                    *bytes_to_next_instruction = 3;

                } else {
                    data = ch[instruction_index + 1];
                    *bytes_to_next_instruction = 2;
                }

                if (!instruction_info.is_immediate) {
                    printf("[%d], ", data);

                } else {
                    printf("%d, ", data);
                }

                PrintRegister(instruction_info, reg_registers);
            }break;

            case InstructionType_JmpJE:
            case InstructionType_JmpJL:
            case InstructionType_JmpJLE:
            case InstructionType_JmpJB:
            case InstructionType_JmpJBE:
            case InstructionType_JmpJP:
            case InstructionType_JmpJO:
            case InstructionType_JmpJS:
            case InstructionType_JmpJNE:
            case InstructionType_JmpJNL:
            case InstructionType_JmpJNLE:
            case InstructionType_JmpJNB:
            case InstructionType_JmpJNBE:
            case InstructionType_JmpJNP:
            case InstructionType_JmpJNO:
            case InstructionType_JmpJNS:
            case InstructionType_Loop:
            case InstructionType_LoopZ:
            case InstructionType_LoopNZ:
            case InstructionType_JmpJCXZ:
            {
                printf("%d", ch[instruction_index+1]);
                *bytes_to_next_instruction = 2;
            } break;

            default:
            {
                printf("Something has seriously gone to shit");
            } break;
        }
    }
}

