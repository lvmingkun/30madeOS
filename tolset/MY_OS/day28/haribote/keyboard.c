#include "bootpack.h"

struct FIFO32 *keyfifo;
int keydata0;

// 鼠标中断利用IRQ12（int 2c) 键盘中断利用IRQ1(int 21)
void inthandler21(int *esp)
{
	int data;
	io_out8(PIC0_OCW2, 0X61);
	/* 通知PIC “IRQ-01”已经受理完毕;这里的计算公式是 0x60 + IRQ号码
   只有输出到ocw2后，PIC才会继续监视IRQ1中断是否发生，如果不写这句话则后面不会再产生中断了
   */
	data = io_in8(PORT_KEYDAT);
	fifo32_put(keyfifo, data + keydata0);
	return;
}

void wait_KBC_sendready(void)
{
	for(;;)
	{
	// 等待键盘控制电路准备完毕，检测到设备号 0x0064 处数据的第 2 位是 0 时，就跳出等待
		if ((io_in8(PORT_KEYSTA) & KEYSTA_SEND_NOTREADY) == 0) {
			break;
		}
	}
	return;
}

void init_keyboard(struct FIFO32 *fifo, int data0)
{
	/* 将fifo缓冲区信息保存到全局变量中 */
	keyfifo = fifo;
	keydata0 = data0;
	/* 初始化键盘控制电路 */
	wait_KBC_sendready();
	io_out8(PORT_KEYCMD, KEYCMD_WRITE_MODE); // 向键盘控制电路发送模式设定指令，模式设定指令是 0x60
	wait_KBC_sendready();
	io_out8(PORT_KEYDAT, KBC_MODE); // 向键盘控制电路发送模式设定指令，鼠标的模式号码是 0x47
	return;
}


