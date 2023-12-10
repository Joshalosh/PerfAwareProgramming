
global ConditionalNOP

section .text

ConditionalNOP:
    xor rax, rax
.loop:
    mov r10, [rdx, rax]
        inc rax
        test r10, 1
    jnz .skip
        nop
.skip:
    cmp rax, rcx
    jb .loop
    ret
