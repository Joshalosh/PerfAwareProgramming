
#define MAX_BUFFER_SIZE 256
#define MAX_REGISTERS 16
#define REGISTER_CHAR_LENGTH 3

void InitInstructionInfo(Instruction_Info *info, char *ch, int instruction_index,
                         Instruction *instruction_table, Instruction_Type instruction_type)
{
    info->opcode          = ch[instruction_index] & instruction_table[instruction_type].op_mask;
    info->d_bit           = ch[instruction_index] & instruction_table[instruction_type].d_mask;
    info->s_bit           = ch[instruction_index] & instruction_table[instruction_type].s_mask;
    info->w_bit           = ch[instruction_index] & instruction_table[instruction_type].w_mask;
    info->mod             = (ch[instruction_index+1] & instruction_table[instruction_type].mod_mask) >> 6;
    info->rm              = ch[instruction_index+1] & instruction_table[instruction_type].rm_mask;
    info->mid_bits        = instruction_table[instruction_type].mid_bits;
    info->op_name         = instruction_table[instruction_type].op_name;
    info->is_immediate    = instruction_table[instruction_type].is_immediate;
    info->is_arithmetic   = instruction_table[instruction_type].is_arithmetic;
    info->arithmetic_type = instruction_table[instruction_type].arithmetic_type;

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

// I need to figure out a way to map these registers 
// to the actual register array.
char *mod_registers[8] = {"bx + si", "bx + di", "bp + si", "bp + di", "si", "di", "bp", "bx"};

u8 mem_map[8] = {0x63, 0x73, 0x65, 0x75, 0x06, 0x07, 0x05, 0x03};    

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
    s16 result = 0;
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

void SimulateRegisters(Instruction_Info instruction_info, Flags *flags, u8 reg_type, 
                       s16 *register_map, s16 value)
{
    s16 new_reg_value = 0;
    s16 sub_value = value;
    switch (instruction_info.arithmetic_type) {
        case ArithType_None: 
        {
            new_reg_value = value;
        } break;

        case ArithType_Add:
        {
            new_reg_value = register_map[reg_type] + value;
        } break;

        case ArithType_Cmp:
        case ArithType_Sub:
        {
            sub_value = ~value + 1;
            new_reg_value  = register_map[reg_type] + sub_value;
        } break;

        default:
        {
            printf("Something has gone real bad boo boo baby");
        } break;
    }

    if(instruction_info.is_arithmetic) {
        for(int index = 0; index < ARRAY_COUNT(flags->flag_array); index++) {
            flags->flag_array[index] = 0;
        }

        if(new_reg_value == 0) {
            flags->zero = 1;
        }
        if(new_reg_value & (1 << 15)) {
            flags->sign = 1;
        }
        if((register_map[reg_type] & 0x8000) == (sub_value & 0x8000) &&
           (register_map[reg_type] & 0x8000) != (new_reg_value & 0x8000) &&
           (sub_value & 0x8000) != (new_reg_value & 0x8000))
        {
           flags->overflow = 1;
        }

        int parity_count = 0;
        for(int i = 0; i < 8; i++)
        {
            if (new_reg_value & (1 << i)) {
                parity_count++;
            }
        }
        if (parity_count % 2 == 0) {
            flags->parity = 1;
        }

        flags->carry = instruction_info.arithmetic_type == ArithType_Sub ? 
                       (u16)new_reg_value > (u16)register_map[reg_type] : 
                       (u16)new_reg_value < (u16)register_map[reg_type];

        //Auxiliary carry checking.
        u8 new_reg_lo_nibble = new_reg_value & 0b0000'1111;
        u8 new_reg_hi_nibble = new_reg_value & 0b1111'0000;
        u8 old_reg_lo_nibble = register_map[reg_type] & 0b0000'1111;
        u8 old_reg_hi_nibble = register_map[reg_type] & 0b1111'0000;

        flags->aux_carry = instruction_info.arithmetic_type == ArithType_Sub ? 
                           new_reg_hi_nibble > old_reg_hi_nibble : 
                           new_reg_lo_nibble < old_reg_lo_nibble;

    }

    if(instruction_info.arithmetic_type != ArithType_Cmp)
    {
        register_map[reg_type] = new_reg_value;
    }
}

#if 1
void SimulateMemory(Instruction_Info instruction_info, Flags *flags, u8 *memory, s16 value, s16 displacement)
{
    s16 new_mem_value = 0;
    s16 sub_value = value;
    switch (instruction_info.arithmetic_type) {
        case ArithType_None: 
        {
            new_mem_value = value;
        } break;

        case ArithType_Add:
        {
            new_mem_value = memory[displacement] + value;
        } break;

        case ArithType_Cmp:
        case ArithType_Sub:
        {
            sub_value = ~value + 1;
            new_mem_value  = memory[displacement] + sub_value;
        } break;

        default:
        {
            printf("Something has gone real bad boo boo baby");
        } break;
    }

    if(instruction_info.is_arithmetic) {
        for(int index = 0; index < ARRAY_COUNT(flags->flag_array); index++) {
            flags->flag_array[index] = 0;
        }

        if(new_mem_value == 0) {
            flags->zero = 1;
        }
        if(new_mem_value & (1 << 15)) {
            flags->sign = 1;
        }
        if((memory[displacement] & 0x8000) == (sub_value & 0x8000) &&
           (memory[displacement] & 0x8000) != (new_mem_value & 0x8000) &&
           (sub_value & 0x8000) != (new_mem_value & 0x8000))
        {
           flags->overflow = 1;
        }

        int parity_count = 0;
        for(int i = 0; i < 8; i++)
        {
            if (new_mem_value & (1 << i)) {
                parity_count++;
            }
        }
        if (parity_count % 2 == 0) {
            flags->parity = 1;
        }

        flags->carry = instruction_info.arithmetic_type == ArithType_Sub ? 
                       (u16)new_mem_value > (u16)memory[displacement] : 
                       (u16)new_mem_value < (u16)memory[displacement];

        //Auxiliary carry checking.
        u8 new_reg_lo_nibble = new_mem_value & 0b0000'1111;
        u8 new_reg_hi_nibble = new_mem_value & 0b1111'0000;
        u8 old_reg_lo_nibble = memory[displacement] & 0b0000'1111;
        u8 old_reg_hi_nibble = memory[displacement] & 0b1111'0000;

        flags->aux_carry = instruction_info.arithmetic_type == ArithType_Sub ? 
                           new_reg_hi_nibble > old_reg_hi_nibble : 
                           new_reg_lo_nibble < old_reg_lo_nibble;

    }

    if(instruction_info.arithmetic_type != ArithType_Cmp)
    {
        memory[displacement] = new_mem_value;
    }
}
#endif

void PrintImmediateMemModeOperations(Instruction_Info instruction_info, char *ch, char *mod_registers[8],
                                     int *instruction_index, s16 bytes_to_displacement, u8 *memory, 
                                     Flags *flags, u8 *mem_map, s16 *register_map)
{
    s16 immediate_displacement = 0;
    s16 reg_displacement = 0;
    u8 reg_index_mask = 0xF;
    u8 reg_index = mem_map[instruction_info.rm] & reg_index_mask;
    u8 second_reg_index = mem_map[instruction_info.rm] & (reg_index_mask << 4);
    
    reg_displacement = second_reg_index ? 
                       register_map[reg_index] + register_map[second_reg_index] :
                       register_map[reg_index];

    s16 displacement = 0;
    printf("[");
    if (instruction_info.rm != 6) {
        PrintRM(instruction_info, mod_registers);
        displacement = reg_displacement;
    }
    if (bytes_to_displacement) {
        immediate_displacement = CalculateWord(ch, *instruction_index, bytes_to_displacement);
        PrintDisplacement(immediate_displacement);
    }
    printf("], ");

    if (instruction_info.w_bit && !instruction_info.s_bit) {
        printf("word ");

        int value_offset = bytes_to_displacement + 2;
        s16 value        = CalculateWord(ch, *instruction_index, value_offset);
        printf("%d", value);

        *instruction_index += 4 + bytes_to_displacement;

        displacement += immediate_displacement;
        SimulateMemory(instruction_info, flags, memory, value, displacement);

    } else {
        char *string = (instruction_info.w_bit) ? "word" : "byte";
        s16 value    = ch[(*instruction_index) + 2 + bytes_to_displacement];
        printf("%s %d", string, value);

        *instruction_index += 3 + bytes_to_displacement;

        displacement += immediate_displacement;
        SimulateMemory(instruction_info, flags, memory, value, displacement);
    }
}

void PrintImmediateMemModeOperations8(Instruction_Info instruction_info, char *ch, char *mod_registers[8],
                                        int *instruction_index, s8 bytes_to_displacement,
                                        s16 *register_map, u8 *memory, Flags *flags)
{
    s16 immediate_displacement = 0;
    s16 reg_displacement = 0;
    u8 reg_index_mask = 0xF;
    u8 reg_index = mem_map[instruction_info.rm] & reg_index_mask;
    u8 second_reg_index = mem_map[instruction_info.rm] & (reg_index_mask << 4);

    reg_displacement = second_reg_index ? 
                       register_map[reg_index] + register_map[second_reg_index] :
                       register_map[reg_index];

    s16 displacement = 0;
    printf("[");
    if (instruction_info.rm != 6) {
        PrintRM(instruction_info, mod_registers);
        displacement = reg_displacement;
    }
    if (bytes_to_displacement) {
        immediate_displacement = ch[*instruction_index + bytes_to_displacement];
        PrintDisplacement(immediate_displacement);
    }
    printf("], ");

    s16 memory_index = instruction_info.rm != 6 ? 
        register_map[instruction_info.rm] + displacement : displacement;
    if (instruction_info.w_bit && !instruction_info.s_bit) {
        printf("word ");

        int value_offset = bytes_to_displacement + 1;
        s16 value        = CalculateWord(ch, *instruction_index, value_offset);
        printf("%d", value);

        *instruction_index += 3 + bytes_to_displacement;

        displacement += immediate_displacement;
        SimulateMemory(instruction_info, flags, memory, value, memory_index); 

    } else {
        char *string = (instruction_info.w_bit) ? "word" : "byte";
        s8 bytes_to_value = bytes_to_displacement + 1;
        s8 value = ch[(*instruction_index) + bytes_to_value];
        printf("%s %d", string, value);

        *instruction_index += 2 + bytes_to_displacement;

        displacement += immediate_displacement;
        SimulateMemory(instruction_info, flags, memory, value, memory_index); 
    }
}

void PrintImmediateRegModeOperations(Instruction_Info instruction_info, char *ch, char *reg_registers[2][8],
                                     int *instruction_index, s16 *register_map, Flags *flags)
{
    PrintRM(instruction_info, reg_registers);
    printf(", ");

    if(instruction_info.w_bit && !instruction_info.s_bit)
    {
        int value_offset = 2;
        s16 value        = CalculateWord(ch, *instruction_index, value_offset);
        printf("%d", value);

        *instruction_index += 4;

        //TODO: I'm going to need to create a special simulation function to handle 
        // the different register operations.
        SimulateRegisters(instruction_info, flags, instruction_info.rm, register_map, value);

    } else {
        printf("%d", ch[(*instruction_index) + 2]);

        *instruction_index += 3;

        SimulateRegisters(instruction_info, flags, instruction_info.rm, 
                          register_map, ch[(*instruction_index) - 1]);
    }
}

void PrintRegisterValues(s16 *register_map)
{
    printf("AX ---> %d\n", register_map[0]); 
    printf("CX ---> %d\n", register_map[1]); 
    printf("DX ---> %d\n", register_map[2]); 
    printf("BX ---> %d\n", register_map[3]); 
    printf("SP ---> %d\n", register_map[4]); 
    printf("BP ---> %d\n", register_map[5]); 
    printf("SI ---> %d\n", register_map[6]); 
    printf("DI ---> %d\n", register_map[7]); 
}

void DecodeInstruction(Instruction_Info instruction_info, Instruction_Type instruction_type, char *ch, 
                       int *instruction_index, s16 *register_map, u8 *memory, Flags *flags)
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
                                                        instruction_index, 2, memory, flags,
                                                        mem_map, register_map);

                    } else {
                        int bytes_to_value = 2;
                        s16 value = CalculateWord(ch, *instruction_index, bytes_to_value);
                        PrintRegister(instruction_info, reg_registers);
                        printf(", [");
                        printf("%d", value);
                        printf("]");

                        *instruction_index += 4;
                        SimulateRegisters(instruction_info, flags, instruction_info.reg, 
                                          register_map, memory[value]);
                    }

                } else if (instruction_info.is_immediate) {
                    PrintImmediateMemModeOperations(instruction_info, ch, mod_registers, instruction_index, 0,
                                                    memory, flags, mem_map, register_map);

                } else {
                    PrintMemModeOperations(instruction_info, reg_registers, mod_registers, 0);

                    *instruction_index += 2;
                }
            } break;

            case Mod_MemModeDisp8: {
                if (instruction_info.is_immediate) {
                    PrintImmediateMemModeOperations8(instruction_info, ch, mod_registers,
                                                     instruction_index, 2, register_map, 
                                                     memory, flags);
                                                    

                } else {
                    int bytes_to_displacement = 2;
                    s16 displacement = ch[(*instruction_index) + bytes_to_displacement];
                    PrintMemModeOperations(instruction_info, reg_registers, mod_registers, displacement);

                    *instruction_index += 3;
                }
            } break; 

            case Mod_MemModeDisp16: {
                int bytes_to_displacement = 2;

                if (instruction_info.is_immediate) {
                    PrintImmediateMemModeOperations(instruction_info, ch, mod_registers, instruction_index, 
                                                    bytes_to_displacement, memory, flags, 
                                                    mem_map, register_map);

                } else {

                    s16 displacement = CalculateWord(ch, *instruction_index, bytes_to_displacement);
                    PrintMemModeOperations(instruction_info, reg_registers, mod_registers, displacement);

                    *instruction_index += 4;
                }
            } break;

            case Mod_RegMode: {
                if (!instruction_info.is_immediate) {
                    if (instruction_info.d_bit) {
                        PrintRegister(instruction_info, reg_registers);
                        printf(", ");
                        PrintRM(instruction_info, reg_registers);

                        *instruction_index += 2;
                        SimulateRegisters(instruction_info, flags, instruction_info.reg, register_map, 
                                          register_map[instruction_info.rm]);

                    } else {
                        PrintRM(instruction_info, reg_registers);
                        printf(", ");
                        if(!instruction_info.is_immediate) {
                            PrintRegister(instruction_info, reg_registers);

                            *instruction_index += 2;
                            SimulateRegisters(instruction_info, flags, instruction_info.rm, register_map,
                                              register_map[instruction_info.reg]);
                        }
                    }


                } else {
                    PrintImmediateRegModeOperations(instruction_info, ch, reg_registers, instruction_index, 
                                                    register_map, flags);
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
                    data = CalculateWord(ch, *instruction_index, data_offset);
                    *instruction_index += 3;

                } else {
                    data = ch[(*instruction_index) + 1];
                    *instruction_index += 2;
                }


                SimulateRegisters(instruction_info, flags, instruction_info.reg, register_map, data);

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
                    data = CalculateWord(ch, *instruction_index, data_offset);
                    *instruction_index += 3;

                } else {
                    data = ch[(*instruction_index) + 1];
                    *instruction_index += 2;
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
                    data = CalculateWord(ch, *instruction_index, data_offset);
                    *instruction_index += 3;

                } else {
                    data = ch[(*instruction_index) + 1];
                    *instruction_index += 2;
                }

                if (!instruction_info.is_immediate) {
                    printf("[%d], ", data);

                } else {
                    printf("%d, ", data);
                }

                PrintRegister(instruction_info, reg_registers);
            }break;

            case InstructionType_JmpJNE:
            {
                printf("%d", ch[*instruction_index+1]+2);
                if (!flags->zero) {
                    *instruction_index += ch[(*instruction_index)+1];
                    *instruction_index += 2;

                } else {
                    *instruction_index += 2;
                }
            } break;
            case InstructionType_JmpJE:
            {
                printf("%d", ch[*instruction_index+1]+2);
                if (flags->zero) {
                    *instruction_index += ch[(*instruction_index)+1];
                    *instruction_index += 2;

                } else {
                    *instruction_index += 2;
                }
            } break;
            case InstructionType_JmpJP:
            {
                printf("%d", ch[*instruction_index+1]+2);
                if (flags->parity) {
                    *instruction_index += ch[(*instruction_index)+1];
                    *instruction_index += 2;

                } else {
                    *instruction_index += 2;
                }
            } break;
            case InstructionType_JmpJL:
            case InstructionType_JmpJLE:
            case InstructionType_JmpJB:
            case InstructionType_JmpJBE:
            case InstructionType_JmpJO:
            case InstructionType_JmpJS:
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
                printf("%d", ch[(*instruction_index)+1]);
                *instruction_index += 2;
            } break;

            default:
            {
                printf("Something has seriously gone to shit");
            } break;
        }
    }
}

