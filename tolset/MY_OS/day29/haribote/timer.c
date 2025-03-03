//定时器

#include "bootpack.h"

#define PIT_CTRL	0x0043
#define PIT_CNT0	0x0040

struct TIMERCTL timerctl;

#define TIMER_FLAGS_ALLOC		1
#define TIMER_FLAGS_USING		2

// 初始化，将中断周期设定为 11932 = 0x2e9c；则实际中断频率= 主频/设定数 = 100 HZ ；（这里主频约为 11931800 左右）
void init_pit(void)
{
	int i;
	struct TIMER *t;
	io_out8(PIT_CTRL, 0x34);
	io_out8(PIT_CNT0, 0x9c);
	io_out8(PIT_CNT0, 0x2e);
	timerctl.count = 0;
	for (i = 0; i < MAX_TIMER; i++) {
		timerctl.timers0[i].flags = 0; // 未使用
	}
	t = timer_alloc();
	t->timeout = 0xffffffff; // 因此最初没有正在运行的定时器
	t->flags = TIMER_FLAGS_USING;
	t->next = 0;
	timerctl.t0 = t;
	timerctl.next = 0xffffffff;
	return;
}

struct TIMER *timer_alloc(void)
{
	int i;
	for (i = 0; i < MAX_TIMER; i++) {
		if (timerctl.timers0[i].flags == 0) {
			timerctl.timers0[i].flags = TIMER_FLAGS_ALLOC;
			timerctl.timers0[i].flags2 = 0;
			return &timerctl.timers0[i];
		}
	}
	return 0; /* 没找到 */
}

void timer_free(struct TIMER *timer)
{
	timer->flags = 0;
	return;
}

void timer_init(struct TIMER *timer, struct FIFO32 *fifo, int data)
{
	timer->fifo = fifo;
	timer->data = data;
	return;
}

// 主程序中对于pit计时器是先注册并设置完一个之后再弄另一个的！！
void timer_settime(struct TIMER *timer, unsigned int timeout)
{
	int e;
	struct TIMER *t, *s;
	timer->timeout = timeout + timerctl.count; // 从现在开始后多少秒以后算超时
	timer->flags = TIMER_FLAGS_USING;
	e = io_load_eflags();
	io_cli();
	t = timerctl.t0;
	if (timer->timeout <= t->timeout) {
		// 插入最前面
		timerctl.t0 = timer;
		timer->next = t; /* t */
		timerctl.next = timer->timeout;
		io_store_eflags(e);
		return;
	}
	// 搜索注册位置
    for(;;)  // 找到比当前计时器拟到达时刻还要晚的计时器，然后插入在此前面
    {
		s = t;
		t = t->next;
		if (timer->timeout <= t->timeout) {
			// 插入到s和t中间
			s->next = timer; // s的下一个是timer
			timer->next = t; // timer的下一个是t
			io_store_eflags(e);
			return;
		}
	}
}

void inthandler20(int *esp)
{
	struct TIMER *timer;
	char ts = 0;
	io_out8(PIC0_OCW2, 0x60);	 // IRQ-0信号接收完后告知PIC
	timerctl.count++;
	if (timerctl.next > timerctl.count) {
		return;  // 还不到下一个时刻，因此返回
	}
	timer = timerctl.t0;
	for (;;) { // timers的定时器都是活动中的因此不需要确认flags
		if (timer->timeout > timerctl.count) {
			break;
		}
		/* 超时 */
		timer->flags = TIMER_FLAGS_ALLOC;
		if (timer != task_timer) {
			fifo32_put(timer->fifo, timer->data);
		} else {
			ts = 1;
		}
		timer = timer->next;
	}
	timerctl.t0 = timer;
	timerctl.next = timer->timeout;
	if (ts != 0) {
		task_switch();
	}
	return;
}

int timer_cancel(struct TIMER *timer)
{
	int e;
	struct TIMER *t;
	e = io_load_eflags();
	  io_cli(); // 在设置过程中禁止改变定时器状态
    if(timer->flags == TIMER_FLAGS_USING) { // 是否需要取消
        if(timer == timerctl.t0) {
            // 第一个定时器的取消处理
			t = timer->next;
			timerctl.t0 = t;
			timerctl.next = t->timeout;
		} else {
			// 非第一个定时器的处理，找到timer前一个定时器
			t = timerctl.t0;
			for (;;) {
				if (t->next == timer) {
					break;
				}
				t = t->next;
			}
			t->next = timer->next; // 将timer的下一个指向timer的下一个
		}
		timer->flags = TIMER_FLAGS_ALLOC;
		io_store_eflags(e);
		return 1;
	}
	io_store_eflags(e);
	return 0;
}

void timer_cancelall(struct FIFO32 *fifo)
{
	int e, i;
	struct TIMER *t;
	e = io_load_eflags();
	io_cli();
	for (i = 0; i < MAX_TIMER; i++) {
		t = &timerctl.timers0[i];
		if (t->flags != 0 && t->flags2 != 0 && t->fifo == fifo) {
			timer_cancel(t);
			timer_free(t);
		}
	}
	io_store_eflags(e);
	return;
}
