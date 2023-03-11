#include <stdio.h>
#include <cstdint>

#define MAX_BUFFER_SIZE 256
#define MAX_REGISTERS 16
#define REGISTER_CHAR_LENGTH 3
#define MAX_INSTRUCTIONS 40

char *SetRegister(char *ch, int index, int bottom_three_bits_mask, char w_mask, 
                  char registers[MAX_REGISTERS][REGISTER_CHAR_LENGTH],
                  bool isImmediate, bool is_register_one)
{
    char *result;
    char temp;
    if(isImmediate)
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

int main()
{
    FILE *file;
    char ch[MAX_BUFFER_SIZE] = {};

    file = fopen("more_movs", "rb");

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
    char mov_mask = 0b10001000;
    char immediate_reg_mov_mask = 0b10110000;
    int instruction_index = 0;
    while(instruction_index < MAX_INSTRUCTIONS)
    {
        bool isImmediate = false;
        if((ch[instruction_index] >> 2) == (mov_mask >> 2))
        {
            printf("MOV ");
        }
        else if((ch[instruction_index] >> 4) == (immediate_reg_mov_mask >> 4))
        {
            printf("MOV ");
            isImmediate = true;
        }
        else 
        {
            printf("Something went horribly wrong!");
        }

        char w_mask = isImmediate ? 8 : 1;
        char d_mask = 2;

        char bottom_three_bits_mask = 7;

        char registers[MAX_REGISTERS][REGISTER_CHAR_LENGTH] = {"AL", "CL", "DL", "BL", "AH", "CH", "DH", "BH",
                                                               "AX", "CX", "DX", "BX", "SP", "BP", "SI", "DI"};

        char *reg;
        if(isImmediate)
        {
            reg = SetRegister(ch, instruction_index, bottom_three_bits_mask,
                              w_mask, registers, isImmediate, false);

            PrintRegister(reg);

            if(ch[instruction_index] & w_mask)
            {
                int16_t temp = ((ch[instruction_index+2] & 0xFF) << 8) | (ch[instruction_index+1] & 0xFF);
                printf("%d\n", temp);

                instruction_index += 3;
            }
            else  
            {
                printf("%d\n", ch[instruction_index+1]);

                instruction_index += 2;
            }
        }
        else 
        {
            char reg_mode   = 0b11000000;
            char mem_mode   = 0b00000000;
            char mem_mode8  = 0b01000000;
            char mem_mode16 = 0b10000000;

            char mod_mask = 0b11000000;
            char mod = ch[instruction_index+1] & mod_mask;

            if(mod == reg_mode)
            {
                reg = SetRegister(ch, instruction_index, bottom_three_bits_mask, 
                                  w_mask, registers, isImmediate, true);
                char *reg_two = SetRegister(ch, instruction_index, bottom_three_bits_mask, 
                                            w_mask, registers, isImmediate, false);
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
            }
            else if(mod == mem_mode)
            {
                reg = SetRegister(ch, instruction_index, bottom_three_bits_mask,
                                  w_mask, registers, isImmediate, true);

                PrintRegister(reg);

                switch(ch[instruction_index+1] & bottom_three_bits_mask)
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

                printf("\n");

                instruction_index += 2;
            }
            else if(mod == mem_mode8)
            {
                reg = SetRegister(ch, instruction_index, bottom_three_bits_mask,
                                  w_mask, registers, isImmediate, true);

                PrintRegister(reg);

                if(!(ch[instruction_index+2]))
                {
                    switch(ch[instruction_index+1] & bottom_three_bits_mask)
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
                else 
                {
                    switch(ch[instruction_index+1] & bottom_three_bits_mask)
                    {
                        case 0b000: printf("[BX + SI + %d] ", ch[instruction_index+2]); break;
                        case 0b001: printf("[BX + DI + %d] ", ch[instruction_index+2]); break;
                        case 0b010: printf("[BP + SI + %d] ", ch[instruction_index+2]); break;
                        case 0b011: printf("[BP + DI + %d] ", ch[instruction_index+2]); break;
                        case 0b100: printf("[SI + %d] ", ch[instruction_index+2]);      break;
                        case 0b101: printf("[DI + %d] ", ch[instruction_index+2]);      break;
                        case 0b110: printf("[BP + %d] ", ch[instruction_index+2]);      break;
                        case 0b111: printf("[BX + %d] ", ch[instruction_index+2]);      break;
                    }
                }

                printf("\n");
                instruction_index += 3;
                
            }
            else if(mod == mem_mode16)
            {
                reg = SetRegister(ch, instruction_index, bottom_three_bits_mask,
                                  w_mask, registers, isImmediate, true);

                PrintRegister(reg);

                int16_t displacement = ((ch[instruction_index+3] & 0xFF) << 8) | (ch[instruction_index+2] & 0xFF);
                if(displacement == 0)
                {
                    switch(ch[instruction_index+1] & bottom_three_bits_mask)
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
                else 
                {
                    switch(ch[instruction_index+1] & bottom_three_bits_mask)
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

                printf("\n");
                instruction_index += 4;
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
