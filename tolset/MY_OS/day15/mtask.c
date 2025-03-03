//多任务
#include "bootpack.h"

struct TIMER *mt_timer;
int mt_tr;

void mt_init(void)
{
    mt_timer = timer_alloc();
    // 这里没必要进行timer_init了, 这里是因为发送超时后不需要像fifo缓冲区写入数据
    timer_settime(mt_timer, 2);
    mt_tr = 3 * 8;
    return;
}

void mt_taskswitch(void)
{
    if(mt_tr == 3 * 8)
    {
        mt_tr = 4 * 8;
    }else{
        mt_tr = 3 * 8;
    }
    timer_settime(mt_timer, 2);
    farjmp(0, mt_tr);
    return;
}
