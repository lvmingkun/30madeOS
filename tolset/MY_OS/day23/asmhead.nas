; haribote-os boot asm
; TAB=4

[INSTRSET "i486p"]

VBEMODE	EQU	0x105			; 1024 x 768 x 8 bit 分辨率
BOTPAK EQU 0x00280000		; 加载 bootpack
DSKCAC EQU 0x00100000		; 磁盘缓存的位置
DSKCAC0	EQU	0x00008000		; 磁盘缓存的位置（实模式）

; 有关Boot_info
CYLS EQU 0X0ff0   ; 启动区设置
LEDS EQU 0X0ff1
VMODE EQU 0X0ff2  ; 设定颜色数目信息，颜色的位数等等
SCRNX EQU 0X0ff4  ; 分辨率的x
SCRNY EQU 0X0ff6  ; 分辨率的y
VRAM EQU 0X0ff8   ; 像缓冲区开始地址

	ORG 0xc200 ; 程序最终被装载到内存的地址
; 即0x8000 + 0x4200 = 0xc2000x8000 是内存存放软盘的地址，0x4200 是软盘存放文件的地方

; 确认VBE是否存在
    MOV AX, 0x9000
	MOV ES, AX
	MOV DI, 0
	MOV AX, 0X4f00 ; 有vbe的话值会自动编程 0x004f
	INT 0x10       ; 取得的数据也是放在es:di中
	CMP AX, 0x004f
	JNE scrn320

; 确认VBE版本，非 2.0 版本不能使用高分辨率
    MOV AX, [ES:DI + 4]
	CMP AX, 0x0200
	JB scrn320

; 取得画面模式信息,这里的数据将会被放在es:di开始的 256 字节中
    MOV CX, VBEMODE
	MOV AX, 0x4f01
	INT 0x10
	CMP AX, 0x004f
	JNE scrn320

; 画面模式信息确认
    CMP BYTE [ES:DI + 0x19], 8
	JNE scrn320
	CMP BYTE [ES:DI + 0x1b], 4
	JNE scrn320
	MOV AX, [ES:DI + 0x00]
	AND AX, 0x0080 
	JZ  scrn320

; 画面模式切换
    MOV BX, VBEMODE + 0x4000 
	mov AX, 0x4f02
    int 0x10
    mov byte [VMODE], 8     ; 屏幕的模式（参考C语言的引用）
	MOV	AX, [ES:DI + 0x12]
	MOV	[SCRNX], AX
	MOV	AX, [ES:DI + 0x14]
	MOV	[SCRNY], AX
	MOV	EAX, [ES:DI + 0x28]
	MOV	[VRAM], EAX
	JMP	keystatus

scrn320:
	MOV	AL, 0x13			; VGA模式， 320 x 200
	MOV	AH, 0x00
	INT	0x10
	MOV	BYTE [VMODE], 8	    ; 记下画面模式，参考c
	MOV	WORD [SCRNX], 320
	MOV	WORD [SCRNY], 200
	MOV	DWORD [VRAM], 0x000a0000
	
keystatus:
; 用BIOS取得键盘上各种LED指示灯的状态
    mov ah, 0x02 
    int 0x16 ; 键盘BIOS
    mov [LEDS], al

; 防止PIC接受所有中断
; AT兼容机的规范、PIC初始化
; 然后之前在CLI不做任何事就挂起
; PIC在同意后初始化

	MOV	AL, 0xff			; 通过向pic的端口 0x21 和 0xa1 写入 0xff 来禁止所有中断
	OUT	0x21, AL
	NOP						; 连续执行OUT指令有些机种无法运行
	OUT	0xa1, AL

	CLI						; 进一步中断CPU

; 让CPU支持 1M 以上内存、设置A20GATE
	CALL	waitkbdout
	MOV		AL, 0xd1
	OUT		0x64, AL
	CALL	waitkbdout
	MOV		AL, 0xdf			; enable A20
	OUT		0x60, AL
	CALL	waitkbdout

; 保护模式转换

[INSTRSET "i486p"]				; 说明使用 486 指令

	LGDT [GDTR0]				; 设置临时GDT
	MOV	EAX, CR0
	AND	EAX, 0x7fffffff			; 使用bit 31（禁用分页）
	OR	EAX, 0x00000001			; bit 0 到 1 转换（保护模式过渡）
	MOV	CR0, EAX
	JMP	pipelineflush

pipelineflush:
	MOV	AX, 1 * 8				;  写32bit的段
	MOV	DS, AX
	MOV	ES, AX
	MOV	FS, AX
	MOV	GS, AX
	MOV	SS, AX

; bootpack传递

	MOV	ESI, bootpack	; 源
	MOV	EDI, BOTPAK		; 目标
	MOV	ECX, 512 * 1024 / 4
	CALL memcpy

; 传输磁盘数据

; 从引导区开始

	MOV	ESI, 0x7c00		; 源
	MOV	EDI, DSKCAC		; 目标
	MOV	ECX, 512 / 4
	CALL memcpy

; 剩余的全部

	MOV	ESI, DSKCAC0 + 512	; 源
	MOV	EDI, DSKCAC+512		; 目标
	MOV	ECX, 0
	MOV	CL, BYTE [CYLS]
	IMUL ECX, 512 * 18 * 2 / 4	; 除以 4 得到字节数
	SUB	ECX, 512 / 4			; IPL 偏移量
	CALL memcpy

; 由于还需要 asmhead 才能完成
; 完成其余的 bootpack 任务

; bootpack启动

	MOV	EBX, BOTPAK
	MOV	ECX, [EBX + 16]
	ADD	ECX, 3			; ECX += 3;
	SHR	ECX, 2			; ECX /= 4;
	JZ skip				; 传输完成
	MOV	ESI, [EBX + 20]	; 源
	ADD	ESI, EBX
	MOV	EDI, [EBX + 12]	; 目标
	CALL memcpy

skip:
	MOV	ESP, [EBX + 12]	; 堆栈的初始化
	JMP	DWORD 2*8:0x0000001b

waitkbdout:
	IN AL, 0x64
	AND	AL, 0x02
	JNZ	waitkbdout		; AND 结果不为 0 跳转到 waitkbdout
	RET

memcpy:
	MOV	EAX, [ESI]
	ADD	ESI, 4
	MOV	[EDI], EAX
	ADD	EDI, 4
	SUB	ECX, 1
	JNZ	memcpy			; 运算结果不为 0 跳转到memcpy
	RET
; memcpy地址前缀大小

ALIGNB 16
GDT0:
	RESB 8								; 初始值
	DW 0xffff, 0x0000, 0x9200, 0x00cf	; 写 32 bit位段寄存器
	DW 0xffff, 0x0000, 0x9a28, 0x0047	; 可执行的文件的 32 bit寄存器（bootpack用）

	DW 0

GDTR0:
	DW 8 * 3 - 1
	DD GDT0

	ALIGNB 16
bootpack: