; naskfunc
; TAB = 4

[FORMAT "WCOFF"]    ; 制作的目标文件的模式
[INSTRSET "i486p"]  ; 告诉汇编编译器，这个是给 486 使用的，而不是 8086 即可以识别到寄存器EAX
[BITS 32]           ; 制作 32 位的机器语言模式
[FILE "naskfunc.nas"]   ; 源文件名信息

; 制作具体的函数库
         
    GLOBAL _io_hlt, _io_cli, _io_sti, _io_stihlt
    GLOBAL _io_in8, _io_in16, _io_in32
    GLOBAL _io_out8, _io_out16, _io_out32
    GLOBAL _io_load_eflags, _io_store_eflags
	GLOBAL _load_gdtr, _load_idtr
	GLOBAL	_load_cr0, _store_cr0
    GLOBAL _asm_inthandler20, _asm_inthandler21, _asm_inthandler27, _asm_inthandler2c
	GLOBAL _asm_inthandler0c, _asm_inthandler0d
	GLOBAL _load_tr, _farjmp, _farcall
	GLOBAL	_memtest_sub, _asm_end_app
	GLOBAL _asm_hrb_api, _start_app
	EXTERN	_inthandler20, _inthandler21, _inthandler27, _inthandler2c, _inthandler0d, _inthandler0c
	EXTERN	_hrb_api


; 以下是实际的函数内容
[SECTION .text]          ; 目标文件中写了这些之后再写程序

_io_hlt:   ; void io_hlt(void);
    HLT 
    RET

_io_cli: ;void io_cli(void)
    CLI 
    RET
        
_io_sti: ; void io_sti(void)
    STI 
    RET
        
_io_stihlt: ; void io_stihld(void)
    STI
    HLT
    RET

_io_in8: ; int io_in8(int port)
    mov edx, [esp + 4]
    mov eax, 0
    in  al, dx  ; 将dx端口的数据读8字节到al中，下面的同理
    RET

_io_in16: ; int io_in16(int port)
    mov edx, [esp + 4]
    mov eax, 0
    in  ax, dx
    RET

_io_in32: ; int io_in32(int port)
    mov edx, [esp + 4]
    in  eax, dx
    RET

_io_out8: ; void io_out8(int port , int data)
    mov edx, [esp + 4] ; port
    mov al, [esp + 8]  ; data
    out dx, al       ; 将al的数据写到dx这个端口
    RET    

_io_out16:  ; void io_out16(int port , int data)
    mov edx, [esp + 4] ; port
    mov ax,[esp + 8]  ; data
    out dx, ax       
    RET 

_io_out32:  ; void io_out32(int port , int data)
    mov edx, [esp + 4] ; port
    mov eax, [esp + 8]  ; data
    out dx, eax      
    RET 

_io_load_eflags: ; int io_load_eflags(void)
    pushfd   ; 指push flag寄存器
    pop eax   ; 这里解释一下：函数的返回值，在汇编中是由：char型 AL ； int型　AX
    RET

_io_store_eflags: ; void io_store_flags(int flags)
    mov eax, [esp + 4]
    push eax
    popfd
    RET

_load_gdtr:		; void load_gdtr(int limit, int addr);
	MOV	AX, [ESP + 4]		; limit
	MOV	[ESP + 6], AX
	LGDT [ESP + 6]
	RET

_load_idtr:		; void load_idtr(int limit, int addr);
	MOV	AX, [ESP + 4]		; limit
	MOV	[ESP + 6], AX
	LIDT [ESP + 6]
	RET

_asm_inthandler0c:
	STI
	PUSH ES
	PUSH DS
	PUSHAD
	MOV	EAX, ESP
	PUSH EAX
	MOV	AX, SS
	MOV	DS, AX
	MOV	ES, AX
	CALL _inthandler0c
	CMP	EAX, 0		
	JNE	_asm_end_app	
	POP	EAX
	POPAD
	POP	DS
	POP	ES
	ADD	ESP, 4		; INT 0x0d 需要这句
	IRETD

_asm_inthandler0d:
	STI
	PUSH ES
	PUSH DS
	PUSHAD
	MOV	EAX, ESP
	PUSH EAX
	MOV	AX, SS
	MOV	DS, AX
	MOV	ES, AX
	CALL _inthandler0d
	CMP	EAX, 0		
	JNE	_asm_end_app
	POP	EAX
	POPAD
	POP	DS
	POP	ES
	ADD	ESP, 4		; INT 0x0d 需要这句
	IRETD

_asm_inthandler20:
	PUSH ES
	PUSH DS
	PUSHAD
	MOV	EAX, ESP
	PUSH EAX
	MOV	AX, SS     ; 以下三步是c规定的，必须相同 ss es ds
	MOV	DS, AX
	MOV	ES, AX
	CALL _inthandler20
	POP	EAX
	POPAD
	POP	DS
	POP	ES
	IRETD

_asm_inthandler21:
	PUSH ES
	PUSH DS
	PUSHAD
	MOV	EAX, ESP
	PUSH EAX
	MOV	AX, SS     ; 以下三步是c规定的，必须相同 ss es ds
	MOV	DS, AX
	MOV	ES, AX
	CALL _inthandler21
	POP	EAX
	POPAD
	POP	DS
	POP	ES
	IRETD

