[FORMAT "WCOFF"]				; 生成对象文件的模式	
[INSTRSET "i486p"]				; 使用 486 兼容模式
[BITS 32]						; 使用 32 位模式机器语言
[FILE "api001.nas"]				; 源文件信息名

    GLOBAL _api_putchar

[SECTION .text]
; 调用cons_putchar这个模式
_api_putchar:	; void api_putchar(int c);
	MOV	EDX, 1
	MOV	AL, [ESP + 4]	; c
	INT	0x40
	RET