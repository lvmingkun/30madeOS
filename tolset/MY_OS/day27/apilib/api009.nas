[FORMAT "WCOFF"]				; 生成对象文件的模式	
[INSTRSET "i486p"]				; 使用 486 兼容模式
[BITS 32]						; 使用 32 位模式机器语言
[FILE "api009.nas"]				; 源文件信息名

    GLOBAL _api_malloc

[SECTION .text]
_api_malloc:		; char *api_malloc(int size);
	PUSH EBX
	MOV	EDX, 9
	MOV	EBX, [CS:0x0020]
	MOV	ECX, [ESP + 8]		; size
	INT	0x40
	POP	EBX
	RET