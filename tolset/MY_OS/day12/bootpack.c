#include "bootpack.h"
#include <stdio.h>

void make_window8(unsigned char *buf, int xsize, int ysize, char *title);

void HariMain(void)
{
	struct BOOTINFO *binfo = ( struct BOOTINFO *) ADR_BOOTINFO;
	struct FIFO8 timerfifo, timerfifo2, timerfifo3;
	struct MOUSE_DEC mdec;
	struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
	char s[40], keybuf[32], mousebuf[128], timerbuf[8], timerbuf2[8], timerbuf3[8];
	struct TIMER *timer, *timer2, *timer3;
	int mx, my, i;
	unsigned int memtotal;
	struct SHTCTL *shtctl;
	struct SHEET *sht_back, *sht_mouse, *sht_win;
	unsigned char *buf_back, buf_mouse[256], *buf_win;

	init_gdtidt();	// 初始化 gdt、idt 表
	init_pic();  // 初始化 pic 控制器
	io_sti();

	fifo8_init(&keyfifo, 32, keybuf);
	fifo8_init(&mousefifo, 128, mousebuf);
	init_pit();
    io_out8(PIC0_IMR, 0xf8); /* 开放 PIC1 , PIT 和键盘中断(11111001),键盘是 IRQ1 */
	io_out8(PIC1_IMR, 0xef); /* 开放鼠标中断(11101111) ，鼠标是 IRQ12 */

	fifo8_init(&timerfifo, 8, timerbuf);
	timer = timer_alloc();
	timer_init(timer, &timerfifo, 1);
	timer_settime(timer, 1000);
	fifo8_init(&timerfifo2, 8, timerbuf2);
	timer2 = timer_alloc();
	timer_init(timer2, &timerfifo2, 1);
	timer_settime(timer2, 300);
	fifo8_init(&timerfifo3, 8, timerbuf3);
	timer3 = timer_alloc();
	timer_init(timer3, &timerfifo3, 1);
	timer_settime(timer3, 50);

	init_keyboard();
	enable_mouse(&mdec);
	memtotal = memtest(0x00400000, 0xbfffffff);
	memman_init(memman);
	memman_free(memman, 0x00001000, 0x0009e000); /* 0x00001000 - 0x0009efff */
	memman_free(memman, 0x00400000, memtotal - 0x00400000);

	init_palette();
    shtctl = shtctl_init(memman, binfo->vram, binfo->scrnx, binfo->scrny); // 传入内存管理表是因为要分配地址给图层
	sht_back  = sheet_alloc(shtctl); // 分配给背景一个图层，背景图层位于最底层,sheets0[0]
	sht_mouse = sheet_alloc(shtctl); // sheets0[1]
	sht_win = sheet_alloc(shtctl);
	buf_back  = (unsigned char *) memman_alloc_4k(memman, binfo->scrnx * binfo->scrny); // 分配给back一个与vram大小一样的地址
	buf_win   = (unsigned char *) memman_alloc_4k(memman, 160 * 52);
	sheet_setbuf(sht_back, buf_back, binfo->scrnx, binfo->scrny, -1); // 没有透明色，由于我们已经给图层管理表的初始地址设定在了vram当中，因此任何图层都会在vram空间里
	sheet_setbuf(sht_mouse, buf_mouse, 16, 16, 99); // 鼠标像素阵 16 x 16，给予最不透明度
	sheet_setbuf(sht_win, buf_win, 160, 52, -1); // 无透明色
    init_screen8(buf_back, binfo->scrnx, binfo->scrny);// 往back地址中填入颜色
    init_mouse_cursor8(buf_mouse, 99); // 背景色 99 号，往mouse中填入颜色和透明度
	make_window8(buf_win, 160, 52, "counter");
	// putfonts8_asc(buf_win, 160, 24, 28, COL8_000000, "welcome to");
	// putfonts8_asc(buf_win, 160, 24, 44, COL8_000000, "FNIX-OS!");
    sheet_slide(sht_back, 0, 0); // 将背景页面不移动，直接覆盖整个画面
 
	mx = (binfo->scrnx - 16) / 2; /* 计算画面的中心坐标*/
	my = (binfo->scrny - 28 - 16) / 2;
	sheet_slide(sht_mouse, mx, my); // 移动鼠标到画面中心
	sheet_slide(sht_win, 80, 72);
	sheet_updown(sht_back,  0); // 背景图的高度设置为 0，并且将 sheets 按高度顺序写入
	sheet_updown(sht_win, 1);
	sheet_updown(sht_mouse, 2); // 鼠标图层高度设置为 1
	sprintf(s, "(%3d, %3d)", mx, my);
	putfonts8_asc(buf_back, binfo->scrnx, 0, 0, COL8_FFFFFF, s);
	sprintf(s, "memory %dMB   free : %dKB",
			memtotal / (1024 * 1024), memman_total(memman) / 1024);
	putfonts8_asc(buf_back, binfo->scrnx, 0, 32, COL8_FFFFFF, s);
	sheet_refresh(sht_back, 0, 0, binfo->scrnx, 48);

	for(;;)   
	{
		sprintf(s, "%010d", timerctl.count);
		boxfill8(buf_win, 160, COL8_C6C6C6,  40, 28, 119, 43);
		putfonts8_asc(buf_win, 160, 40, 28, COL8_000000, s);
		sheet_refresh(sht_win, 40, 28, 120, 44);
		io_cli(); // IF = 0
		if (fifo8_status(&keyfifo) + fifo8_status(&mousefifo) + fifo8_status(&timerfifo) 
		+ fifo8_status(&timerfifo2) + fifo8_status(&timerfifo3) == 0) {
					io_sti();
		} else {
			if (fifo8_status(&keyfifo) != 0) {
			    i = fifo8_get(&keyfifo);
			    io_sti();
			    sprintf(s, "%02X", i);
			    boxfill8(buf_back, binfo->scrnx, COL8_FF00FF,  0, 16, 15, 31);
			    putfonts8_asc(buf_back, binfo->scrnx, 0, 16, COL8_FFFFFF, s);
		        sheet_refresh(sht_back, 0, 16, 16, 32);
		    } else if(fifo8_status(&mousefifo) != 0)
		    {
		    	i = fifo8_get(&mousefifo);
				io_sti();
				if (mouse_decode(&mdec, i) != 0)
				{
				// 凑齐三字节了进行输出！！
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
					s[2] = 'C';
				}
				boxfill8(buf_back, binfo->scrnx, COL8_FF00FF, 32, 16, 32 + 15 * 8 - 1, 31);
				putfonts8_asc(buf_back, binfo->scrnx, 32, 16, COL8_FFFFFF, s);
				sheet_refresh(sht_back, 32, 16, 32 + 15 * 8, 32);
				// 让鼠标动起来
				mx += mdec.x;
				my += mdec.y;
				if(mx < 0)
				{
					mx = 0;
				}
				if(my < 0)
				{
					my = 0;
				}
				if(mx > binfo->scrnx - 1)
				{
					mx = binfo->scrnx - 1;
				}
				if(my > binfo->scrny - 1)
				{
					my = binfo->scrny - 1;
				}
				sprintf(s, "(%3d, %3d)", mx, my);
				boxfill8(buf_back, binfo->scrnx, COL8_FF00FF, 0, 0, 79, 15); // 隐藏坐标
				putfonts8_asc(buf_back, binfo->scrnx, 0, 0, COL8_FFFFFF, s); // 显示坐标
				sheet_refresh(sht_back, 0, 0, 80, 16);
				sheet_slide(sht_mouse, mx, my); // 包含sheet_slide与sheet_refresh
				}
				} else if (fifo8_status(&timerfifo) != 0)
				{
				i = fifo8_get(&timerfifo); // 首先读入，为了设定起始点
				io_sti();
				putfonts8_asc(buf_back, binfo->scrnx, 0, 64, COL8_FFFFFF, "10[sec]");
				sheet_refresh(sht_back, 0, 64, 56, 80);
				} else if (fifo8_status(&timerfifo2) != 0){
			    i = fifo8_get(&timerfifo2); 
				io_sti();
				putfonts8_asc(buf_back, binfo->scrnx, 0, 80, COL8_FFFFFF, "3[sec]");
				sheet_refresh(sht_back, 0, 80, 48, 96);
				} else if (fifo8_status(&timerfifo3) != 0) { // 模拟光标
				i = fifo8_get(&timerfifo3);
				io_sti();
				if (i != 0) {
					timer_init(timer3, &timerfifo3, 0); // 然后设置为 0
					boxfill8(buf_back, binfo->scrnx, COL8_FFFFFF, 8, 96, 15, 111);
				} else {
					timer_init(timer3, &timerfifo3, 1); // 然后设置为 1
					boxfill8(buf_back, binfo->scrnx, COL8_FF00FF, 8, 96, 15, 111);
				}
				timer_settime(timer3, 50);
				sheet_refresh(sht_back, 8, 96, 16, 112);
			}				
	   	}
	}
}

