;hello-os
;TAB=4
   

    ORG 0x7c00 ;启动程序的装载地址一般为0x7c00~0x7dff

;标准的FAT12格式软盘的必备专用的代码 Stand FAT12 format floppy code，即书写在开头的文件描述系统
	jmp entry
	DB		0x90
		DB		"HELLOIPL"		; 启动扇区名称（8字节）
		DW		512				; 每个扇区（sector）大小（必须512字节）
		DB		1				; 簇（cluster）大小（必须为1个扇区）
		DW		1				; FAT起始位置（一般为第一个扇区）
		DB		2				; FAT个数（必须为2）
		DW		224				; 根目录大小（一般为224项）
		DW		2880			; 该磁盘大小（必须为2880扇区1440*1024/512）
		DB		0xf0			; 磁盘类型（必须为0xf0）
		DW		9				; FAT的长度（必须是9扇区）
		DW		18				; 一个磁道（track）有几个扇区（必须为18）
		DW		2				; 磁头数（必须是2）
		DD		0				; 不使用分区，必须是0
		DD		2880			; 重写一次磁盘大小
		DB		0,0,0x29		; 意义不明（固定）
		DD		0xffffffff		; （可能是）卷标号码
		DB		"HELLO-OS   "	; 磁盘的名称（必须为11字字节，不足填空格）
		DB		"FAT12   "		; 磁盘格式名称（必须8字，不足填空格）
		RESB	18				; 先空出18字节
;程序主主体
    entry:
    ;将启动区的初始段地址（0000）设置给以及将启动扇区的偏移地址（7c00)进行设置到栈和数据段中
         mov ax,0
         mov ss,ax
         mov sp,0x7c00
         mov ds,ax
         mov es,ax

         mov si,msg
;程序启动后我们想要显示的内容
    putloop:
         mov al,[si]
         add si,1   ; 给SI加1,循环打印
         cmp al,0
         je fin  ;若数据内存该处的值为零，则该程序终止
         mov ah,0x0e  ; 显示一个文字，调用0x0e号子中断程序
         mov bh,0   ;第0页
         mov bl,11001010B  ; 指定字符颜色
         int 0x10 ; 设置中断程序。10h号中断代表显示字符串，调用bios的中断
         jmp putloop 

    fin: 
         HLT ;cpu停止运行，等待指令，节约资源
         jmp fin ; 无限循环

    msg:
        DB		0x0a, 0x0a		; 换行两次
		DB		"hello, world"
		DB      0x0a
		DB      "OS made by FH"
		DB		0x0a			; 换行
		DB		0

		RESB	0x7dfe-$		; 填写0x00直到0x001fe

		DB		0x55, 0xaa  ;这里标志着有启动程序，因为计算机一般先读最后两字节进行判断是否含启动程序的！！


