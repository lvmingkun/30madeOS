[FORMAT "WCOFF"]
[INSTRSET "i486p"]
[BITS 32]
[FILE "api008.nas"]

	GLOBAL	_api_initmalloc

[SECTION .text]

_api_initmalloc:	; void api_initmalloc(void);
	PUSH EBX
	MOV	EDX, 8
	MOV	EBX, [CS:0x0020]		; malloc的内存地址空间
	MOV	EAX, EBX
	ADD	EAX, 32 * 1024			; 因为管理内存空间的结构大小也要在其中存储，因此实际的内存起始地址在 32 kb之后
	MOV	ECX, [CS:0x0000]		; 数据段大小
	SUB	ECX, EAX
	INT	0x40
	POP	EBX
	RET
