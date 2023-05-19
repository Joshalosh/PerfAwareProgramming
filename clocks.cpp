
void CalculateClocks(Instruction_Info *instruction_info)
{
    switch (instruction_info->arithmetic_type)
    {
        case ArithType_None:
        {
            switch (instruction_info->operation_type)
            {
                case OpType_Mem_Accum:
                {
                    instruction_info->clocks += 10;
                } break;

                case OpType_Accum_Mem:
                {
                    instruction_info->clocks += 10;
                } break;

                case OpType_Reg_Reg:
                {
                    instruction_info->clocks += 2;
                } break;

                case OpType_Reg_Mem:
                {
                    instruction_info->clocks += 8;
                } break;

                case OpType_Mem_Reg:
                {
                    instruction_info->clocks += 9;
                } break;

                case OpType_Reg_Imm:
                {
                    instruction_info->clocks += 4;
                } break;

                case OpType_Mem_Imm:
                {
                    instruction_info->clocks += 10;
                } break;
            }
        } break;

        case ArithType_Add:
        {
            switch (instruction_info->operation_type)
            {
                case OpType_Reg_Reg:
                {
                    instruction_info->clocks += 3;
                } break;

                case OpType_Reg_Mem:
                {
                    instruction_info->clocks += 9;
                } break;

                case OpType_Mem_Reg:
                {
                    instruction_info->clocks += 16;
                } break;

                case OpType_Reg_Imm:
                {
                    instruction_info->clocks += 4;
                } break;

                case OpType_Mem_Imm:
                {
                    instruction_info->clocks += 17;
                } break;

                case OpType_Accum_Imm:
                {
                    instruction_info->clocks += 4;
                } break;
            }
        } break;
    }
};
