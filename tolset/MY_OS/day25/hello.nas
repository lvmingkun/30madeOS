[FORMAT "WCOFF"]
[INSTRSET "i486p"]
[BITS 32]
[FILE "hello.nas"]

    GLOBAL _HariMain, _putloop, _fin

[section .text]

_HariMain:
    MOV ECX, msg
    MOV EDX, 1

_putloop:
    MOV AL, [CS:ECX]
    CMP AL, 0
    JE _fin
    int 0x40
    ADD ECX, 1
    JMP _putloop

_fin:
    MOV EDX, 4
    INT 0x40

[section .data]

msg:
   DB "FH-OS", 0

