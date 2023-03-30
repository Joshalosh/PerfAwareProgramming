#include <stdio.h>
#include <stdint.h>
#include "decode.cpp"

int main() {
    FILE *file;
    char ch[MAX_BUFFER_SIZE] = {};
    int file_size = 0;

#if 0
    file = fopen("single_register", "rb");
#endif

#if 0
    file = fopen("multi_register", "rb");
#endif

#if 0
    file = fopen("more_movs", "rb");
#endif

#if 1
    file = fopen("challenge", "rb");
#endif

#if 0
    file = fopen("add_sub_cmp", "rb");
#endif

    printf("The assembly instructions of this file is: \n");

    if(file != NULL) {
        fread(ch, sizeof(ch),1,file);

        fseek(file, 0, SEEK_END);
        file_size = ftell(file);

        fclose(file);

    } else {
        printf("The file can't be opened \n");
    }

    int instruction_index = 0;
    while (instruction_index < file_size) {

        Instruction_Type instruction_type = InstructionType_Count;
        for (int index = 0; index < InstructionType_Count; index++) {
            if ((ch[instruction_index] & instruction_table[index].op_mask) == instruction_table[index].op_bits) {
                instruction_type = (Instruction_Type)index;
                break;
            }
        }

        int bytes_to_next_instruction = 0;

        if (instruction_type != InstructionType_Count) {
            Instruction_Info instruction_info = {};
            InitInstructionInfo(&instruction_info, ch, instruction_index, instruction_table, instruction_type);
            PrintInstructionType(instruction_info);

            DecodeInstruction(instruction_info, instruction_type, ch, 
                              instruction_index, &bytes_to_next_instruction);
        }

        instruction_index += bytes_to_next_instruction;
        printf("\n");
    }


    printf("\n");
}

#if 0
for (int i = 0; i < 2; i++) {
    for (int j = 0; j < 8; j++) {
        for (int k = 0; k < 2; k++) {

            printf("%c", registers[i][j][k]);
        }
        printf(" ");
    }
    printf("\n");
}

enum Mov_Type : char
{
    Mov_RegMov,
    Mov_ImmediateMem,
    Mov_ImmediateReg,
    Mov_MemToAccumulator,
    Mov_AccumulatorToMem,

    Add_RegMov,
    Add_ImmediateMem,
    Add_ImmediateAccum,

    Jmp,
};

char *SetRegister(char *ch, int index, char registers[MAX_REGISTERS][REGISTER_CHAR_LENGTH],
                  Mov_Type mov_type, bool is_register_one)
{
    char *result;
    char temp;
    char w_mask = mov_type == Mov_ImmediateReg ? 8 : 1;
    char bottom_three_bits_mask = 7;

    if(mov_type == Mov_ImmediateReg)
    {
        temp = ch[index] & bottom_three_bits_mask;
    }
    else if(mov_type == Add_ImmediateAccum)
    {
        temp = 0;
    }
    else 
    {
        temp = is_register_one ? ((ch[index+1] >> 3) & bottom_three_bits_mask) : ch[index+1] & bottom_three_bits_mask;
    }
    for(int reg_index = 0; reg_index < 8; reg_index++)
    {
        if((temp == reg_index))
        {
            if(ch[index] & w_mask)
            {
                result = &registers[reg_index+8][0];
                break;
            }
            else 
            {
                result = &registers[reg_index][0];
                break;
            }
        }
    }
    return result;
}

void PrintRegister(char *reg)
{
    for(int i = 0; i < 2; i++)
    {
        printf("%c", reg[i]);
    }
    printf(" ");
}

int16_t GetWordValue(char *ch, int instruction_index) 
{
    int16_t result = ((ch[instruction_index+2] & 0xFF) << 8) | (ch[instruction_index+1] & 0xFF);
    return result;
}

void PrintAddressCalculation(char *ch, int instruction_index, char bottom_three_bits_mask)
{
    uint8_t addressing_mode = ch[instruction_index+1] & bottom_three_bits_mask;
    switch(addressing_mode)
    {
        case 0b000: printf("[BX + SI] "); break;
        case 0b001: printf("[BX + DI] "); break;
        case 0b010: printf("[BP + SI] "); break;
        case 0b011: printf("[BP + DI] "); break;
        case 0b100: printf("[SI] ");      break;
        case 0b101: printf("[DI] ");      break;
        case 0b110: printf("[BP] ");      break;
        case 0b111: printf("[BX] ");      break;
    }
}

