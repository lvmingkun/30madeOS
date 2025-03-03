[FORMAT "WCOFF"]				; 生成对象文件的模式	
[INSTRSET "i486p"]				; 使用 486 兼容模式
[BITS 32]						; 使用 32 位模式机器语言
[FILE "api017.nas"]				; 源文件信息名

    GLOBAL _api_inittimer

[SECTION .text]
_api_inittimer:		; void api_inittimer(int timer, int data);
	PUSH EBX
	MOV	EDX, 17
	MOV	EBX, [ESP + 8]		; timer
	MOV	EAX, [ESP + 12]		; data
	INT	0x40
	POP	EBX
	RET
