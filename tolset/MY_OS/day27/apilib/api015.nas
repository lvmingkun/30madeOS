[FORMAT "WCOFF"]				; 生成对象文件的模式	
[INSTRSET "i486p"]				; 使用 486 兼容模式
[BITS 32]						; 使用 32 位模式机器语言
[FILE "api015.nas"]				; 源文件信息名

    GLOBAL _api_getkey

[SECTION .text]
_api_getkey: 		; int appi_getkey(int mode)
    MOV EDX, 15
	MOV	EAX, [ESP + 4]
	INT 0x40
	RET