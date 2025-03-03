#include "bootpack.h"
#include <stdio.h>

extern struct FIFO8 keyfifo, mousefifo;
void wait_KBC_sendready(void);
void init_keyboard(void);
void enable_mouse(struct MOUSE_DEC *mdec);
int mouse_decode(struct MOUSE_DEC *mdec, unsigned char dat);

void HariMain(void)
{
	struct BOOTINFO *binfo = ( struct BOOTINFO *) ADR_BOOTINFO;
	struct MOUSE_DEC mdec;
	char s[40], mcursor[256], keybuf[32], mousebuf[128];
	int mx, my, i;

	init_gdtidt();//初始化gdt、idt表
	init_pic();  //初始化pic控制器
	io_sti();

	fifo8_init(&keyfifo, 32, keybuf);
	fifo8_init(&mousefifo, 128, mousebuf);
    io_out8(PIC0_IMR, 0xf9); /* 开放PIC1和键盘中断(11111001),键盘是IRQ1 */
	io_out8(PIC1_IMR, 0xef); /* 开放鼠标中断(11101111) ，鼠标是IRQ12*/

	init_keyboard();

	init_palette();
    init_screen8(binfo->vram, binfo->scrnx, binfo->scrny);
    /* 显示鼠标 */
	mx = (binfo->scrnx - 16) / 2; /* 计算画面的中心坐标*/
	my = (binfo->scrny - 28 - 16) / 2;
	init_mouse_cursor8(mcursor, COL8_840084);
	putblock8_8(binfo->vram, binfo->scrnx, 16, 16, mx, my, mcursor, 16);
	sprintf(s, "(%d, %d)", mx, my);
	putfonts8_asc(binfo->vram, binfo->scrnx, 0, 0, COL8_FFFFFF, s);

	enable_mouse(&mdec);

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

void wait_KBC_sendready(void)
{
	for(;;)
	{
	//等待键盘控制电路准备完毕，检测到设备号0x0064处数据的第2位是0时，就跳出等待
		if ((io_in8(PORT_KEYSTA) & KEYSTA_SEND_NOTREADY) == 0) {
			break;
		}
	}
	return;
}

void init_keyboard(void)
{
	/* 初始化键盘控制电路 */
	wait_KBC_sendready();
	io_out8(PORT_KEYCMD, KEYCMD_WRITE_MODE);//向键盘控制电路发送模式设定指令，模式设定指令是0x60
	wait_KBC_sendready();
	io_out8(PORT_KEYDAT, KBC_MODE);//向键盘控制电路发送模式设定指令，鼠标的模式号码是0x47
	return;
}

void enable_mouse(struct MOUSE_DEC *mdec)
{
	//激活鼠标
	wait_KBC_sendready();
	io_out8(PORT_KEYCMD, KEYCMD_SENDTO_MOUSE);//往键盘控制电路发送0xd4，则代表下一个数据发给鼠标
	wait_KBC_sendready();
	io_out8(PORT_KEYDAT, MOUSECMD_ENABLE);//激活鼠标数据 0xfa
	return; //顺利的话，键盘控制其返回ACK（0xfa) <== 答复信息
}

int mouse_decode(struct MOUSE_DEC *mdec, unsigned char dat)
{
	if (mdec->phase == 0) {
		/*等待鼠标的0xfa阶段，舍弃激活后的答复0xfa */
		if (dat == 0xfa) {
			mdec->phase = 1;
		}
		return 0;
	}
	if (mdec->phase == 1) {
		/* 等待鼠标第一字节 */
		if((dat & 0xc8) == 0x08)
		{//如果第一字节正确的话
			mdec->buf[0] = dat;
		    mdec->phase = 2;
		}
		return 0;
	}
	if (mdec->phase == 2) {
		/* 等待鼠标第二字节 */
		mdec->buf[1] = dat;
		mdec->phase = 3;
		return 0;
	}
	if (mdec->phase == 3) {
		/* 等待鼠标第三字节*/
		mdec->buf[2] = dat;
		mdec->phase = 1;

		mdec->btn = mdec->buf[0] & 0x07; //鼠标按键状态是在buf[0]的低三位，因此只取出这一部分即可
		mdec->x = mdec->buf[1];
		mdec->y = mdec->buf[2];

		if((mdec->buf[0] & 0x10) != 0)
		{
			mdec->x |= 0xffffff00;//xy的左右移动信息就只有8位，因此用这个方法取出即可
		}
		if((mdec->buf[0] & 0x20) != 0)
		{
			mdec->y |= 0xffffff00;
		}
		mdec->y = - mdec->y;//鼠标的y方向与屏幕的y方向相反
		return 1;
	}
	return -1; /* 应该不会来这的，鼠标激活了就会传送数据的 */
}



