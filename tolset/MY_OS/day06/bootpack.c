#include "bootpack.h"
#include <stdio.h>

void HariMain(void)
{
	struct BOOTINFO *binfo = ( struct BOOTINFO *) ADR_BOOTINFO;
	char s[40], mcursor[256];
	int mx,my;

	init_gdtidt();//初始化gdt、idt表
	init_pic();  //初始化pic控制器
	io_sti();

	init_palette();
    init_screen8(binfo->vram, binfo->scrnx, binfo->scrny);
    /* 显示鼠标 */
	mx = (binfo->scrnx - 16) / 2; /* 计算画面的中心坐标*/
	my = (binfo->scrny - 28 - 16) / 2;
	init_mouse_cursor8(mcursor, COL8_008484);
	putblock8_8(binfo->vram, binfo->scrnx, 16, 16, mx, my, mcursor, 16);
	sprintf(s, "FOS made by FH! (%d, %d)", mx, my);
	putfonts8_asc(binfo->vram, binfo->scrnx, 0, 0, COL8_FFFFFF, s);
  
    io_out8(PIC0_IMR, 0xf9); /* 开放PIC1和键盘中断(11111001),键盘是IRQ1 */
	io_out8(PIC1_IMR, 0xef); /* 开放鼠标中断(11101111) ，鼠标是IRQ12*/

	for(;;)   
	{
	     io_hlt();
	    //自建的hlt函数，源目标程序在naskfunc.nas中，c语言本身没有hlt这个命
	}
}

