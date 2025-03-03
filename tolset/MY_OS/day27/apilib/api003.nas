[FORMAT "WCOFF"]				; 生成对象文件的模式	
[INSTRSET "i486p"]				; 使用 486 兼容模式
[BITS 32]						; 使用 32 位模式机器语言
[FILE "api003.nas"]				; 源文件信息名

    GLOBAL _api_putstr1

[SECTION .text]
_api_putstr1:  ; void api_putstr1(char *s, int l);
    PUSH EBX
	MOV	EDX, 3
	MOV	EBX, [ESP + 8]	; S
    MOV ECX, [ESP + 12]
	INT	0x40
	POP	EBX
	RET