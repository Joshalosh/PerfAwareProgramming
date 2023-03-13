#include <stdio.h>
#include <stdint.h>

#define MAX_BUFFER_SIZE 256
#define MAX_REGISTERS 16
#define REGISTER_CHAR_LENGTH 3
#define MAX_INSTRUCTIONS 40

char *SetRegister(char *ch, int index, char bottom_three_bits_mask, char w_mask, 
                  char registers[MAX_REGISTERS][REGISTER_CHAR_LENGTH],
                  bool isImmediateReg, bool is_register_one)
{
    char *result;
    char temp;
    if(isImmediateReg)
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

int16_t GetWordDisplacement(char *ch, int instruction_index, bool isImmediateReg) 
{
    int16_t result = isImmediateReg ? ((ch[instruction_index+2] & 0xFF) << 8) | (ch[instruction_index+1] & 0xFF) :
                                    ((ch[instruction_index+3] & 0xFF) << 8) | (ch[instruction_index+2] & 0xFF);

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
    switch(addressing_mode)
    {
        case 0b000: printf("[BX + SI + %d] ", displacement); break;
        case 0b001: printf("[BX + DI + %d] ", displacement); break;
        case 0b010: printf("[BP + SI + %d] ", displacement); break;
        case 0b011: printf("[BP + DI + %d] ", displacement); break;
        case 0b100: printf("[SI + %d] ", displacement);      break;
        case 0b101: printf("[DI + %d] ", displacement);      break;
        case 0b110: printf("[BP + %d] ", displacement);      break;
        case 0b111: printf("[BX + %d] ", displacement);      break;
    }
}

enum Mov_Type : char
{
    Mov_RegMov,
    Mov_ImmediateMem,
    Mov_ImmediateReg,
};

int main()
{
    FILE *file;
    char ch[MAX_BUFFER_SIZE] = {};

    file = fopen("challenge", "rb");

    printf("The assembly instructions of this file is: \n");

    if(file != NULL)
    {
        fread(ch, sizeof(ch),1,file);

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
    int instruction_index       = 0;

    while(instruction_index < MAX_INSTRUCTIONS)
    {
        bool isImmediateReg = false;
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
            isImmediateReg = true;
            mov_type = Mov_ImmediateReg; 
        }
        else 
        {
            printf("Something went horribly wrong!");
        }

        char w_mask = isImmediateReg ? 8 : 1;
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

        switch(mov_type)
        {
            case Mov_ImmediateReg:
            {
                reg = SetRegister(ch, instruction_index, bottom_three_bits_mask,
                                  w_mask, registers, isImmediateReg, false);

                PrintRegister(reg);

                if(ch[instruction_index] & w_mask)
                {
                    int16_t displacement = GetWordDisplacement(ch, instruction_index, isImmediateReg);
                    printf("%d\n", displacement);

                    instruction_index += 3;
                }
                else  
                {
                    printf("%d\n", ch[instruction_index+1]);

                    instruction_index += 2;
                }
            } break ;
            case Mov_ImmediateMem:
            {
                switch(mod)
                {
                    case mem_mode:
                    {
                        PrintAddressCalculation(ch, instruction_index, bottom_three_bits_mask);
                        if(ch[instruction_index] & w_mask)
                        {
                            int16_t value = GetWordDisplacement(ch, instruction_index, false);
                            printf("WORD %d\n", value);
                            instruction_index += 4;
                        }
                        else 
                        {
                            int8_t value = ch[instruction_index+2];
                            printf("BYTE %d\n", value);
                            instruction_index += 3;
                        }
                    } break;
                    case mem_mode8:
                    {
                        int8_t displacement = ch[instruction_index+2];
                        PrintAddressCalculation(ch, instruction_index, bottom_three_bits_mask, displacement);
                        if(ch[instruction_index] & w_mask)
                        {

                            uint8_t instruction_index_offset = instruction_index + 2;
                            int16_t value = GetWordDisplacement(ch, instruction_index_offset, true);
                            printf("WORD %d\n", value);
                            instruction_index += 5;
                        }
                        else 
                        {
                            int8_t value = ch[instruction_index+3];
                            printf("BYTE %d\n", value);
                            instruction_index += 4;
                        }
                    } break;
                    case mem_mode16:
                    {
                        int16_t displacement = GetWordDisplacement(ch, instruction_index, false); 
                        PrintAddressCalculation(ch, instruction_index , bottom_three_bits_mask, displacement);
                        if(ch[instruction_index] & w_mask)
                        {
                            uint8_t instruction_index_offset = instruction_index + 3;
                            int16_t value = GetWordDisplacement(ch, instruction_index_offset, true);
                            printf("WORD %d\n", value);
                            instruction_index += 5;
                        }
                        else 
                        {
                            int8_t value = ch[instruction_index+4];
                            printf("BYTE %d\n", value);
                            instruction_index += 6;
                        }
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
                                          w_mask, registers, isImmediateReg, true);
                        char *reg_two = SetRegister(ch, instruction_index, bottom_three_bits_mask, 
                                                    w_mask, registers, isImmediateReg, false);
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

                        instruction_index += 2;
                    } break;
                    case mem_mode:
                    {
                        reg = SetRegister(ch, instruction_index, bottom_three_bits_mask,
                                          w_mask, registers, isImmediateReg, true);

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

                        instruction_index += 2;
                    } break;
                    case mem_mode8:
                    {
                        reg = SetRegister(ch, instruction_index, bottom_three_bits_mask,
                                          w_mask, registers, isImmediateReg, true);

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
                        instruction_index += 3;
                        
                    } break;
                    case mem_mode16:
                    {
                        reg = SetRegister(ch, instruction_index, bottom_three_bits_mask,
                                          w_mask, registers, isImmediateReg, true);

                        if(ch[instruction_index] & d_mask)
                        {
                            PrintRegister(reg);

                            int16_t displacement = GetWordDisplacement(ch, instruction_index, isImmediateReg);
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
                            int16_t displacement = GetWordDisplacement(ch, instruction_index, isImmediateReg);
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
                        instruction_index += 4;
                    } break;
                    default:
                    {
                        instruction_index += 2;
                    }
                }
            } break ;
            default:
            {
                printf("Something went horribly horribly wrong!");
            }
        }
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
