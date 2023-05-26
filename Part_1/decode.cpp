
#define MAX_BUFFER_SIZE 0xFFFF
#define MAX_REGISTERS 16
#define REGISTER_CHAR_LENGTH 3

Instruction_Info InitInstructionInfo(char *memory, int instruction_index,
                         Instruction *instruction_table, Instruction_Type instruction_type)
{
    Instruction_Info info = {};
    info.opcode           = memory[instruction_index] & instruction_table[instruction_type].op_mask;
    info.d_bit            = memory[instruction_index] & instruction_table[instruction_type].d_mask;
    info.s_bit            = memory[instruction_index] & instruction_table[instruction_type].s_mask;
    info.w_bit            = memory[instruction_index] & instruction_table[instruction_type].w_mask;
    info.mod              = (memory[instruction_index+1] & instruction_table[instruction_type].mod_mask) >> 6;
    info.rm               = memory[instruction_index+1] & instruction_table[instruction_type].rm_mask;
    info.mid_bits         = instruction_table[instruction_type].mid_bits;
    info.op_name          = instruction_table[instruction_type].op_name;
    info.is_immediate     = instruction_table[instruction_type].is_immediate;
    info.is_arithmetic    = instruction_table[instruction_type].is_arithmetic;
    info.arithmetic_type  = instruction_table[instruction_type].arithmetic_type;

    info.reg = (instruction_table[instruction_type].reg_on_first_byte) ? 
        memory[instruction_index] & instruction_table[instruction_type].reg_mask :
        (memory[instruction_index+1] & instruction_table[instruction_type].reg_mask) >> 3;

    info.has_second_instruction_byte = instruction_table[instruction_type].has_second_instruction_byte;

    return info;
}

void PrintInstructionType(Instruction_Info instruction_info)
{
    printf("%s ", instruction_info.op_name);
}

char *reg_registers[2][8] = {{"al", "cl", "dl", "bl", "ah", "ch", "dh", "bh"}, 
                             {"ax", "cx", "dx", "bx", "sp", "bp", "si", "di"}};

// I need to figure out a way to map these registers 
// to the actual register array.
// TODO: get rid of these mod registers.
char *mod_registers[8] = {"bx + si", "bx + di", "bp + si", "bp + di", "si", "di", "bp", "bx"};

// This mapping takes the instruction_info.rm as an index and the value at that
// index has the two corresponding register indexes needed for the register map.
// it requires masking to extract as the bottom 4 bits contain one index, and the 
// high 4 bits contains the second. If there is no second register required the top
// 4 bits remain zero. With this you can extract the indexes and feed them into 
// the register_map to gain access to the correct register values.
enum Mem_Map_Values : u8 {
    MemMap_BX_SI = 0x63,
    MemMap_BX_DI = 0x73,
    MemMap_BP_SI = 0x65,
    MemMap_BP_DI = 0x75,
    MemMap_SI    = 0x06,
    MemMap_DI    = 0x07,
    MemMap_BP    = 0x05,
    MemMap_BX    = 0x03,
};
u8 mem_map[8] = {MemMap_BX_SI, MemMap_BX_DI, MemMap_BP_SI, MemMap_BP_DI, 
                 MemMap_SI, MemMap_DI, MemMap_BP, MemMap_BX};    

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


void PrintRegDisplacement(s16 value)
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

