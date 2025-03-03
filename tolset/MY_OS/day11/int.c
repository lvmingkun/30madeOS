#include "bootpack.h"
#include <stdio.h>

#define PORT_KEYDAT	0x0060

struct FIFO8 keyfifo;
struct FIFO8 mousefifo;


/*PIC0 ==> 主PIC  PIC1 ==> 从PIC
  IMR  ==> 中断屏蔽寄存器，8位分别对应着8路IRQ信号，置为1则该路的中断会被屏蔽；
  ICW  ==> ICW1与ICW4 是与PIC主板配线，电气特性有关；
           ICW3与主从有关；
           ICW2可用于设定中断号，即将PIC0-15与int 20-2f对应起来，进行统一管理。  
*/
void init_pic(void)
{
	io_out8(PIC0_IMR, 0xff); //禁止所有中断
	io_out8(PIC1_IMR, 0xff); //禁止所有中断

	io_out8(PIC0_ICW1, 0x11); //边沿触发模式
	io_out8(PIC0_ICW2, 0x20); //IRQ0-7由int20-27接收；
	io_out8(PIC0_ICW3, 1 << 2); //PIC1由IRQ2连接；由于硬件已经规定了主的2号连接从，因此这里要写入00000100
	io_out8(PIC0_ICW4, 0x01); //无缓冲区模式

	io_out8(PIC1_ICW1, 0x11); //边沿触发模式
	io_out8(PIC1_ICW2, 0x28); //IRQ8-15由int28-2f接收
	io_out8(PIC1_ICW3,  2  ); //PIC1由IRQ2连接
	io_out8(PIC1_ICW4, 0x01); //无缓冲区模式

	io_out8(PIC0_IMR, 0xfb); //11111011 PIC1外全禁止
	io_out8(PIC1_IMR, 0xff); //11111111 禁止所有中断

	return;
}

void inthandler27(int *esp)
/* PIC0中断的不完整策略 */
/* 这个中断在Athlon64X2上通过芯片组提供的便利，只需执行一次 */
/* 这个中断只是接收，不执行任何操作 */
/* 为什么不处理？
	→  因为这个中断可能是电气噪声引发的、只是处理一些重要的情况。*/
{
	io_out8(PIC0_OCW2, 0x67); /* 通知PIC的IRQ-07（参考7-1） */
	return;
}

