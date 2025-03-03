#include "bootpack.h"

#define FLAGS_OVERRUN 0x0001

void fifo8_init(struct FIFO8 *fifo, int size, unsigned char *buf)
// 初始化fifo地址
{
	fifo->size = size;
	fifo->buf = buf;
	fifo->free = size;
	fifo->flags = 0;
	fifo->p = 0 ;
	fifo->q = 0 ;
	return;
}

int fifo8_put(struct FIFO8 *fifo, unsigned char data) 
// 向 fifo 传递数据并保存
{
	if(fifo->free == 0)
	{
		// 无空余，溢出
		fifo->flags |= FLAGS_OVERRUN;
		return -1;
	}
	fifo->buf[fifo->p] = data;
	fifo->p++;
	if(fifo->p == fifo->size)
	{
		fifo->p = 0;
	}
	fifo->free--;
	return 0;
}

int fifo8_get(struct FIFO8 *fifo)
// 从fifo中获取一个数据
{
	int data;
	if (fifo->free == fifo->size)
	// 缓冲区为空返回 -1
	{
		return -1;
	}
	data = fifo->buf[fifo->q];
	fifo->q++;
	if(fifo->q == fifo->size)
	{
		fifo->q = 0;
	}
	fifo->free++;
	return data;
}

int fifo8_status(struct FIFO8 *fifo)
{
	return fifo->size - fifo->free;
}
