
static void WriteToAllBytesBackward(Repetition_Tester *tester, Read_Parameters *params) {
    while (IsTesting(tester)) {
        File_Content dest_buffer = params->dest;
        HandleAllocation(params, &dest_buffer);

        BeginTime(tester);
        for (u64 index = 0; index < dest_buffer.size; ++index) {
            dest_buffer.data[(dest_buffer.size - 1) - index] = (u8)index;
        }
        EndTime(tester);

        CountBytes(tester, dest_buffer.size);

        HandleDeallocation(params, &dest_buffer);
    }
}
