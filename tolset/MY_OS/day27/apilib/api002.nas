[FORMAT "WCOFF"]				; 生成对象文件的模式	
[INSTRSET "i486p"]				; 使用 486 兼容模式
[BITS 32]						; 使用 32 位模式机器语言
[FILE "api002.nas"]				; 源文件信息名

    GLOBAL _api_putstr0

[SECTION .text]
_api_putstr0:  ; void api_putstr0(char *s);
    PUSH EBX
	MOV	EDX, 2
	MOV	EBX, [ESP + 8]	; S
	INT	0x40
	POP	EBX
	RET