void PrintAddressCalculation(char *ch, int instruction_index, char bottom_three_bits_mask, int16_t displacement)
{
    uint8_t addressing_mode = ch[instruction_index+1] & bottom_three_bits_mask;

    char sign; 

    if((displacement >> 15) == 0)
    {
        sign = '+';
    }
    else 
    {
        sign = '-';
        displacement *= -1;
    }
    switch(addressing_mode)
    {
        case 0b000: printf("[BX + SI %c %d] ", sign, displacement); break;
        case 0b001: printf("[BX + DI %c %d] ", sign, displacement); break;
        case 0b010: printf("[BP + SI %c %d] ", sign, displacement); break;
        case 0b011: printf("[BP + DI %c %d] ", sign, displacement); break;
        case 0b100: printf("[SI %c %d] ", sign, displacement);      break;
        case 0b101: printf("[DI %c %d] ", sign, displacement);      break;
        case 0b110: printf("[BP %c %d] ", sign, displacement);      break;
        case 0b111: printf("[BX %c %d] ", sign, displacement);      break;
    }
}

void CalculateImmediateMemoryValue(char *ch, int instruction_index, int *index_counter, char w_mask,
                                   int bytes_before_value, int bytes_before_next_instruction)
{
    if(w_mask == 2)
    {
        int8_t value = ch[bytes_before_value+1];
        printf("WORD %d\n", value);
        *index_counter += bytes_before_next_instruction;
    }
    else if(ch[instruction_index] & w_mask)
    {
        int index_offset = bytes_before_value;
        int16_t value = GetWordValue(ch, index_offset);
        printf("WORD %d\n", value);
        bytes_before_next_instruction += 1;
        *index_counter += bytes_before_next_instruction;
    }
    else 
    {
        int8_t value = ch[bytes_before_value+1];
        printf("BYTE %d\n", value);
        *index_counter += bytes_before_next_instruction;
    }
}

