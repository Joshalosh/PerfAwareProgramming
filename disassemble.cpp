#include <stdio.h>

#define MAX_BUFFER_SIZE 256
#define MAX_REGISTERS 16
#define REGISTER_CHAR_LENGTH 3
#define MAX_INSTRUCTIONS 22

char *SetRegister(unsigned char *ch, int index, unsigned int low_mask, unsigned int w_mask, 
                  char registers[MAX_REGISTERS][REGISTER_CHAR_LENGTH], bool is_register_one)
{
    char *result;
    unsigned int temp = is_register_one ? ((ch[index+1] >> 3) & low_mask) : ch[index+1] & low_mask;
    for(int reg_index = 0; reg_index < 8; reg_index++)
    {
        if(!(temp ^ reg_index))
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

void PrintRegisterOrder(char *register_one, char *register_two)
{
    for(int i = 0; i < 3; i++)
    {
        printf("%c", register_one[i]);
    }
    printf(" ");

    for(int i = 0; i < 3; i++)
    {
        printf("%c", register_two[i]);
    }
    printf("\n");
}

int main()
{
    FILE *file;
    unsigned char ch[MAX_BUFFER_SIZE];

    file = fopen("multi_register", "rb");

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

    unsigned int instruction_mask = 0x88;
    int instruction_index = 0;
    while(instruction_index < MAX_INSTRUCTIONS)
    {
        if(!((ch[instruction_index] >> 2) ^ (instruction_mask >> 2)))
        {
            printf("MOV ");
        }
        else 
        {
            printf("Something went horribly wrong!");
        }

        unsigned int w_mask = 1;
        unsigned int d_mask = 2;

        unsigned int low_mask = 7;

        char registers[MAX_REGISTERS][REGISTER_CHAR_LENGTH] = {"AL", "CL", "DL", "BL", "AH", "CH", "DH", "BH",
                                                               "AX", "CX", "DX", "BX", "SP", "BP", "SI", "DI"};

        char *register_one = SetRegister(ch, instruction_index, low_mask, w_mask, registers, true);
        char *register_two = SetRegister(ch, instruction_index, low_mask, w_mask, registers, false);

        if(ch[instruction_index] & d_mask)
        {
            PrintRegisterOrder(register_one, register_two);
        }
        else 
        {
            PrintRegisterOrder(register_two, register_one);
        }
        instruction_index += 2;
    }
}
