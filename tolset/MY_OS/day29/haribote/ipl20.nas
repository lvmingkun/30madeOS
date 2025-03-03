; FH-os
; TAB=4

    CYLS EQU 30 ; 柱面�?
    ORG 0x7c00 ; �?动程序的装载地址一�?�? 0x7c00 ~ 0x7dff

    jmp entry
	DB 0x90
    ; 标准的FAT12格式�?盘的必�?�专用的代码 Stand FAT12 format floppy code，即书写在开头的文件描述系统

		DB "HARIBOTE"	    ; �?动扇区名称（ 8 字节�?
		DW 512				; 每个扇区（sector）大小（必须 512 字节�?
		DB 1				; 簇（cluster）大小（必须�? 1 �?扇区�?
		DW 1				; FAT起�?�位�?（一�?为�??一�?扇区�?
		DB 2				; FAT�?数（必须�? 2�?
		DW 224				; 根目录大小（一�?�? 224 项）
		DW 2880			    ; 该�?�盘大小（必须为 2880 扇区 1440 * 1024 / 512�?
		DB 0xf0			    ; 磁盘类型（必须为 0xf0�?
		DW 9				; FAT的长度（必须�? 9 扇区�?
		DW 18				; 一�?磁道（track）有几个扇区（必须为 18�?
		DW 2				; 磁头数（必须�? 2�?
		DD 0				; 不使用分区，必须�? 0
		DD 2880			    ; 重写一次�?�盘大小
		DB 0, 0, 0x29		; 意义不明（固定）
		DD 0xffffffff		; （可能是）卷标号�?
		DB "HARIBOTEOS "	; 磁盘的名称（必须�? 11 字字节，不足�?空格�?
		DB "FAT12   "		; 磁盘格式名称（必�? 8 字，不足�?空格�?
		RESB 18				; 先空�? 18 字节

; 程序主主�?
    entry:
    ; 将启动区的初始�?�地址�?0000）�?�置给以及将�?动扇区的偏移地址�?7c00)进�?��?�置到栈和数�?段中
        mov ax, 0
    	mov ss, ax
        mov sp, 0x7c00
        mov ds, ax

	; 磁盘读取部分
		mov ax, 0x0820
		mov es, ax
		mov ch, 0      ; 柱面
		mov dh, 0		; 磁头
		mov cl, 2		; 扇区

	readloop:
		mov si, 0		; 记录失败次数的寄存器 

	retry:
		mov ah, 0x02	; 读盘
		mov al, 1		; 1 �?扇区
		mov bx, 0
		mov dl, 0x00	; A驱动�?
		int 0x13		; 调用BIOS
		jnc next
		add si, 1
		cmp si, 5
		JAE error
		mov ah, 0x00	 
		mov dl, 0x00
		int 0x13
		jmp retry
	
	next:
		mov ax, es		; 把内存地址后移 0x200
		add ax, 0x0020
		mov es, ax
		add cl, 1
		cmp cl, 18
		jbe readloop
		mov cl, 1
		add dh, 1
		cmp dh, 2
		jb readloop
		mov dh, 0
		add ch, 1
		cmp ch, CYLS
		jb readloop

		MOV [0x0ff0], CH         ; �? cyls 的值移动到 0x0ff0 当中，为了告诉sys磁盘装载内�?�的结束地址，因为cyls代表�? 10 �?柱面
        JMP 0xc200


	error:
        mov si, msg
		 
; 程序�?动后我们想�?�显示的内�??
    putloop:
        mov al, [si]
        add si, 1   ; 给SI�? 1, �?�?打印
        cmp al, 0
        je fin  ; 若数�?内存该�?�的值为零，则�?�程序终�?
        mov ah,0x0e  ; 显示一�?文字，调�? 0x0e 号子�?�?程序
        mov bx, 15
        int 0x10 ; 设置�?�?程序, 10h 号中�?代表显示字�?�串，调用bios的中�?
        jmp putloop 

    fin: 
        HLT ; cpu停�?�运行，等待指令，节约资�?
        jmp fin ; 无限�?�?



    msg:
        DB	 0x0a, 0x0a      ; 换�?�两�?
		DB	 "load error"
		DB   0x0a
		DB   "OS made by FH"
		DB	 0x0a			; 换�??
		DB	 0

		RESB 0x7dfe-$		; �?�? 0x00 直到 0x001fe

		DB	 0x55, 0xaa  ; 这里标志着有启动程序，因为计算机一�?先�?�最后两字节进�?�判�?�?否含�?动程序的！！