_asm_inthandler27:
	PUSH ES
	PUSH DS
	PUSHAD
	MOV	EAX, ESP
	PUSH EAX
	MOV	AX, SS
	MOV	DS, AX
	MOV	ES, AX
	CALL _inthandler27
	POP	EAX
	POPAD
	POP	DS
	POP	ES
	IRETD

_asm_inthandler2c:
	PUSH ES
	PUSH DS
	PUSHAD
	MOV	EAX, ESP
	PUSH EAX
	MOV	AX, SS
	MOV	DS, AX
	MOV	ES, AX
	CALL _inthandler2c
	POP	EAX
	POPAD
	POP	DS
	POP	ES
	IRETD

_load_cr0: ; int load_cro(void)
        mov eax, cr0 
        RET

_store_cr0: ; void load_cro(int cr0)
        mov EAX, [esp + 4]
        mov cr0, EAX
        RET  

_memtest_sub:	; unsigned int memtest_sub(unsigned int start, unsigned int end)
	PUSH	EDI
	PUSH	ESI
	PUSH	EBX
	MOV	ESI, 0xaa55aa55			; pat0 = 0xaa55aa55;
	MOV	EDI, 0x55aa55aa			; pat1 = 0x55aa55aa;
	MOV	EAX, [ESP + 12 + 4]			; i = start;

mts_loop:
	MOV	EBX, EAX
	ADD	EBX, 0xffc				; p = i + 0xffc;
	MOV	EDX, [EBX]				; old = *p;
	MOV	[EBX], ESI				; *p = pat0;
	XOR	DWORD [EBX], 0xffffffff	; *p ^= 0xffffffff;
	CMP	EDI, [EBX]				; if (*p != pat1) goto fin;
	JNE	mts_fin
	XOR	DWORD [EBX], 0xffffffff	; *p ^= 0xffffffff;
	CMP	ESI, [EBX]				; if (*p != pat0) goto fin;
	JNE	mts_fin
	MOV	[EBX], EDX				; *p = old;
	ADD	EAX, 0x1000				; i += 0x1000;
	CMP	EAX, [ESP + 12 + 8]		; if (i <= end) goto mts_loop;
	JBE	mts_loop
	POP	EBX
	POP	ESI
	POP	EDI
	RET

mts_fin:
	MOV	[EBX], EDX 		; *p = old;
	POP	EBX
	POP	ESI
	POP	EDI
	RET

_load_tr: 				; void load_tr(int tr);
	LTR [esp + 4] 		; tr
	RET

_farjmp: 				; void farjmp(int eip, int cs);
	JMP far [esp + 4] 	; 高地址是cs，低地址是eip
	RET

_farcall: 				; void farcall(int eip, int cs)
    CALL FAR [ESP + 4]
	RET

_asm_hrb_api:
		STI
		PUSH DS
		PUSH ES
		PUSHAD		; 用于保存的PUSH
		PUSHAD		; 用于向hrb_api传值的PUSH
		MOV	AX, SS
		MOV	DS, AX  ; 将操作系统用段地址存入DS和ES
		MOV	ES, AX
		CALL _hrb_api
		CMP	EAX, 0  ; 当EAX不为 0 时程序结束
		JNE	_asm_end_app
		ADD	ESP, 32
		POPAD
		POP	ES
		POP	DS
		IRETD

_asm_end_app:
		; EAX为tss.esp0的地址
		MOV	ESP,[EAX]
		MOV	DWORD [EAX + 4], 0
		POPAD
		RET	; 返回cmd_app


; 此时还在操作系统内部
_start_app:		; void start_app(int eip, int cs, int esp, int ds, int *tss_esp0);
		PUSHAD				; 将操作系统内部此时的32位寄存器值全部保存起来
		MOV	EAX, [ESP + 36]	; 导入应用程序用EIP
		MOV	ECX, [ESP + 40]	; 应用程序用CS
		MOV	EDX, [ESP + 44]	; 应用程序用ESP
		MOV	EBX, [ESP + 48]	; 应用程序用DS/SS
		MOV	EBP, [ESP + 52]	; tss.esp0的地址, 可参考task的定义
		MOV	[EBP], ESP		; 保存操作系统用ESP，[ebp] 指向的是task->tss.esp
		MOV	[EBP + 4], SS	; 保存操作系统用SS, [ebp + 4] 指向的是task->tss.ss
		MOV	ES, BX			; 将DS/SS进行复制，且以防万一，也给其他寄存器值进行赋值
		MOV	DS, BX
		MOV	FS, BX
		MOV	GS, BX
; 下面调整栈，以免用RETF跳转到应用程序
		OR	ECX, 3 			; 将应用程序用段号和3进行OR运算，以便后面跳转到应用程序段
		OR  EBX, 3 			; 将应用程序用段号和3进行OR运算
		PUSH EBX			; 应用程序的SS
		PUSH EDX			; 应用程序的ESP
		PUSH ECX			; 应用程序的CS
		PUSH EAX			; 应用程序的EIP
		RETF				;取得CS和EIP，然后跳转到应用程序的地址
; 应用程序结束后不会回到这里，此时还剩与esp、ss、pushad的各个值
