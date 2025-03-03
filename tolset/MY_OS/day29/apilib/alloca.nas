[FORMAT "WCOFF"] 	; 生成对象文件的模式
[INSTRSET "i486p"] 	; 使用 486 兼容模式
[BITS 32] 			; 使用 32 位模式机器语言
[FILE "alloca.nas"] ; 源文件信息名

	GLOBAL	__alloca

[SECTION .text]

__alloca:
	ADD	EAX, -4
	SUB	ESP, EAX
	JMP	DWORD [ESP + EAX]
