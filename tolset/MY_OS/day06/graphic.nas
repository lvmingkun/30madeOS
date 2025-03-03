[FORMAT "WCOFF"]
[INSTRSET "i486p"]
[OPTIMIZE 1]
[OPTION 1]
[BITS 32]
	EXTERN	_table_rgb.0
	EXTERN	_io_load_eflags
	EXTERN	_io_cli
	EXTERN	_io_out8
	EXTERN	_io_store_eflags
	EXTERN	_hankaku
[FILE "graphic.c"]
[SECTION .data]
_table_rgb.0:
	DB	0
	DB	0
	DB	0
	DB	-1
	DB	0
	DB	0
	DB	0
	DB	-1
	DB	0
	DB	-1
	DB	-1
	DB	0
	DB	0
	DB	0
	DB	-1
	DB	-1
	DB	0
	DB	-1
	DB	0
	DB	-1
	DB	-1
	DB	-1
	DB	-1
	DB	-1
	DB	-58
	DB	-58
	DB	-58
	DB	-124
	DB	0
	DB	0
	DB	0
	DB	-124
	DB	0
	DB	-124
	DB	-124
	DB	0
	DB	0
	DB	0
	DB	-124
	DB	-124
	DB	0
	DB	-124
	DB	0
	DB	-124
	DB	-124
	DB	-124
	DB	-124
	DB	-124
[SECTION .text]
	GLOBAL	_init_palette
_init_palette:
	PUSH	EBP
	MOV	EBP,ESP
	PUSH	_table_rgb.0
	PUSH	15
	PUSH	0
	CALL	_set_palette
	LEAVE
	RET
	GLOBAL	_set_palette
_set_palette:
	PUSH	EBP
	MOV	EBP,ESP
	PUSH	EDI
	PUSH	ESI
	PUSH	EBX
	PUSH	ECX
	MOV	EBX,DWORD [8+EBP]
	MOV	EDI,DWORD [12+EBP]
	MOV	ESI,DWORD [16+EBP]
	CALL	_io_load_eflags
	MOV	DWORD [-16+EBP],EAX
	CALL	_io_cli
	PUSH	EBX
	PUSH	968
	CALL	_io_out8
	CMP	EBX,EDI
	POP	EAX
	POP	EDX
	JLE	L7
L9:
	MOV	EAX,DWORD [-16+EBP]
	MOV	DWORD [8+EBP],EAX
	LEA	ESP,DWORD [-12+EBP]
	POP	EBX
	POP	ESI
	POP	EDI
	POP	EBP
	JMP	_io_store_eflags
L7:
	MOV	AL,BYTE [ESI]
	INC	EBX
	SHR	AL,2
	MOVZX	EAX,AL
	PUSH	EAX
	PUSH	969
	CALL	_io_out8
	MOV	AL,BYTE [1+ESI]
	SHR	AL,2
	MOVZX	EAX,AL
	PUSH	EAX
	PUSH	969
	CALL	_io_out8
	MOV	AL,BYTE [2+ESI]
	SHR	AL,2
	ADD	ESI,3
	MOVZX	EAX,AL
	PUSH	EAX
	PUSH	969
	CALL	_io_out8
	ADD	ESP,24
	CMP	EBX,EDI
	JLE	L7
	JMP	L9
	GLOBAL	_boxfill8
_boxfill8:
	PUSH	EBP
	MOV	EBP,ESP
	PUSH	EDI
	PUSH	ESI
	PUSH	EBX
	PUSH	EDI
	PUSH	EDI
	MOV	AL,BYTE [16+EBP]
	MOV	ECX,DWORD [24+EBP]
	MOV	EDI,DWORD [28+EBP]
	MOV	BYTE [-13+EBP],AL
	CMP	ECX,DWORD [32+EBP]
	JG	L22
	MOV	EBX,DWORD [12+EBP]
	IMUL	EBX,ECX
L20:
	MOV	EDX,DWORD [20+EBP]
	CMP	EDX,EDI
	JG	L24
	MOV	ESI,DWORD [8+EBP]
	ADD	ESI,EBX
	ADD	ESI,EDX
	MOV	DWORD [-20+EBP],ESI
