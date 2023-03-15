#include <stdio.h>
#include <stdint.h>

#define MAX_BUFFER_SIZE 256
#define MAX_REGISTERS 16
#define REGISTER_CHAR_LENGTH 3

enum Mov_Type : char
{
    Mov_RegMov,
    Mov_ImmediateMem,
    Mov_ImmediateReg,
    Mov_MemToAccumulator,
    Mov_AcculmulatorToMem,
};

char *SetRegister(char *ch, int index, char bottom_three_bits_mask, char w_mask, 
                  char registers[MAX_REGISTERS][REGISTER_CHAR_LENGTH],
                  Mov_Type mov_type, bool is_register_one)
{
    char *result;
    char temp;
    if(mov_type == Mov_ImmediateReg)
    {
        temp = ch[index] & bottom_three_bits_mask;
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
            }
            else 
            {
                result = &registers[reg_index][0];
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
    if(ch[instruction_index] & w_mask)
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

#if 1
    file = fopen("challenge", "rb");
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
    char mov_mask               = 0b10001000;
    char mem_immediate_mov_mask = 0b11000110;
    char reg_immediate_mov_mask = 0b10110000;
    char mem_to_accum_mov_mask  = 0b10100000;
    char accum_to_mem_mov_mask  = 0b10100010;
    int instruction_index       = 0;

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
            mov_type = Mov_AcculmulatorToMem;
        }
        else 
        {
            printf("Something went horribly wrong!");
        }

        char w_mask = mov_type == Mov_ImmediateReg ? 8 : 1;
        char d_mask = 2;
        char bottom_three_bits_mask = 7;

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

        switch(mov_type)
        {
            case Mov_ImmediateReg:
            {
                reg = SetRegister(ch, instruction_index, bottom_three_bits_mask,
                                  w_mask, registers, mov_type, false);

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
            } break ;
            case Mov_ImmediateMem:
            {
                switch(mod)
                {
                    case reg_mode:
                    {
                        printf("This should never get hit");
                        index_counter += 2;
                    } break;
                    case mem_mode:
                    {

                        PrintAddressCalculation(ch, instruction_index, bottom_three_bits_mask);

                        bytes_before_value = instruction_index + 1;
                        bytes_before_next_instruction = 3;
                        CalculateImmediateMemoryValue(ch, instruction_index, &index_counter, w_mask,
                                                      bytes_before_value, bytes_before_next_instruction);
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
                        CalculateImmediateMemoryValue(ch, instruction_index, &index_counter, w_mask,
                                                      bytes_before_value, bytes_before_next_instruction);
                    } break; 
                }
            } break ;
            case Mov_RegMov: 
            {
                switch(mod)
                {
                    case reg_mode:
                    {
                        reg = SetRegister(ch, instruction_index, bottom_three_bits_mask, 
                                          w_mask, registers, mov_type, true);
                        char *reg_two = SetRegister(ch, instruction_index, bottom_three_bits_mask, 
                                                    w_mask, registers, mov_type, false);
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
                        reg = SetRegister(ch, instruction_index, bottom_three_bits_mask,
                                          w_mask, registers, mov_type, true);

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
                        reg = SetRegister(ch, instruction_index, bottom_three_bits_mask,
                                          w_mask, registers, mov_type, true);

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
                        reg = SetRegister(ch, instruction_index, bottom_three_bits_mask,
                                          w_mask, registers, mov_type, true);

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
            } break ;
            case Mov_MemToAccumulator:
            {
                reg = &registers[8][0];

                PrintRegister(reg);
                int16_t value = GetWordValue(ch, instruction_index);

                printf("[%d] ", value);
                printf("\n");
                index_counter += 3;
            } break;
            case Mov_AcculmulatorToMem:
            {
                reg = &registers[8][0];

                int16_t value = GetWordValue(ch, instruction_index);
                printf("[%d] ", value);

                PrintRegister(reg);
                printf("\n");
                index_counter += 3;
            } break;
            default:
            {
                printf("Something went horribly horribly wrong!");
            }
        }

        instruction_index += index_counter;
    }
#endif


#if 0
    for(int index = 0; index < MAX_BUFFER_SIZE; index++)
    {
        unsigned char bit_index = 0b10000000;
        //unsigned int instruction_mask = 
        while(bit_index > 0)
        {
            if(bit_index > 128)
            {
            }
            else if((ch[index] & bit_index) == 0)
            {
                printf("0");
            }
            else 
            {
                printf("1");
            }


            bit_index = bit_index >> 1;
        } 
        printf(" ");
    }
#endif

#if 0
    for(int i = 0; i < MAX_BUFFER_SIZE; i++)
    {
        unsigned char bit_mask = 0b10000000;
        for(int j = 0; j < 8; j++)
        {
            if(((bit_mask >> j) & ch[i]) == 0)
            {
                printf("0");
            }
            else 
            {
                printf("1");
                
            }
        }
        printf(" ");
    }
#endif
}
