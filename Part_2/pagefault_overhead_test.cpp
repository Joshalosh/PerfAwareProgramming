
static void WriteToAllBytes(Repetition_Tester *tester, Read_Parameters *params) {
    while (IsTesting(tester)) {
        File_Content dest_buffer = params->dest;
        HandleAllocation(tester, params, &dest_buffer);

        BeginTime(tester);
        for (u64 index = 0; index < dest_buffer.size; ++index) {
            dest_buffer.data[index] = (u8)index;
        }
        EndTime(tester);

        CountBytes(tester, dest_buffer.size);

        HandleDeallocation(params, &dest_buffer);
    }
}