L19:
	MOV	ESI,DWORD [-20+EBP]
	MOV	AL,BYTE [-13+EBP]
	INC	EDX
	MOV	BYTE [ESI],AL
	INC	ESI
	MOV	DWORD [-20+EBP],ESI
	CMP	EDX,EDI
	JLE	L19
L24:
	INC	ECX
	ADD	EBX,DWORD [12+EBP]
	CMP	ECX,DWORD [32+EBP]
	JLE	L20
L22:
	POP	EBX
	POP	ESI
	POP	EBX
	POP	ESI
	POP	EDI
	POP	EBP
	RET
	GLOBAL	_init_screen8
_init_screen8:
	PUSH	EBP
	MOV	EBP,ESP
	PUSH	EDI
	PUSH	ESI
	PUSH	EBX
	SUB	ESP,12
	MOV	EAX,DWORD [16+EBP]
	MOV	EDI,DWORD [12+EBP]
	SUB	EAX,29
	DEC	EDI
	PUSH	EAX
	PUSH	EDI
	PUSH	0
	PUSH	0
	PUSH	14
	PUSH	DWORD [12+EBP]
	PUSH	DWORD [8+EBP]
	CALL	_boxfill8
	MOV	EAX,DWORD [16+EBP]
	SUB	EAX,28
	PUSH	EAX
	PUSH	EDI
	PUSH	EAX
	PUSH	0
	PUSH	8
	PUSH	DWORD [12+EBP]
	PUSH	DWORD [8+EBP]
	CALL	_boxfill8
	MOV	EAX,DWORD [16+EBP]
	ADD	ESP,56
	SUB	EAX,27
	PUSH	EAX
	PUSH	EDI
	PUSH	EAX
	PUSH	0
	PUSH	7
	PUSH	DWORD [12+EBP]
	PUSH	DWORD [8+EBP]
	CALL	_boxfill8
	MOV	EAX,DWORD [16+EBP]
	DEC	EAX
	PUSH	EAX
	MOV	EAX,DWORD [16+EBP]
	PUSH	EDI
	SUB	EAX,26
	PUSH	EAX
	PUSH	0
	PUSH	8
	PUSH	DWORD [12+EBP]
	PUSH	DWORD [8+EBP]
	CALL	_boxfill8
	MOV	ESI,DWORD [16+EBP]
	ADD	ESP,56
	SUB	ESI,24
	PUSH	ESI
	PUSH	59
	PUSH	ESI
	PUSH	3
	PUSH	7
	PUSH	DWORD [12+EBP]
	PUSH	DWORD [8+EBP]
	CALL	_boxfill8
	MOV	EAX,DWORD [16+EBP]
	SUB	EAX,4
	PUSH	EAX
	MOV	DWORD [-16+EBP],EAX
	PUSH	2
	PUSH	ESI
	PUSH	2
	PUSH	7
	PUSH	DWORD [12+EBP]
	PUSH	DWORD [8+EBP]
	CALL	_boxfill8
	ADD	ESP,56
	PUSH	DWORD [-16+EBP]
	PUSH	59
	PUSH	DWORD [-16+EBP]
	PUSH	3
	PUSH	15
	PUSH	DWORD [12+EBP]
	PUSH	DWORD [8+EBP]
	CALL	_boxfill8
	MOV	EAX,DWORD [16+EBP]
	SUB	EAX,5
	PUSH	EAX
	MOV	EAX,DWORD [16+EBP]
	PUSH	59
	SUB	EAX,23
	PUSH	EAX
	MOV	DWORD [-20+EBP],EAX
	PUSH	59
	PUSH	15
	PUSH	DWORD [12+EBP]
	PUSH	DWORD [8+EBP]
	CALL	_boxfill8
	MOV	EAX,DWORD [16+EBP]
	ADD	ESP,56
	SUB	EAX,3
	MOV	DWORD [-24+EBP],EAX
	PUSH	EAX
	PUSH	59
	PUSH	EAX
	PUSH	2
	PUSH	0
	PUSH	DWORD [12+EBP]
	PUSH	DWORD [8+EBP]
	CALL	_boxfill8
	PUSH	DWORD [-24+EBP]
	PUSH	60
	PUSH	ESI
	PUSH	60
	PUSH	0
	PUSH	DWORD [12+EBP]
	PUSH	DWORD [8+EBP]
	CALL	_boxfill8
	MOV	EDI,DWORD [12+EBP]
	ADD	ESP,56
	MOV	EBX,DWORD [12+EBP]
	SUB	EBX,4
	SUB	EDI,47
	PUSH	ESI
	PUSH	EBX
	PUSH	ESI
	PUSH	EDI
	PUSH	15
	PUSH	DWORD [12+EBP]
	PUSH	DWORD [8+EBP]
	CALL	_boxfill8
	PUSH	DWORD [-16+EBP]
	PUSH	EDI
	PUSH	DWORD [-20+EBP]
	PUSH	EDI
	PUSH	15
	PUSH	DWORD [12+EBP]
	PUSH	DWORD [8+EBP]
	CALL	_boxfill8
	ADD	ESP,56
	PUSH	DWORD [-24+EBP]
	PUSH	EBX
	PUSH	DWORD [-24+EBP]
	PUSH	EDI
	PUSH	7
	PUSH	DWORD [12+EBP]
	PUSH	DWORD [8+EBP]
	CALL	_boxfill8
	MOV	EAX,DWORD [12+EBP]
	PUSH	DWORD [-24+EBP]
	SUB	EAX,3
	PUSH	EAX
	PUSH	ESI
	PUSH	EAX
	PUSH	7
	PUSH	DWORD [12+EBP]
	PUSH	DWORD [8+EBP]
	CALL	_boxfill8
	LEA	ESP,DWORD [-12+EBP]
	POP	EBX
	POP	ESI
	POP	EDI
	POP	EBP
	RET
	GLOBAL	_putfont8
