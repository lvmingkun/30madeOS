[FORMAT "WCOFF"]				; 生成对象文件的模式	
[INSTRSET "i486p"]				; 使用 486 兼容模式
[BITS 32]						; 使用 32 位模式机器语言
[FILE "api018.nas"]				; 源文件信息名

    GLOBAL _api_settimer

[SECTION .text]
_api_settimer:		; void api_settimer(int timer, int time);
	PUSH EBX
	MOV	EDX,18
	MOV	EBX, [ESP + 8]		; timer
	MOV	EAX, [ESP + 12]		; time
	INT	0x40
	POP	EBX
	RET