int main()
{
    FILE *file;
    char ch[MAX_BUFFER_SIZE] = {};
    int file_size = 0;

#if 0
    file = fopen("more_movs", "rb");
#endif

#if 0
    file = fopen("challenge", "rb");
#endif

#if 1
    file = fopen("add_sub_cmp", "rb");
#endif

    printf("The assembly instructions of this file is: \n");

    if(file != NULL)
    {
        fread(ch, sizeof(ch),1,file);

        fseek(file, 0, SEEK_END);
        file_size = ftell(file);

        fclose(file);
    }
    else
    {
        printf("The file can't be opened \n");
    }

#if 1
    // These are the MOV masks
    char mov_mask               = 0b10001000;
    char mem_immediate_mov_mask = 0b11000110;
    char reg_immediate_mov_mask = 0b10110000;
    char mem_to_accum_mov_mask  = 0b10100000;
    char accum_to_mem_mov_mask  = 0b10100010;

    // These are the Add masks
    char add_mask = 0b0;
    char add_sub_cmp_mask = 0b10000000;
    char immediate_to_accum_add_mask = 0b00000100;

    // These are the SUB masks
    char sub_mask = 0b101;
    char reg_sub_mask = 0b00101000;
    char immediate_from_accum_sub_mask = 0b00101100;

    // These are the CMP masks
    char cmp_mask = 0b111;
    char reg_cmp_mask = 0b00111000;
    char immediate_with_accum = 0b00111100;

    // These are the JUMP masks
    char Je_Jz   = 0b01110100;
    char Jl_Jnge = 0b01111100;
    char Jle_Jng = 0b01111110;
    char Jb_Jnae = 0b01110010;
    char Jbe_Jna = 0b01110110; 
    char Jp_Jpe  = 0b01111010;
    char Jo      = 0b01110000;
    char Js      = 0b01111000;
    char Jne_Jnz = 0b01110101;
    char Jnl_Jge = 0b01111101;
    char Jnle_Jg = 0b01111111; 
    char Jnb_Jae = 0b01110011; 
    char Jnbe_Ja = 0b01110111;
    char Jnp_Jpo = 0b01111011;
    char Jno     = 0b01110001;
    char Jns     = 0b01111001;
    char Loop    = 0b11100010;
    char Loopz   = 0b11100001;
    char Loopnz  = 0b11100000;
    char Jcxz    = 0b11100011;

    int instruction_index       = 0;

    char bottom_three_bits_mask = 7;
    while(instruction_index < file_size)
    {
        int index_counter = 0;
        Mov_Type mov_type;
        if((ch[instruction_index] >> 2) == (mov_mask >> 2))
        {
            printf("MOV ");
            mov_type = Mov_RegMov;
        }
        else if((ch[instruction_index] >> 1) == (mem_immediate_mov_mask >> 1))
        {
            printf("MOV ");
            mov_type = Mov_ImmediateMem;
        }
        else if((ch[instruction_index] >> 4) == (reg_immediate_mov_mask >> 4))
        {
            printf("MOV ");
            mov_type = Mov_ImmediateReg; 
        }
        else if((ch[instruction_index] >> 1) == (mem_to_accum_mov_mask >> 1))
        {
            printf("MOV ");
            mov_type = Mov_MemToAccumulator;
        }
        else if((ch[instruction_index] >> 1) == (accum_to_mem_mov_mask >> 1))
        {
            printf("MOV ");
            mov_type = Mov_AccumulatorToMem;
        }
        else if((ch[instruction_index] >> 2) == 0)
        {
            printf("ADD ");
            mov_type = Add_RegMov;
        }
        else if((ch[instruction_index] >> 2) == (add_sub_cmp_mask >> 2))
        {
            if(((ch[instruction_index+1] >> 3) & bottom_three_bits_mask) == add_mask)
            {
                printf("ADD ");
                mov_type = Add_ImmediateMem;
            }
            else if(((ch[instruction_index+1] >> 3) & bottom_three_bits_mask) == sub_mask)
            {
                printf("SUB ");
                mov_type = Add_ImmediateMem;
            }
            else if(((ch[instruction_index+1] >> 3) & bottom_three_bits_mask) == cmp_mask)
            {
                printf("CMP ");
                mov_type = Add_ImmediateMem;
            }
            else 
            {
                printf("Something went horribly wrong");
            }
        }
        else if((ch[instruction_index] >> 1) == (immediate_to_accum_add_mask >> 1))
        {
            printf("ADD ");
            mov_type = Add_ImmediateAccum;
        }
        else if((ch[instruction_index] >> 2) == (reg_sub_mask >> 2))
        {
            printf("SUB ");
            mov_type = Add_RegMov;
        }
        else if((ch[instruction_index] >> 1) == (immediate_from_accum_sub_mask >> 1))
        {
            printf("SUB ");
            mov_type = Add_ImmediateAccum;
        }
        else if((ch[instruction_index] >> 2) == (reg_cmp_mask >> 2))
        {
            printf("CMP ");
            mov_type = Add_RegMov;
        }
        else if((ch[instruction_index] >> 1) == (immediate_with_accum >> 1))
        {
            printf("CMP ");
            mov_type = Add_ImmediateAccum;
        }
        else if(ch[instruction_index] == Je_Jz)
        {
            printf("JE ");
            mov_type = Jmp;
        }
        else if(ch[instruction_index] == Jl_Jnge)
        {
            printf("JL ");
            mov_type = Jmp;
        }
        else if(ch[instruction_index] == Jle_Jng)
        {
            printf("JLE ");
            mov_type = Jmp;
        }
        else if(ch[instruction_index] == Jb_Jnae)
        {
            printf("JB ");
            mov_type = Jmp;
        }
        else if(ch[instruction_index] == Jbe_Jna)
        {
            printf("JBE ");
            mov_type = Jmp;
        }
        else if(ch[instruction_index] == Jp_Jpe)
        {
            printf("JP ");
            mov_type = Jmp;
        }
        else if(ch[instruction_index] == Jo)
        {
            printf("JO ");
            mov_type = Jmp;
        }
        else if(ch[instruction_index] == Js)
        {
            printf("JS ");
            mov_type = Jmp;
        }
        else if(ch[instruction_index] == Jne_Jnz)
        {
            printf("JNE ");
            mov_type = Jmp;
        }
        else if(ch[instruction_index] == Jnl_Jge)
        {
            printf("JNL ");
            mov_type = Jmp;
        }
        else if(ch[instruction_index] == Jnle_Jg)
        {
            printf("JNLE ");
            mov_type = Jmp;
        }
        else if(ch[instruction_index] == Jnb_Jae)
        {
            printf("JNB ");
            mov_type = Jmp;
        }
        else if(ch[instruction_index] == Jnbe_Ja)
        {
            printf("JNBE ");
            mov_type = Jmp;
        }
        else if(ch[instruction_index] == Jnp_Jpo)
        {
            printf("JNP ");
            mov_type = Jmp;
        }
        else if(ch[instruction_index] == Jno)
        {
            printf("JNO ");
            mov_type = Jmp;
        }
        else if(ch[instruction_index] == Jns)
        {
            printf("JNS ");
            mov_type = Jmp;
        }
        else if(ch[instruction_index] == Loop)
        {
            printf("LOOP ");
            mov_type = Jmp;
        }
        else if(ch[instruction_index] == Loopz)
        {
            printf("LOOPZ ");
            mov_type = Jmp;
        }
        else if(ch[instruction_index] == Loopnz)
        {
            printf("LOOPNZ ");
            mov_type = Jmp;
        }
        else if(ch[instruction_index] == Jcxz)
        {
            printf("JCXZ ");
            mov_type = Jmp;
        }
        else 
        {
            printf("Something went horribly wrong!");
        }

        char w_mask = mov_type == Mov_ImmediateReg ? 8 : 1;
        char d_mask = 2;

        char registers[MAX_REGISTERS][REGISTER_CHAR_LENGTH] = {"AL", "CL", "DL", "BL", "AH", "CH", "DH", "BH",
                                                               "AX", "CX", "DX", "BX", "SP", "BP", "SI", "DI"};

        char *reg;

        // If it's not an immediate mov we have to check the mod bits 
        // to figure out what kind of MOV we are going to do, and how 
        // to inteprate the following bytes.
        const char mem_mode   = 0b00000000;
        const char mem_mode8  = 0b01000000;
        const char mem_mode16 = 0b10000000;
        const char reg_mode   = 0b11000000;

        char mod_mask = 0b11000000;
        char mod = ch[instruction_index+1] & mod_mask;

        int bytes_before_value;
        int bytes_before_next_instruction;

        if(mov_type == Mov_ImmediateReg || mov_type == Add_ImmediateAccum)
        {
            reg = SetRegister(ch, instruction_index, registers, mov_type, false);

            PrintRegister(reg);

            if(ch[instruction_index] & w_mask)
            {
                int16_t displacement = GetWordValue(ch, instruction_index);
                printf("%d\n", displacement);

                index_counter += 3;
            }
            else  
            {
                printf("%d\n", ch[instruction_index+1]);

                index_counter += 2;
            }
        }
        else if (mov_type == Mov_ImmediateMem || mov_type == Add_ImmediateMem)
        {
            switch(mod)
            {
                case reg_mode:
                {
                    reg = SetRegister(ch, instruction_index, registers, mov_type, false);

                    PrintRegister(reg);

                    printf("%d\n", ch[instruction_index+2]);

                    index_counter += 3;
                } break;
                case mem_mode:
                {

#if 1
                    if(mov_type == Add_ImmediateMem && ((ch[instruction_index+1] & bottom_three_bits_mask) == 0b110))
                    {
                        int index_offset = instruction_index + 1;
                        int16_t value = GetWordValue(ch, index_offset);
                        printf("[%d] WORD %d\n", value, ch[instruction_index+4]);

                        index_counter += 5;
                    }
                    else 
#endif
                    {
                        PrintAddressCalculation(ch, instruction_index, bottom_three_bits_mask);

                        bytes_before_value = instruction_index + 1;
                        bytes_before_next_instruction = 3;

                        if(mov_type == Add_ImmediateMem)
                        {
                            uint8_t s_mask = 0b10;
                            CalculateImmediateMemoryValue(ch, instruction_index, &index_counter, s_mask,
                                                          bytes_before_value, bytes_before_next_instruction);
                        }
                        else
                        {
                            CalculateImmediateMemoryValue(ch, instruction_index, &index_counter, w_mask,
                                                          bytes_before_value, bytes_before_next_instruction);
                        }
                    }
                } break;
                case mem_mode8:
                {
                    int8_t address_displacement = ch[instruction_index+2];
                    PrintAddressCalculation(ch, instruction_index, bottom_three_bits_mask, 
                                            address_displacement);

                    bytes_before_value = instruction_index + 2;
                    bytes_before_next_instruction = 4;
                    CalculateImmediateMemoryValue(ch, instruction_index, &index_counter, w_mask,
                                                  bytes_before_value, bytes_before_next_instruction);
                } break;
                case mem_mode16:
                {
                    int index_offset = instruction_index + 1;
                    int16_t address_displacement = GetWordValue(ch, index_offset); 
                    PrintAddressCalculation(ch, instruction_index , bottom_three_bits_mask,
                                            address_displacement);

                    bytes_before_value = index_offset + 2; 
                    bytes_before_next_instruction = 5;

                    if(mov_type == Add_ImmediateMem)
                    {
                        uint8_t s_mask = 0b10;
                        CalculateImmediateMemoryValue(ch, instruction_index, &index_counter, s_mask,
                                                      bytes_before_value, bytes_before_next_instruction);
                    }
                    else 
                    {
                        CalculateImmediateMemoryValue(ch, instruction_index, &index_counter, w_mask,
                                                      bytes_before_value, bytes_before_next_instruction);
                    }
                } break; 
            }
        }
        else if(mov_type == Mov_RegMov || mov_type == Add_RegMov) 
        {
            switch(mod)
            {
                case reg_mode:
                {
                    reg = SetRegister(ch, instruction_index, registers, mov_type, true);
                    char *reg_two = SetRegister(ch, instruction_index, registers, mov_type, false);
                    if(ch[instruction_index] & d_mask)
                    {
                        PrintRegister(reg);
                        PrintRegister(reg_two);
                        printf("\n");
                    }
                    else 
                    {
                        PrintRegister(reg_two);
                        PrintRegister(reg);
                        printf("\n");
                    }

                    index_counter += 2;
                } break;
                case mem_mode:
                {
                    reg = SetRegister(ch, instruction_index, registers, mov_type, true);

                    int8_t direct_addressing_mode = 0b00000110;
                    bool is_direct_addressing = (ch[instruction_index+1] & bottom_three_bits_mask) ==
                                                 direct_addressing_mode;
                    
                    if(is_direct_addressing)
                    {
                        PrintRegister(reg);
                        int index_offset = instruction_index + 1;
                        int16_t value = GetWordValue(ch, index_offset);
                        printf("[%d]\n", value);
                        
                        index_counter += 4;
                    }
                    else 
                    {
                        if(ch[instruction_index] & d_mask)
                        {
                            PrintRegister(reg);
                            PrintAddressCalculation(ch, instruction_index, bottom_three_bits_mask);
                        }
                        else 
                        {
                            PrintAddressCalculation(ch, instruction_index, bottom_three_bits_mask);
                            PrintRegister(reg);
                        }

                        printf("\n");
                        index_counter += 2;
                    }
                } break;
                case mem_mode8:
                {
                    reg = SetRegister(ch, instruction_index, registers, mov_type, true);

                    if(ch[instruction_index] & d_mask)
                    {
                        PrintRegister(reg);

                        if(!(ch[instruction_index+2]))
                        {
                            PrintAddressCalculation(ch, instruction_index, bottom_three_bits_mask);
                        }
                        else 
                        {
                            int8_t displacement = ch[instruction_index+2]; 
                            PrintAddressCalculation(ch, instruction_index, bottom_three_bits_mask, displacement);
                        }
                    }
                    else 
                    {
                        if(!(ch[instruction_index+2]))
                        {
                            PrintAddressCalculation(ch, instruction_index, bottom_three_bits_mask);
                        }
                        else 
                        {
                            int8_t displacement = ch[instruction_index+2]; 
                            PrintAddressCalculation(ch, instruction_index, bottom_three_bits_mask, displacement);
                        }

                        PrintRegister(reg);
                    }

                    printf("\n");
                    index_counter += 3;
                    
                } break;
                case mem_mode16:
                {
                    reg = SetRegister(ch, instruction_index, registers, mov_type, true);

                    if(ch[instruction_index] & d_mask)
                    {
                        PrintRegister(reg);

                        int index_offset = instruction_index + 1;
                        int16_t displacement = GetWordValue(ch, index_offset);
                        if(displacement == 0)
                        {
                            PrintAddressCalculation(ch, instruction_index, bottom_three_bits_mask);
                        }
                        else 
                        {
                            PrintAddressCalculation(ch, instruction_index, bottom_three_bits_mask, displacement);
                        }
                    }
                    else 
                    {
                        int index_offset = instruction_index + 1;
                        int16_t displacement = GetWordValue(ch, index_offset);
                        if(displacement == 0)
                        {
                            PrintAddressCalculation(ch, instruction_index, bottom_three_bits_mask);
                        }
                        else 
                        {
                            PrintAddressCalculation(ch, instruction_index, bottom_three_bits_mask, displacement);
                        }

                        PrintRegister(reg);
                    }

                    printf("\n");
                    index_counter += 4;
                } break;
                default:
                {
                    index_counter += 2;
                }
            }
        }
        else if(mov_type == Mov_MemToAccumulator)
        {
            reg = &registers[8][0];

            PrintRegister(reg);
            int16_t value = GetWordValue(ch, instruction_index);

            printf("[%d] ", value);
            printf("\n");
            index_counter += 3;
        }
        else if(mov_type ==  Mov_AccumulatorToMem)
        {
            reg = &registers[8][0];

            int16_t value = GetWordValue(ch, instruction_index);
            printf("[%d] ", value);

            PrintRegister(reg);
            printf("\n");
            index_counter += 3;
        }
        else if(mov_type == Jmp)
        {
            printf("%d\n", ch[instruction_index+1]);
            index_counter += 2;
        }
        else
        {
            printf("Something went horribly horribly wrong!");
        }

        instruction_index += index_counter;
    }

#endif
}
#endif
