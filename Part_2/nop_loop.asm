
global MOVAllBytesASM
global NOPAllBytesASM
global CMPAllBytesASM
global DECAllBytesASM

section .text

; NOTE: These ASM routines are written for the windows
; 64-bit ABI. They expect RCX to be the first parameter (the count),
; and in the case of MOVAllBytesASM, RDX to be the 
; second parameter (the data pointer). To use these on a platform
; with a different ABI, we would have to change those registers
; to match the ABI

MOVAllBytesASM:
    xor rax, rax
.loop:
    mov [rdx + rax], al
    inc rax
    cmp rax, rcx 
    jb .loop 
    ret

NOPAllBytesASM:
    xor rax, rax
.loop:
    db 0x0f, 0x1f, 0x00 ; NOTE: This is the byte sequence for a 3-byte NOP
    inc rax
    cmp rax, rcx
    jb .loop
    ret

CMPAllBytesASM:
    xor rax, rax
.loop:
    inc rax
    cmp rax, rcx
    jb .loop
    ret

DECAllBytesASM:
.loop:
    dec rcx
    jnz .loop
    ret
