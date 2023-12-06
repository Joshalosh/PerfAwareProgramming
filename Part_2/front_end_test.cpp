
static void WriteToAllByte(Repetition_Tester *tester, Read_Parameters *params) {
    while (IsTesting(tester)) {
        File_Content dest_buffer = params->dest; 
        HandleAllocation(tester, params, &dest_buffer);

        BeginTime(tester);
        for (u64 index = 0; index < dest_buffer.size; index++) {
            dest_buffer.data[index] = (u8)index;
        }
        EndTime(tester);

        CountBytes(tester, dest_buffer.size);
        HandleDeallocation(params, &dest_buffer);
    }
}

extern "C" void MOVAllBytesASM(u64 Count, char *Data);
extern "C" void NOPAllBytesASM(u64 Count);
extern "C" void CMPAllBytesASM(u64 Count);
extern "C" void DECAllBytesASM(u64 Count);
#pragma comment (lib, "nop_loop");

static void MOVAllBytes(Repetition_Tester *tester, Read_Parameters *params) {
    while (IsTesting(tester)) {
        File_Content dest_buffer = params->dest;
        HandleAllocation(tester, params, &dest_buffer);

        BeginTime(tester);
        MOVAllBytesASM(dest_buffer.size, dest_buffer.data);
        EndTime(tester);

        CountBytes(tester, dest_buffer.size);
        HandleDeallocation(params, &dest_buffer);
    }
}

static void NOPAllBytes(Repetition_Tester *tester, Read_Parameters *params) {
    while (IsTesting(tester)) {
        File_Content dest_buffer = params->dest;
        HandleAllocation(tester, params, &dest_buffer);

        BeginTime(tester);
        NOPAllBytesASM(dest_buffer.size);
        EndTime(tester);

        CountBytes(tester, dest_buffer.size);
        HandleDeallocation(params, &dest_buffer);
    }
}

static void CMPAllBytes(Repetition_Tester *tester, Read_Parameters *params) {
    while (IsTesting(tester)) {
        File_Content dest_buffer = params->dest;
        HandleAllocation(tester, params, &dest_buffer);

        BeginTime(tester);
        CMPAllBytesASM(dest_buffer.size);
        EndTime(tester);

        CountBytes(tester, dest_buffer.size);
        HandleDeallocation(params, &dest_buffer);
    }
}

static void DECAllBytes(Repetition_Tester *tester, Read_Parameters *params) {
    while (IsTesting(tester)) {
        File_Content dest_buffer = params->dest;
        HandleAllocation(tester, params, &dest_buffer);

        BeginTime(tester);
        DECAllBytesASM(dest_buffer.size);
        EndTime(tester);

        CountBytes(tester, dest_buffer.size);
        HandleDeallocation(params, &dest_buffer);
    }
}
