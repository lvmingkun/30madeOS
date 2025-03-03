//定时器
#include "bootpack.h"

#define PIT_CTRL 0x0043
#define PIT_CNT0 0x0040
#define TIMER_FLAGS_ALLOC 1
#define TIMER_FLAGS_USING 2

struct TIMERCTL timerctl;

// 初始化，将中断周期设定为 11932 = 0x2e9c；则实际中断频率= 主频/设定数 = 100 HZ ；（这里主频约为 11931800 左右）
void init_pit(void)
{
    int i;
    io_out8(PIT_CTRL, 0X34);
    io_out8(PIT_CNT0, 0x9c);
    io_out8(PIT_CNT0, 0x2e);
    timerctl.count = 0;
    timerctl.next = 0xffffffff; // 因此最初没有正在运行的定时器
    timerctl.using = 0;
    for(i = 0; i < MAX_TIMER; i++)
    {
        timerctl.timers0[i].flags = 0; // 未使用
    }
    return;
}

struct TIMER *timer_alloc(void)
{
    int i;
    for(i = 0; i < MAX_TIMER; i++)
    {
        if(timerctl.timers0[i].flags == 0)
        {
            timerctl.timers0[i].flags = TIMER_FLAGS_ALLOC;
            return &timerctl.timers0[i];
        }
    }
    return 0; // 没找到
}

void timer_free(struct TIMER *timer)
{
	timer->flags = 0;
	return;
}

void timer_init(struct TIMER *timer, struct FIFO8 *fifo, unsigned char data)
{
	timer->fifo = fifo;
	timer->data = data;
	return;
}


// 主程序中对于pit计时器是先注册并设置完一个之后再弄另一个的！！
void timer_settime(struct TIMER *timer, unsigned int timeout)
{
    int e, i, j;
    timer->timeout = timeout + timerctl.count; // 从现在开始后多少秒以后算超时
    timer->flags = TIMER_FLAGS_USING;
    e = io_load_eflags();
    io_cli();
    // 搜索注册位置
    for(i = 0; i < timerctl.using; i++)  // 找到比当前计时器拟到达时刻还要晚的计时器，然后插入在此前面
    {
        if(timerctl.timers[i]->timeout >= timer->timeout)
        {
            break;
        }
    }
    // i号之后全部移一位
    for(j = timerctl.using; j > i; j--)
    {
        timerctl.timers[j] = timerctl.timers[j - 1];
    }
    timerctl.using++;
    // 插入空位
    timerctl.timers[i] = timer;
    timerctl.next = timerctl.timers[0]->timeout;
    io_store_eflags(e);
    return;
}

void inthandler20(int *esp)
{
    int i, j;
    io_out8(PIC0_OCW2, 0x60); // IRQ-0信号接收完后告知PIC
    timerctl.count++;
    if(timerctl.next > timerctl.count)
    {
        return; // 还不到下一个时刻，因此返回
    }
	for (i = 0; i < timerctl.using; i++) { // timers的定时器都是活动中的因此不需要确认flags
        if(timerctl.timers[i]->timeout > timerctl.count )
        {
            break;
        }
        // 超时
        timerctl.timers[i]->flags = TIMER_FLAGS_ALLOC;
		fifo8_put(timerctl.timers[i]->fifo, timerctl.timers[i]->data);
    }
    // 正好有i个计时器所以移位
    timerctl.using -= i;
    for(j = 0; j < timerctl.using; j++)
    {
        timerctl.timers[j] = timerctl.timers[i + j];
    }
    if(timerctl.using > 0)
    {
        timerctl.next = timerctl.timers[0]->timeout;
    }else{
        timerctl.next = 0xfffffff;
    }
	return;
}

