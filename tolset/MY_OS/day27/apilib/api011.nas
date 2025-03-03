[FORMAT "WCOFF"]				; 生成对象文件的模式	
[INSTRSET "i486p"]				; 使用 486 兼容模式
[BITS 32]						; 使用 32 位模式机器语言
[FILE "api011.nas"]				; 源文件信息名

    GLOBAL _api_point

[SECTION .text]
_api_point:		; void api_point(int win, int x, int y, int col);
	PUSH EDI
	PUSH ESI
	PUSH EBX
	MOV	EDX, 11
	MOV	EBX, [ESP + 16]		; win
	MOV	ESI, [ESP + 20]		; x
	MOV	EDI, [ESP + 24]		; y
	MOV	EAX, [ESP + 28]		; col
	INT	0x40
	POP	EBX
	POP	ESI
	POP	EDI
	RET