[FORMAT "WCOFF"]
[INSTRSET "i486p"]
[BITS 32]
[FILE "hello2.nas"]

    GLOBAL _HariMain
    
[section .text]

_HariMain:
	MOV	EDX, 2
	MOV	EBX, msg
	INT	0x40
	MOV EDX, 4
    INT 0x40

[section .data]
msg:
	DB "FH-OS", 0
