#include <windows.h>
#include <fcntl.h>
#include <io.h>

struct Read_Parameters {
    File_Content dest;
    char const *filename;
};

typedef void read_overhead_test_func(Repetition_Tester *tester, Read_Parameters *params);

static void ReadViaFRead(Repetition_Tester *tester, Read_Parameters *params) {
    while (IsTesting(tester)) {
        FILE *file = fopen(params->filename, "rb");
        if (file) {
            File_Content dest_buffer = params->dest;

            BeginTime(tester);
            size_t result = fread(dest_buffer.data, dest_buffer.size, 1, file);
            EndTime(tester);

            if (result == 1) {
                CountBytes(tester, dest_buffer.size);
            } else {
                Error(tester, "fread failed");
            }

            fclose(file);
        } else {
            Error(tester, "fopen failed");
        }
    }
}

static void ReadViaRead(Repetition_Tester *tester, Read_Parameters *params) {
    while (IsTesting(tester)) {
        int file = _open(params->filename, _O_BINARY|_O_RDONLY);
        if (file != -1) {
            File_Content dest_buffer = params->dest;

            char *dest = dest_buffer.data;
            u64 size_remaining = dest_buffer.size;
            while (size_remaining) {
                u32 read_size = INT_MAX;
                if ((u64)read_size > size_remaining) {
                    read_size = (u32)size_remaining;
                }

                BeginTime(tester);
                int result = _read(file, dest, read_size);
                EndTime(tester);

                if(result == (int)read_size) {
                    CountBytes(tester, read_size);
                } else {
                    Error(tester, "_read failed");
                    break;
                }

                size_remaining -= read_size;
                dest += read_size;
            }

            _close(file);
        } else {
            Error(tester, "_open failed");
        }
    }
}

static void ReadViaReadFile(Repetition_Tester *tester, Read_Parameters *params) {
    while (IsTesting(tester)) {
        HANDLE file = CreateFileA(params->filename, GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE, 0,
                                  OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
        if(file != INVALID_HANDLE_VALUE) {
            File_Content dest_buffer = params->dest;

            u64 size_remaining = params->dest.size;
            u8 *dest = (u8 *)dest_buffer.data;
            while (size_remaining) {
                u32 read_size = (u32)-1;
                if ((u64)read_size > size_remaining) {
                    read_size = (u32)size_remaining;
                }

                DWORD bytes_read = 0;
                BeginTime(tester);
                BOOL result = ReadFile(file, dest, read_size, &bytes_read, 0);
                EndTime(tester);

                if (result && (bytes_read == read_size)) {
                    CountBytes(tester, read_size);
                } else {
                    Error(tester, "ReadFile failed");
                }

                size_remaining -= read_size;
                dest += read_size;
            }

            CloseHandle(file);
        } else {
            Error(tester, "CreateFileA failed");
        }
    }
}
