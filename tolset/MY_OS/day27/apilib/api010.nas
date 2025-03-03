[FORMAT "WCOFF"]				; 生成对象文件的模式	
[INSTRSET "i486p"]				; 使用 486 兼容模式
[BITS 32]						; 使用 32 位模式机器语言
[FILE "api010.nas"]				; 源文件信息名

    GLOBAL _api_free

[SECTION .text]
_api_free:			; void api_free(char *addr, int size);
	PUSH EBX
	MOV	EDX, 10
	MOV	EBX, [CS:0x0020]
	MOV	EAX, [ESP + 8]		; addr
	MOV	ECX, [ESP + 12]		; size
	INT	0x40
	POP	EBX
	RET