void make_window8(unsigned char *buf, int xsize, int ysize, char *title)
{
	// x按钮功能，和 init_moise_cursor8类似
	static char closebtn[14][16] = {
		"OOOOOOOOOOOOOOO@",
		"OQQQQQQQQQQQQQ$@",
		"OQQQQQQQQQQQQQ$@",
		"OQQQ@@QQQQ@@QQ$@",
		"OQQQQ@@QQ@@QQQ$@",
		"OQQQQQ@@@@QQQQ$@",
		"OQQQQQQ@@QQQQQ$@",
		"OQQQQQ@@@@QQQQ$@",
		"OQQQQ@@QQ@@QQQ$@",
		"OQQQ@@QQQQ@@QQ$@",
		"OQQQQQQQQQQQQQ$@",
		"OQQQQQQQQQQQQQ$@",
		"O$$$$$$$$$$$$$$@",
		"@@@@@@@@@@@@@@@@"
	};
	int x, y;
	char c;
	// 制作窗口
	boxfill8(buf, xsize, COL8_C6C6C6, 0, 0, xsize - 1, 0);
	boxfill8(buf, xsize, COL8_FFFFFF, 1, 1,  xsize - 2, 1);
	boxfill8(buf, xsize, COL8_C6C6C6, 0, 0, 0, ysize - 1);
	boxfill8(buf, xsize, COL8_FFFFFF, 1, 1, 1, ysize - 2);
	boxfill8(buf, xsize, COL8_FF00FF, xsize - 2, 1, xsize - 2, ysize - 2);
	boxfill8(buf, xsize, COL8_000000, xsize - 1, 0, xsize - 1, ysize - 1);
	boxfill8(buf, xsize, COL8_C6C6C6, 2, 2, xsize - 3, ysize - 3);
	boxfill8(buf, xsize, COL8_840084, 3, 3, xsize - 4, 20);
	boxfill8(buf, xsize, COL8_FF00FF, 1, ysize - 2, xsize - 2, ysize - 2);
	boxfill8(buf, xsize, COL8_000000, 0, ysize - 1, xsize - 1, ysize - 1);
	putfonts8_asc(buf, xsize, 24, 4, COL8_FFFFFF, title);
	// 给鼠标填色！
	for (y = 0; y < 14; y++) {
		for (x = 0; x < 16; x++) {
			c = closebtn[y][x];
			if (c == '@') {
				c = COL8_000000;
			} else if (c == '$') {
				c = COL8_FF00FF;
			} else if (c == 'Q') {
				c = COL8_C6C6C6;
			} else {
				c = COL8_FFFFFF;
			}
			buf[(5 + y) * xsize + (xsize - 21 + x)] = c;
		}
	}
	return;
}








