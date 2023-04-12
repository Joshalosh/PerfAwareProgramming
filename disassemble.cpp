#include <stdio.h>
#include <stdint.h>

#include "disassemble.h"

u16 register_map[8] = {};
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

#if 0
    file = fopen("challenge", "rb");
#endif

#if 0
    file = fopen("add_sub_cmp", "rb");
#endif

#if 0 
    file = fopen("immediate_movs", "rb");
#endif

#if 1 
    file = fopen("register_movs", "rb");
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

        // This is a long process for deciding what the instruction type is going
        // to be. Some instructions have an identical first bit so we need to account
        // for that otherwise the first equivalent instruction type is chosen and
        // that might not be the correct one. Due to this every new instruction byte 
        // is checked against ALL the possible instructions. If there are multiple hits
        // then we have to deal with the special case.
        Instruction_Type instruction_type = InstructionType_Count;
        Type_Bucket types = {};
        for (int index = 0; index < InstructionType_Count; index++) {
            if ((ch[instruction_index] & instruction_table[index].op_mask) == instruction_table[index].op_bits) {
                types.array[types.size] = (Instruction_Type)index;
                types.size++;
            }
        }


        if (types.size > 1) {
            for(int index = 0; index < types.size; index++)
            {
                u8 mid_bits_mask = 0b00'111'000; 
                if (instruction_table[types.array[index]].mid_bits == 
                    (ch[instruction_index + 1] & mid_bits_mask)) {
                    instruction_type = types.array[index];
                    break;
                }
            }

        } else {
            instruction_type = types.array[0];
        }

        int bytes_to_next_instruction = 0;

        Instruction_Info instruction_info = {};
        InitInstructionInfo(&instruction_info, ch, instruction_index, instruction_table, instruction_type);

        PrintInstructionType(instruction_info);

        Flags flags = {};
        DecodeInstruction(instruction_info, instruction_type, ch, instruction_index, 
                          &bytes_to_next_instruction, &flags);
        instruction_index += bytes_to_next_instruction;
        printf("\n");
    }

    printf("\n");
    PrintRegisterValues(register_map);
}
