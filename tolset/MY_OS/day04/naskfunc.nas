; naskfunc
; TAB = 4

[FORMAT "WCOFF"]    ;制作的目标文件的模式
[INSTRSET "i486p"]  ;告诉汇编编译器，这个是给486使用的，而不是8086 即可以识别到寄存器EAX
[BITS 32]           ;制作32位的机器语言模式
[FILE "naskfunc.nas"]   ;源文件名信息

;制作具体的函数库
         
         GLOBAL         _write_mem8    ;程序中包含的函数名，一定要以_开头，为了衔接c语言的函数库
         GLOBAL         _io_hlt, _io_cli, _io_sti, _io_stihlt
         GLOBAL         _io_in8, _io_in16, _io_in32
         GLOBAL         _io_out8, _io_out16, _io_out32
         GLOBAL         _io_load_eflags, _io_store_eflags

; 以下是实际的函数内容
[SECTION .text]          ;目标文件中写了这些之后再写程序

_write_mem8: ; void wirte_mem8(int addr, int data);
         mov ECX,[esp+4]   ;[ESP+4]中存放的是地址，将其读入搭配ecx中
         mov al,[esp+8]    ;[esp+8]中存放的是数据，将其放入al中
         mov [ECX],al
         RET

_io_hlt:   ;void io_hlt(void);
         HLT 
         RET

_io_cli: ;void io_cli(void)
         CLI 
         RET
        
_io_sti: ;void io_sti(void)
         STI 
         RET
        
_io_stihlt: ;void io_stihld(void)
         STI
         HLT
         RET

_io_in8: ;int io_in8(int port)
         mov edx,[esp+4]
         mov eax,0
         in  al,dx  ;将dx端口的数据读8字节到al中，下面的同理
         RET

_io_in16: ;int io_in16(int port)
         mov edx,[esp+4]
         mov eax,0
         in  ax,dx
         RET

_io_in32: ;int io_in32(int port)
         mov edx,[esp+4]
         in  eax,dx
         RET

_io_out8: ;void io_out8(int port ,int data)
         mov edx,[esp+4] ;port
         mov al,[esp+8]  ;data
         out dx,al       ;将al的数据写到dx这个端口
         RET    

_io_out16:  ;void io_out16(int port ,int data)
         mov edx,[esp+4] ;port
         mov ax,[esp+8]  ;data
         out dx,ax       
         RET 

_io_out32:  ;void io_out32(int port ,int data)
         mov edx,[esp+4] ;port
         mov eax,[esp+8]  ;data
         out dx,eax      
         RET 

_io_load_eflags: ;int io_load_eflags(void)
         pushfd   ;指push flag寄存器
         pop eax   ;这里解释一下：函数的返回值，在汇编中是由：char型 AL ； int型　AX
         RET

_io_store_eflags: ; void io_store_flags(int flags)
         mov eax,[esp+4]
         push eax
         popfd
         RET

