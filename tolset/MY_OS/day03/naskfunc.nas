; naskfunc
; TAB = 4

[FORMAT "WCOFF"]        ; 制作目标文件的模式
[BITS 32]               ; 制作32位模式用的机器语言

; 制作目标文件的信息

[FILE "naskfunc.nas"]   ; 源文件名的信息
        GLOBAL _io_hlt  ; 程序中包含的函数名

; 以下实际函数

[SECTION .text]         ; 目标文件中写了这些后写程序

_io_hlt:
        HLT
        RET