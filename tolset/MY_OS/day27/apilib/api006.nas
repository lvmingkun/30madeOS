[FORMAT "WCOFF"]				; 生成对象文件的模式	
[INSTRSET "i486p"]				; 使用 486 兼容模式
[BITS 32]						; 使用 32 位模式机器语言
[FILE "api006.nas"]				; 源文件信息名

    GLOBAL _api_putstrwin

[SECTION .text]
_api_putstrwin:	; void api_putstrwin(int win, int x, int y, int col, int len, char *str);
	PUSH EDI
	PUSH ESI
	PUSH EBP
	PUSH EBX
	MOV	EDX, 6
	MOV	EBX, [ESP + 20]	; win
	MOV	ESI, [ESP + 24]	; x
	MOV	EDI, [ESP + 28]	; y
	MOV	EAX, [ESP + 32]	; col
	MOV	ECX, [ESP + 36]	; len
	MOV	EBP, [ESP + 40]	; str
	INT	0x40
	POP	EBX
	POP	EBP
	POP	ESI
	POP	EDI
	RET