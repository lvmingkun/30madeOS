[FORMAT "WCOFF"]				; 生成对象文件的模式	
[INSTRSET "i486p"]				; 使用 486 兼容模式
[BITS 32]						; 使用 32 位模式机器语言
[FILE "api019.nas"]				; 源文件信息名

    GLOBAL _api_freetimer

[SECTION .text]
_api_freetimer:		; void api_freetimer(int timer);
	PUSH EBX
	MOV	EDX, 19
	MOV	EBX, [ESP + 8]		; timer
	INT	0x40
	POP	EBX
	RET