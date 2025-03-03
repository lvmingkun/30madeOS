[FORMAT "WCOFF"]				; 生成对象文件的模式	
[INSTRSET "i486p"]				; 使用 486 兼容模式
[BITS 32]						; 使用 32 位模式机器语言
[FILE "api020.nas"]				; 源文件信息名

    GLOBAL _api_beep

[SECTION .text]
_api_beep:  		; void api_beep(int tone)
	MOV	EDX, 20
	MOV EAX, [ESP + 4]
	INT 0x40
	RET