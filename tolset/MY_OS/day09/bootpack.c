#include "bootpack.h"
#include <stdio.h>

void HariMain(void)
{
	struct BOOTINFO *binfo = ( struct BOOTINFO *) ADR_BOOTINFO;
	struct MOUSE_DEC mdec;
	char s[40], mcursor[256], keybuf[32], mousebuf[128];
	int mx, my, i;
	unsigned int memtotal;

	init_gdtidt();//初始化gdt、idt表
	init_pic();  //初始化pic控制器
	io_sti();

	fifo8_init(&keyfifo, 32, keybuf);
	fifo8_init(&mousefifo, 128, mousebuf);
    io_out8(PIC0_IMR, 0xf9); /* 开放PIC1和键盘中断(11111001),键盘是IRQ1 */
	io_out8(PIC1_IMR, 0xef); /* 开放鼠标中断(11101111) ，鼠标是IRQ12*/

	init_keyboard();
	enable_mouse(&mdec);

	init_palette();
    init_screen8(binfo->vram, binfo->scrnx, binfo->scrny);
    /* 显示鼠标 */
	mx = (binfo->scrnx - 16) / 2; /* 计算画面的中心坐标*/
	my = (binfo->scrny - 28 - 16) / 2;
	init_mouse_cursor8(mcursor, COL8_840084);
	putblock8_8(binfo->vram, binfo->scrnx, 16, 16, mx, my, mcursor, 16);
	sprintf(s, "(%d, %d)", mx, my);
	putfonts8_asc(binfo->vram, binfo->scrnx, 0, 0, COL8_FFFFFF, s);
	i = memtest(0x00400000, 0xbfffffff) / (1024 * 1024); //算出内存地址,因为0x00400000之前的地址我们依已经用来做系统启动了肯定是能用的
	sprintf(s, "memory %dMB", i);
	putfonts8_asc(binfo->vram, binfo->scrnx, 0, 32, COL8_FFFFFF, s);

	for (;;) {
		io_cli();
		if (fifo8_status(&keyfifo) + fifo8_status(&mousefifo) == 0) {
			io_stihlt();
		} else {
			if (fifo8_status(&keyfifo) != 0) {
				i = fifo8_get(&keyfifo);
				io_sti();
				sprintf(s, "%02X", i);
				boxfill8(binfo->vram, binfo->scrnx, COL8_840084,  0, 16, 15, 31);
				putfonts8_asc(binfo->vram, binfo->scrnx, 0, 16, COL8_FFFFFF, s);
			} else if(fifo8_status(&mousefifo) != 0)
		    {
		    	i = fifo8_get(&mousefifo);
				io_sti();
				if(mouse_decode(&mdec,i) != 0)
				{
				//凑齐三字节了进行输出！！
				sprintf(s, "[lcr %4d %4d]", mdec.x, mdec.y);
				if((mdec.btn & 0x01) != 0)
				{
					s[1] = 'L';
				}
				if((mdec.btn & 0x02) != 0)
				{
					s[3] = 'R';
				}
				if((mdec.btn & 0x04) != 0)
				{
					s[3] = 'C';
				}
				boxfill8(binfo->vram, binfo->scrnx, COL8_840084, 32, 16, 32+15*8 - 1, 31);
				putfonts8_asc(binfo->vram, binfo->scrnx, 32, 16, COL8_FFFFFF, s);
				//让鼠标动起来
				boxfill8(binfo->vram, binfo->scrnx, COL8_840084, mx, my, mx+15, my+15);//隐藏鼠标
				mx += mdec.x;
				my += mdec.y;
				if(mx < 0)
				{
					mx =0;
				}
				if(my < 0)
				{
					my = 0;
				}
				if(mx > binfo->scrnx - 16)
				{
					mx = binfo->scrnx - 16;
				}
				if(my > binfo->scrny - 16)
				{
					my = binfo->scrny - 16;
				}
				sprintf(s, "(%3d, %3d)", mx, my);
				boxfill8(binfo->vram, binfo->scrnx, COL8_840084, 0, 0, 79, 15);//隐藏坐标
				putfonts8_asc(binfo->vram, binfo->scrnx, 0, 0, COL8_FFFFFF, s);//显示坐标
				putblock8_8(binfo->vram, binfo->scrnx, 16, 16, mx, my, mcursor, 16);//描绘图像
				}
		    }
		}
	}
}