_putfont8:
	PUSH	EBP
	MOV	EBP,ESP
	PUSH	EDI
	PUSH	ESI
	XOR	ESI,ESI
	PUSH	EBX
	MOV	EDI,DWORD [28+EBP]
	MOV	BL,BYTE [24+EBP]
L39:
	MOV	EAX,DWORD [20+EBP]
	MOV	EDX,DWORD [16+EBP]
	ADD	EAX,ESI
	IMUL	EAX,DWORD [12+EBP]
	ADD	EAX,DWORD [8+EBP]
	LEA	ECX,DWORD [EDX+EAX*1]
	MOV	DL,BYTE [ESI+EDI*1]
	TEST	DL,DL
	JNS	L31
	MOV	BYTE [ECX],BL
L31:
	MOV	AL,DL
	AND	EAX,64
	TEST	AL,AL
	JE	L32
	MOV	BYTE [1+ECX],BL
L32:
	MOV	AL,DL
	AND	EAX,32
	TEST	AL,AL
	JE	L33
	MOV	BYTE [2+ECX],BL
L33:
	MOV	AL,DL
	AND	EAX,16
	TEST	AL,AL
	JE	L34
	MOV	BYTE [3+ECX],BL
L34:
	MOV	AL,DL
	AND	EAX,8
	TEST	AL,AL
	JE	L35
	MOV	BYTE [4+ECX],BL
L35:
	MOV	AL,DL
	AND	EAX,4
	TEST	AL,AL
	JE	L36
	MOV	BYTE [5+ECX],BL
L36:
	MOV	AL,DL
	AND	EAX,2
	TEST	AL,AL
	JE	L37
	MOV	BYTE [6+ECX],BL
L37:
	AND	EDX,1
	TEST	DL,DL
	JE	L29
	MOV	BYTE [7+ECX],BL
L29:
	INC	ESI
	CMP	ESI,15
	JLE	L39
	POP	EBX
	POP	ESI
	POP	EDI
	POP	EBP
	RET
	GLOBAL	_putfonts8_asc
_putfonts8_asc:
	PUSH	EBP
	MOV	EBP,ESP
	PUSH	EDI
	PUSH	ESI
	PUSH	EBX
	PUSH	EAX
	MOV	EBX,DWORD [28+EBP]
	MOV	AL,BYTE [24+EBP]
	MOV	BYTE [-13+EBP],AL
	MOV	ESI,DWORD [16+EBP]
	MOV	EDI,DWORD [20+EBP]
	CMP	BYTE [EBX],0
	JNE	L47
L49:
	LEA	ESP,DWORD [-12+EBP]
	POP	EBX
	POP	ESI
	POP	EDI
	POP	EBP
	RET