s16 CalculateWord(char *memory, int instruction_index, int offset)
{
    s16 result = 0;
    result = ((memory[instruction_index + offset + 1] & 0xFF) << 8) | (memory[instruction_index + offset] & 0xFF);

    return result;
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
        if((register_map[reg_type] & 0x8000) == (sub_value & 0x8000)     &&
           (register_map[reg_type] & 0x8000) != (new_reg_value & 0x8000) &&
           (sub_value & 0x8000)              != (new_reg_value & 0x8000))
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
void SimulateMemory(Instruction_Info instruction_info, Flags *flags, char *memory, s16 value, s16 displacement)
{
    s16 new_mem_value = 0;
    s16 sub_value     = value;
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
            sub_value     = ~value + 1;
            new_mem_value = memory[displacement] + sub_value;
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

s16 GetMemAddress(Instruction_Info *instruction_info, s16 *register_map)
{
    // TODO: Adding some of the cycle counts in this function is probably 
    // the easiest place to do it without having to do these operations again.
    s16 result     = 0;

    u8 mem_displacement = mem_map[instruction_info->rm];

    u8 reg_a_mask  = 0x0F;
    u8 reg_b_mask  = 0xF0;
    u8 reg_a_index = (mem_displacement & reg_a_mask) >> 0;
    u8 reg_b_index = (mem_displacement & reg_b_mask) >> 4;
    
    result = reg_b_index ? 
             register_map[reg_a_index] + register_map[reg_b_index] : 
             register_map[reg_a_index];

    if (!reg_b_index) {
        instruction_info->ea = 5; 
    }

    switch (mem_displacement)
    {
        case MemMap_BP_DI: 
        case MemMap_BX_SI:
        {
            instruction_info->ea = 7;
        } break;

        case MemMap_BP_SI:
        case MemMap_BX_DI:
        {
            instruction_info->ea = 8;
        }
    }

    return result;
}

void PrintImmediateMemModeOperations(Instruction_Info instruction_info, char *memory,
                                     int *instruction_index, s16 bytes_to_displacement,
                                     Flags *flags, s16 *register_map)
{
    // This is all for mapping the rm bits into the right register 
    // from the mem_map.
    s16 displacement           = 0;
    s16 immediate_displacement = 0;
    s16 reg_displacement       = GetMemAddress(&instruction_info, register_map);
   
    // Need to check if the displacement spans 1 byte or 2. 
    // That will effect the calculations of the rest of the instructions.
    Mod_Type mod_type = CheckMod(instruction_info);
    u8 bytes_to_value = mod_type == Mod_MemModeDisp8 ? 1 : 2;

    printf("[");
    // TODO: Definitely need to try and clean up this check 
    // when i'm not feeling sick.
    if (!(instruction_info.rm == 6 && mod_type == 0)) {
        PrintRM(instruction_info, mod_registers);
        displacement = reg_displacement;
    }

    if (bytes_to_displacement) {
        immediate_displacement = mod_type != Mod_MemModeDisp8 ? 
                                 CalculateWord(memory, *instruction_index, bytes_to_displacement) :
                                 memory[*instruction_index + bytes_to_displacement];

        // A different printout will occur if there is a register 
        // TODO: Definitely need to try and clean up this check 
        // when i'm not feeling sick.
        if (!(instruction_info.rm == 6 && mod_type == 0)) {
            PrintRegDisplacement(immediate_displacement);

        } else {
            printf("%d", immediate_displacement);
        }
    }
    displacement += immediate_displacement;
    printf("], ");

    int value_offset         = bytes_to_displacement + bytes_to_value;
    s16 value                = 0;
    int bytes_to_instruction = 1;
    if (instruction_info.w_bit && !instruction_info.s_bit) {
        value = CalculateWord(memory, *instruction_index, value_offset);
        bytes_to_instruction++;

    } else {
        value = memory[(*instruction_index) + value_offset];
    }

    char *string = (instruction_info.w_bit) ? "word" : "byte";
    printf("%s %d", string, value);

    *instruction_index += bytes_to_instruction + bytes_to_displacement + bytes_to_value;
    SimulateMemory(instruction_info, flags, memory, value, displacement);
}

void PrintMemModeOperations(Instruction_Info *instruction_info, char *memory, 
                            s16 *register_map, Flags *flags, s16 displacement)
{
    s16 mem_address = GetMemAddress(instruction_info, register_map);
    mem_address    += displacement;

    // We need to add in the extra clocks if a displacement has to also
    // be added.
    if (displacement) {
        instruction_info->ea += 4;
    }

    if (instruction_info->d_bit) {
        PrintRegister(*instruction_info, reg_registers);
        printf(", [");
        PrintRM(*instruction_info, mod_registers);
        PrintRegDisplacement(displacement);
        printf("]");

        instruction_info->operation_type = OpType_Reg_Mem;
        SimulateRegisters(*instruction_info, flags, instruction_info->reg, 
                          register_map, memory[mem_address]);

    } else {
        printf("[");
        PrintRM(*instruction_info, mod_registers);
        PrintRegDisplacement(displacement);
        printf("], ");
        PrintRegister(*instruction_info, reg_registers);

        instruction_info->operation_type = OpType_Mem_Reg;
        SimulateMemory(*instruction_info, flags, memory, register_map[instruction_info->reg], 
                       mem_address);  
    }
}

void PrintImmediateRegModeOperations(Instruction_Info instruction_info, char *memory, 
                                     int *instruction_index, s16 *register_map, Flags *flags)
{
    PrintRM(instruction_info, reg_registers);
    printf(", ");

    if(instruction_info.w_bit && !instruction_info.s_bit)
    {
        int value_offset = 2;
        s16 value        = CalculateWord(memory, *instruction_index, value_offset);
        printf("%d", value);

        *instruction_index += 4;

        //TODO: I'm going to need to create a special simulation function to handle 
        // the different register operations.
        SimulateRegisters(instruction_info, flags, instruction_info.rm, register_map, value);

    } else {
        printf("%d", memory[(*instruction_index) + 2]);

        *instruction_index += 3;

        SimulateRegisters(instruction_info, flags, instruction_info.rm, 
                          register_map, memory[(*instruction_index) - 1]);
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

void DecodeInstruction(Instruction_Info *instruction_info, Instruction_Type instruction_type, char *memory, 
                       int *instruction_index, s16 *register_map, Flags *flags)
{
    if (instruction_info->has_second_instruction_byte) {
        Mod_Type mod_type = CheckMod(*instruction_info);
        switch (mod_type) { 

            case Mod_MemModeNoDisp: {
                // For some reason there is a funny exeption in this mode where
                // if the RM bits equal 110 then there is a 16 bit value that goes
                // directly into the register.
                if (instruction_info->rm == 0b110) {
                    if (instruction_info->is_immediate) {
                        PrintImmediateMemModeOperations(*instruction_info, memory, 
                                                        instruction_index, 2, flags,
                                                        register_map);
                        instruction_info->operation_type = OpType_Mem_Imm;

                    } else {
                        int bytes_to_value = 2;
                        s16 value          = CalculateWord(memory, *instruction_index, bytes_to_value);
                        PrintRegister(*instruction_info, reg_registers);
                        printf(", [");
                        printf("%d", value);
                        printf("]");

                        *instruction_index   += 4;
                        instruction_info->ea  = 6;
                        instruction_info->operation_type = OpType_Reg_Mem;
                        SimulateRegisters(*instruction_info, flags, instruction_info->reg, 
                                          register_map, memory[value]);
                    }

                } else if (instruction_info->is_immediate) {
                    PrintImmediateMemModeOperations(*instruction_info, memory, instruction_index,
                                                    0, flags, register_map);
                    instruction_info->operation_type = OpType_Mem_Imm;

                } else {
                    PrintMemModeOperations(instruction_info, memory, register_map, flags, 0);

                    *instruction_index += 2;
                }

                CalculateClocks(instruction_info);
            } break;

            case Mod_MemModeDisp8: {
                if (instruction_info->is_immediate) {
                    PrintImmediateMemModeOperations(*instruction_info, memory,
                                                    instruction_index, 2, 
                                                    flags, register_map);
                    instruction_info->operation_type = OpType_Mem_Imm;

                } else {
                    int bytes_to_displacement = 2;
                    s16 displacement          = memory[(*instruction_index) + bytes_to_displacement];
                    PrintMemModeOperations(instruction_info, memory, register_map, 
                                           flags, displacement);

                    *instruction_index += 3;
                }

                CalculateClocks(instruction_info);
            } break; 

            case Mod_MemModeDisp16: {
                int bytes_to_displacement = 2;

                if (instruction_info->is_immediate) {
                    PrintImmediateMemModeOperations(*instruction_info, memory, instruction_index, 
                                                    bytes_to_displacement, flags, 
                                                    register_map);
                    instruction_info->operation_type = OpType_Mem_Imm;

                } else {

                    s16 displacement = CalculateWord(memory, *instruction_index, bytes_to_displacement);
                    PrintMemModeOperations(instruction_info, memory, register_map, 
                                           flags, displacement);

                    *instruction_index += 4;
                }

                CalculateClocks(instruction_info);
            } break;

            case Mod_RegMode: {
                if (!instruction_info->is_immediate) {
                    if (instruction_info->d_bit) {
                        PrintRegister(*instruction_info, reg_registers);
                        printf(", ");
                        PrintRM(*instruction_info, reg_registers);

                        *instruction_index += 2;
                        SimulateRegisters(*instruction_info, flags, instruction_info->reg, register_map, 
                                          register_map[instruction_info->rm]);

                    } else {
                        PrintRM(*instruction_info, reg_registers);
                        printf(", ");
                        if(!instruction_info->is_immediate) {
                            PrintRegister(*instruction_info, reg_registers);

                            *instruction_index += 2;
                            SimulateRegisters(*instruction_info, flags, instruction_info->rm, register_map,
                                              register_map[instruction_info->reg]);
                        }
                    }

                    instruction_info->operation_type = OpType_Reg_Reg;

                } else {
                    PrintImmediateRegModeOperations(*instruction_info, memory, instruction_index, 
                                                    register_map, flags);
                    instruction_info->operation_type = OpType_Reg_Imm;
                }

                CalculateClocks(instruction_info);
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
                    instruction_info->w_bit >>= 3;
                }
                PrintRegister(*instruction_info, reg_registers);
                printf(", ");

                s16 data = 0;
                if (instruction_info->w_bit) {
                    int data_offset = 1;
                    data = CalculateWord(memory, *instruction_index, data_offset);
                    *instruction_index += 3;

                } else {
                    data = memory[(*instruction_index) + 1];
                    *instruction_index += 2;
                }
                printf("%d", data);

                instruction_info->operation_type = OpType_Reg_Imm; 
                CalculateClocks(instruction_info);
                SimulateRegisters(*instruction_info, flags, instruction_info->reg, register_map, data);
            } break;

            case InstructionType_CmpImmediateWithAccum:
            case InstructionType_SubImmediateFromAccum:
            case InstructionType_AddImmediateToAccum:
            case InstructionType_MovMemToAccum: 
            {
                PrintRegister(*instruction_info, reg_registers);
                printf(", ");

                s16 data = 0;

                if (instruction_info->w_bit) {
                    int data_offset = 1;
                    data            = CalculateWord(memory, *instruction_index, data_offset);
                    *instruction_index += 3;

                } else {
                    data = memory[(*instruction_index) + 1];
                    *instruction_index += 2;
                }

                if (!instruction_info->is_immediate) {
                    printf("[%d]", data);
                    instruction_info->operation_type = OpType_Mem_Accum;

                } else {
                    printf("%d", data);
                    instruction_info->operation_type = OpType_Accum_Imm;
                }

                CalculateClocks(instruction_info);
            } break;

            case InstructionType_MovAccumToMem:
            {
                s16 data = 0;
                if (instruction_info->w_bit) {
                    int data_offset = 1;
                    data            = CalculateWord(memory, *instruction_index, data_offset);
                    *instruction_index += 3;

                } else {
                    data = memory[(*instruction_index) + 1];
                    *instruction_index += 2;
                }

                if (!instruction_info->is_immediate) {
                    printf("[%d], ", data);
                    instruction_info->operation_type = OpType_Accum_Mem;

                } else {
                    printf("%d, ", data);
                    instruction_info->operation_type = OpType_Accum_Imm;
                }

                PrintRegister(*instruction_info, reg_registers);
                CalculateClocks(instruction_info);
            }break;

            case InstructionType_JmpJNE:
            {
                printf("%d", memory[*instruction_index+1]+2);
                if (!flags->zero) {
                    *instruction_index += memory[(*instruction_index)+1];
                    *instruction_index += 2;

                } else {
                    *instruction_index += 2;
                }
            } break;
            case InstructionType_JmpJE:
            {
                printf("%d", memory[*instruction_index+1]+2);
                if (flags->zero) {
                    *instruction_index += memory[(*instruction_index)+1];
                    *instruction_index += 2;

                } else {
                    *instruction_index += 2;
                }
            } break;
            case InstructionType_JmpJP:
            {
                printf("%d", memory[*instruction_index+1]+2);
                if (flags->parity) {
                    *instruction_index += memory[(*instruction_index)+1];
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
                printf("%d", memory[(*instruction_index)+1]);
                *instruction_index += 2;
            } break;

            default:
            {
                printf("Something has seriously gone to shit");
            } break;
        }
    }
}

