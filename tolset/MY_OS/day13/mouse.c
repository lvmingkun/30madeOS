#include "bootpack.h"

struct FIFO32 *mousefifo;
int mousedata0;

void inthandler2c(int *esp)
/* 来自PS/2鼠标的中断 */
{
	int data;
	io_out8(PIC1_OCW2, 0X64); // 通知PIC1 已经完成 IRQ-12
	io_out8(PIC0_OCW2, 0X62);// 通知PIC0 已经完成 IRQ-2
	data = io_in8(PORT_KEYDAT);
	fifo32_put(mousefifo, data + mousedata0);
  	return;
}

void enable_mouse(struct FIFO32 *fifo, int data0, struct MOUSE_DEC *mdec)
{
	mousefifo = fifo;
	mousedata0 = data0;
	// 激活鼠标
	wait_KBC_sendready();
	io_out8(PORT_KEYCMD, KEYCMD_SENDTO_MOUSE);// 往键盘控制电路发送 0xd4，则代表下一个数据发给鼠标
	wait_KBC_sendready();
	io_out8(PORT_KEYDAT, MOUSECMD_ENABLE); // 激活鼠标数据 0xfa
	mdec->phase = 0;
	return; // 顺利的话，键盘控制其返回 ACK（0xfa) <== 答复信息
}

int mouse_decode(struct MOUSE_DEC *mdec, unsigned char dat)
{
	if (mdec->phase == 0) {
		/* 等待鼠标的 0xfa 阶段，舍弃激活后的答复0xfa */
		if (dat == 0xfa) {
			mdec->phase = 1;
		}
		return 0;
	}
	if (mdec->phase == 1) {
		/* 等待鼠标第一字节 */
		if((dat & 0xc8) == 0x08)
		{   // 如果第一字节正确的话
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

		mdec->btn = mdec->buf[0] & 0x07; // 鼠标按键状态是在buf[0]的低三位，因此只取出这一部分即可
		mdec->x = mdec->buf[1];
		mdec->y = mdec->buf[2];

		if((mdec->buf[0] & 0x10) != 0)
		{
			mdec->x |= 0xffffff00; // xy的左右移动信息就只有8位，因此用这个方法取出即可
		}
		if((mdec->buf[0] & 0x20) != 0)
		{
			mdec->y |= 0xffffff00;
		}
		mdec->y = - mdec->y; // 鼠标的y方向与屏幕的y方向相反
		return 1;
	}
	return -1; /* 应该不会来这的，鼠标激活了就会传送数据的 */
}