L47:
	MOVZX	EAX,BYTE [EBX]
	SAL	EAX,4
	INC	EBX
	ADD	EAX,_hankaku
	PUSH	EAX
	MOVSX	EAX,BYTE [-13+EBP]
	PUSH	EAX
	PUSH	EDI
	PUSH	ESI
	ADD	ESI,8
	PUSH	DWORD [12+EBP]
	PUSH	DWORD [8+EBP]
	CALL	_putfont8
	ADD	ESP,24
	CMP	BYTE [EBX],0
	JNE	L47
	JMP	L49
[SECTION .data]
_cursor.1:
	DB	"**************.."
	DB	"*OOOOOOOOOOO*..."
	DB	"*OOOOOOOOOO*...."
	DB	"*OOOOOOOOO*....."
	DB	"*OOOOOOOO*......"
	DB	"*OOOOOOO*......."
	DB	"*OOOOOOO*......."
	DB	"*OOOOOOOO*......"
	DB	"*OOOO**OOO*....."
	DB	"*OOO*..*OOO*...."
	DB	"*OO*....*OOO*..."
	DB	"*O*......*OOO*.."
	DB	"**........*OOO*."
	DB	"*..........*OOO*"
	DB	"............*OO*"
	DB	".............***"
[SECTION .text]
	GLOBAL	_init_mouse_cursor8
_init_mouse_cursor8:
	PUSH	EBP
	MOV	EBP,ESP
	PUSH	EDI
	PUSH	ESI
	XOR	EDI,EDI
	PUSH	EBX
	XOR	EBX,EBX
	PUSH	EAX
	MOV	AL,BYTE [12+EBP]
	MOV	ESI,DWORD [8+EBP]
	MOV	BYTE [-13+EBP],AL
L63:
	XOR	EDX,EDX
L62:
	LEA	EAX,DWORD [EDX+EDI*1]
	CMP	BYTE [_cursor.1+EAX],42
	JE	L68
L59:
	CMP	BYTE [_cursor.1+EAX],79
	JE	L69
L60:
	CMP	BYTE [_cursor.1+EAX],46
	JE	L70
L57:
	INC	EDX
	CMP	EDX,15
	JLE	L62
	INC	EBX
	ADD	EDI,16
	CMP	EBX,15
	JLE	L63
	POP	EAX
	POP	EBX
	POP	ESI
	POP	EDI
	POP	EBP
	RET
L70:
	MOV	CL,BYTE [-13+EBP]
	MOV	BYTE [EAX+ESI*1],CL
	JMP	L57
L69:
	MOV	BYTE [EAX+ESI*1],7
	JMP	L60
L68:
	MOV	BYTE [EAX+ESI*1],0
	JMP	L59
	GLOBAL	_putblock8_8
_putblock8_8:
	PUSH	EBP
	MOV	EBP,ESP
	PUSH	EDI
	PUSH	ESI
	XOR	ESI,ESI
	PUSH	EBX
	SUB	ESP,12
	CMP	ESI,DWORD [20+EBP]
	JGE	L83
	XOR	EDI,EDI
L81:
	XOR	EBX,EBX
	CMP	EBX,DWORD [16+EBP]
	JGE	L85
	MOV	EAX,DWORD [32+EBP]
	ADD	EAX,EDI
	MOV	DWORD [-20+EBP],EAX
L80:
	MOV	EAX,DWORD [28+EBP]
	MOV	EDX,DWORD [24+EBP]
	ADD	EAX,ESI
	ADD	EDX,EBX
	IMUL	EAX,DWORD [12+EBP]
	ADD	EAX,EDX
	MOV	ECX,DWORD [8+EBP]
	MOV	EDX,DWORD [-20+EBP]
	INC	EBX
	MOV	DL,BYTE [EDX]
	MOV	BYTE [EAX+ECX*1],DL
	INC	DWORD [-20+EBP]
	CMP	EBX,DWORD [16+EBP]
	JL	L80
L85:
	INC	ESI
	ADD	EDI,DWORD [36+EBP]
	CMP	ESI,DWORD [20+EBP]
	JL	L81
L83:
	ADD	ESP,12
	POP	EBX
	POP	ESI
	POP	EDI
	POP	EBP
	RET
