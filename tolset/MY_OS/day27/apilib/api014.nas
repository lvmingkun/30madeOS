[FORMAT "WCOFF"]				; 生成对象文件的模式	
[INSTRSET "i486p"]				; 使用 486 兼容模式
[BITS 32]						; 使用 32 位模式机器语言
[FILE "api014.nas"]				; 源文件信息名

    GLOBAL _api_closewin

[SECTION .text]
_api_closewin:		; void api_closewin(int win)
    PUSH EBX
	MOV	EDX, 14
	MOV	EBX, [ESP + 8]	; Win
	INT 0x40
	POP	EBX
	RET