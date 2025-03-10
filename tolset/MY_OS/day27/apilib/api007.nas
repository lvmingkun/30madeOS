[FORMAT "WCOFF"]				; 生成对象文件的模式	
[INSTRSET "i486p"]				; 使用 486 兼容模式
[BITS 32]						; 使用 32 位模式机器语言
[FILE "api007.nas"]				; 源文件信息名

    GLOBAL _api_boxfilwin

[SECTION .text]
_api_boxfilwin:	; void api_boxfilwin(int win, int x0, int y0, int x1, int y1, int col);
	PUSH EDI
	PUSH ESI
	PUSH EBP
	PUSH EBX
	MOV	EDX, 7
	MOV	EBX, [ESP + 20]	; win
	MOV	EAX, [ESP + 24]	; x0
	MOV	ECX, [ESP + 28]	; y0
	MOV	ESI, [ESP + 32]	; x1
	MOV	EDI, [ESP + 36]	; y1
	MOV	EBP, [ESP + 40]	; col
	INT	0x40
	POP	EBX
	POP	EBP
	POP	ESI
	POP	EDI
	RET