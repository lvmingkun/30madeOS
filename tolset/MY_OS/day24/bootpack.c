#include "bootpack.h"
#include <stdio.h>

#define KEYCMD_LED 0xed

static char keytable0[0x80] = {
	0,   0,   '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', 0,   0,
	'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '[', ']', 0,   0,   'A', 'S',
	'D', 'F', 'G', 'H', 'J', 'K', 'L', ';', '\'', '`',     0,   '\\','Z', 'X', 'C', 'V',
	'B', 'N', 'M', ',', '.', '/', 0,   '*', 0,   ' ', 0,   0,   0,   0,   0,   0,
	0,   0,   0,   0,   0,   0,   0,   '7', '8', '9', '-', '4', '5', '6', '+', '1',
	'2', '3', '0', '.', 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	0,   0,   0,   0x5c, 0,  0,   0,   0,   0,   0,   0,   0,   0,   0x5c, 0,  0
};
static char keytable1[0x80] = {
	0,   0,   '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', 0,   0,
	'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', 0,   0,   'A', 'S',
	'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~',   0,   '|', 'Z', 'X', 'C', 'V',
	'B', 'N', 'M', '<', '>', '?', 0,   '*', 0,   ' ', 0,   0,   0,   0,   0,   0,
	0,   0,   0,   0,   0,   0,   0,   '7', '8', '9', '-', '4', '5', '6', '+', '1',
	'2', '3', '0', '.', 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	0,   0,   0,   '_', 0,   0,   0,   0,   0,   0,   0,   0,   0,   '|', 0,   0
};


void HariMain(void)
{
	struct BOOTINFO *binfo = ( struct BOOTINFO *) ADR_BOOTINFO;
	struct FIFO32 fifo, keycmd;
	struct MOUSE_DEC mdec;
	struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
	struct CONSOLE *cons;
	char s[40];
	struct TIMER *timer;
	int mx, my, i, cursor_x, cursor_c, j, x, y, mmx = -1, mmy = -1;
	int fifobuf[128], keycmd_buf[32];
	unsigned int memtotal;
	struct SHTCTL *shtctl;
	struct SHEET *sht = 0, *key_win;
	struct SHEET *sht_back, *sht_mouse, *sht_win, *sht_cons;
	unsigned char *buf_back, buf_mouse[256], *buf_win, *buf_cons;
	struct TASK *task_a, *task_cons;
	int key_shift = 0, key_leds = (binfo->leds >> 4) & 7, keycmd_wait = -1;

	init_gdtidt();	// 初始化 gdt、idt 表
	init_pic();  // 初始化 pic 控制器
	io_sti();

	fifo32_init(&fifo, 128, fifobuf, 0);
	init_pit();
	init_keyboard(&fifo, 256);
	enable_mouse(&fifo, 512, &mdec);
    io_out8(PIC0_IMR, 0xf8); /* 开放 PIC1 , PIT 和键盘中断(11111001),键盘是 IRQ1 */
	io_out8(PIC1_IMR, 0xef); /* 开放鼠标中断(11101111) ，鼠标是 IRQ12 */
	fifo32_init(&keycmd, 32, keycmd_buf, 0);

	memtotal = memtest(0x00400000, 0xbfffffff);
	memman_init(memman);
	memman_free(memman, 0x00001000, 0x0009e000); /* 0x00001000 - 0x0009efff */
	memman_free(memman, 0x00400000, memtotal - 0x00400000);

	init_palette();
    shtctl = shtctl_init(memman, binfo->vram, binfo->scrnx, binfo->scrny); // 传入内存管理表是因为要分配地址给图层
	task_a = task_init(memman);
	fifo.task = task_a;
	task_run(task_a, 1, 2);
	*((int *) 0x0fe4) = (int) shtctl;
	/* sht_back */	
	sht_back  = sheet_alloc(shtctl); // 分配给背景一个图层，背景图层位于最底层,sheets0[0]
	buf_back  = (unsigned char *) memman_alloc_4k(memman, binfo->scrnx * binfo->scrny); // 分配给back一个与vram大小一样的地址
	sheet_setbuf(sht_back, buf_back, binfo->scrnx, binfo->scrny, -1); // 没有透明色，由于我们已经给图层管理表的初始地址设定在了vram当中，因此任何图层都会在vram空间里
	init_screen8(buf_back, binfo->scrnx, binfo->scrny); // 往back地址中填入颜色
	/* sht_cons */
	sht_cons = sheet_alloc(shtctl);
	buf_cons = (unsigned char *) memman_alloc_4k(memman, 768 * 560);
	sheet_setbuf(sht_cons, buf_cons, 768, 560, -1); /* 透明色 */
	make_window8(buf_cons, 768, 560, "console", 0);
	make_textbox8(sht_cons, 8, 28, 752, 520, COL8_000000);
	task_cons = task_alloc();
	task_cons->tss.esp = memman_alloc_4k(memman, 64 * 1024) + 64 * 1024 - 12;
	task_cons->tss.eip = (int) &console_task;
	task_cons->tss.es = 1 * 8;
	task_cons->tss.cs = 2 * 8;
	task_cons->tss.ss = 1 * 8;
	task_cons->tss.ds = 1 * 8;
	task_cons->tss.fs = 1 * 8;
	task_cons->tss.gs = 1 * 8;
	*((int *) (task_cons->tss.esp + 4)) = (int) sht_cons;
	*((int *) (task_cons->tss.esp + 8)) = memtotal;
	task_run(task_cons, 2, 2); /* level = 2, priority = 2 */
	/* sht_win */
	sht_win = sheet_alloc(shtctl);
	buf_win = (unsigned char *) memman_alloc_4k(memman, 160 * 52);
	sheet_setbuf(sht_win, buf_win, 144, 52, -1);
	make_window8(buf_win, 144, 52, "task_a", 1);
	make_textbox8(sht_win, 8, 28, 128, 16, COL8_FFFFFF);
	cursor_x = 8;
	cursor_c = COL8_FFFFFF;
	timer = timer_alloc();
	timer_init(timer, &fifo, 1);
	timer_settime(timer, 50);
	/* sht_mouse */
	sht_mouse = sheet_alloc(shtctl); // sheets0[1]
	sheet_setbuf(sht_mouse, buf_mouse, 16, 16, 99); // 鼠标像素阵 16 x 16，给予最不透明度
    init_mouse_cursor8(buf_mouse, 99); // 背景色 99 号，往mouse中填入颜色和透明度
	mx = (binfo->scrnx - 16) / 2; /* 计算画面的中心坐标*/
	my = (binfo->scrny - 28 - 16) / 2;
    sheet_slide(sht_back, 0, 0); // 将背景页面不移动，直接覆盖整个画面
	sheet_slide(sht_cons, 32, 4);
	sheet_slide(sht_win, 64, 56);
	sheet_slide(sht_mouse, mx, my); // 移动鼠标到画面中心
	sheet_updown(sht_back, 0);
	sheet_updown(sht_cons, 1);
	sheet_updown(sht_win, 2);
	sheet_updown(sht_mouse, 3);
	key_win = sht_win;
	sht_cons->task = task_cons;
	sht_cons->flags |= 0x20;
	// sprintf(s, "(%3d, %3d)", mx, my);
	// putfonts8_asc_sht(sht_back, 0, 0, COL8_FFFFFF, COL8_840084, s, 10);
	// sprintf(s, "memory %dMB   free : %dKB", memtotal / (1024 * 1024), memman_total(memman) / 1024);
	// putfonts8_asc_sht(sht_back, 0, 32, COL8_FFFFFF, COL8_840084, s, 40);
	fifo32_put(&keycmd, KEYCMD_LED);
	fifo32_put(&keycmd, key_leds);

	for(;;) {
		if(fifo32_status(&keycmd) > 0 && keycmd_wait < 0) {
			// 如果存在向键盘控制器发送数据，则进行发送
			keycmd_wait = fifo32_get(&keycmd);
			wait_KBC_sendready();
			io_out8(PORT_KEYDAT, keycmd_wait);
		}
		io_cli(); // IF = 0
		if (fifo32_status(&fifo) == 0) {
			task_sleep(task_a);
			io_sti();
		} else {
			i = fifo32_get(&fifo);
			io_sti();
			if (key_win->flags == 0) {
				key_win = shtctl->sheets[shtctl->top - 1];
				cursor_c = keywin_on(key_win, sht_win, cursor_c);
			}

			if (256 <= i && i <= 511) {
				sprintf(s, "%02X", i - 256);
				putfonts8_asc_sht(sht_back, 0, 16, COL8_FFFFFF, COL8_840084, s, 2);
				if(i < 0x80 + 256) {
					// 将按键编码转化为字符编码
					if(key_shift == 0) {
						s[0] = keytable0[i - 256];
					} else {
						s[0] = keytable1[i - 256];
					}
				} else {
					s[0] = 0;
				}
				if(s[0] >= 'A' && s[0] <= 'Z') {
					if (((key_leds & 4) == 0 && key_shift == 0) || ((key_leds & 4) != 0 && key_shift != 0)) {
						s[0] += 0x20;
					}
				}
				if(s[0] != 0) // 一般字符
				{
					if(key_win == sht_win) {
						// 发送给任务a
						if(cursor_x < 128) {
							// 显示一个字符光标就向后移动一次
							s[1] = 0;
							putfonts8_asc_sht(sht_win, cursor_x, 28, COL8_000000, COL8_C6C6C6, s, 1);
							cursor_x += 8;
						}
					} else {
						// 发送给命令行
						fifo32_put(&task_cons->fifo, s[0] + 256);
					}
				}
				if(i == 256 + 0x0e) // 退格键
				{   // 用空格把光标消去后移动一次
					if(key_win == sht_win) {
						if(cursor_x > 8){
							putfonts8_asc_sht(sht_win, cursor_x, 28, COL8_000000, COL8_FFFFFF, " ", 1);
							cursor_x -= 8;
						}
					} else {
						fifo32_put(&key_win->task->fifo, 8 + 256); // 这里定义在console中退格键的编码为 8
					}
				}
				if(i == 256 + 0x1c) { // Enter
					if(key_win != sht_win) { // 发送到命令行窗口
						fifo32_put(&key_win->task->fifo, 10 + 256);
					}
				}
				if(i == 256 + 0x0f) { // tab键
				    cursor_c = keywin_off(key_win, sht_win, cursor_c, cursor_x);
				    j = key_win->height - 1;
				    if (j == 0) {
						j = shtctl->top - 1;
					}
					key_win = shtctl->sheets[j];
					cursor_c = keywin_on(key_win, sht_win, cursor_c);
				}					
				if (i == 256 + 0x2a) {
					key_shift |= 1;
				}
				if (i == 256 + 0x36) {
					key_shift |= 2;
				}
				if (i == 256 + 0xaa) {
					key_shift &= ~1;
				}
				if (i == 256 + 0xb6) {
					key_shift &= ~2;
				}
				if (i == 256 + 0x3a) {
					key_leds ^= 4;
					fifo32_put(&keycmd, KEYCMD_LED);
					fifo32_put(&keycmd, key_leds);
				}
				if (i == 256 + 0x45) {
					key_leds ^= 2;
					fifo32_put(&keycmd, KEYCMD_LED);
					fifo32_put(&keycmd, key_leds);
				}
				if (i == 256 + 0x46) {
					key_leds ^= 1;
					fifo32_put(&keycmd, KEYCMD_LED);
					fifo32_put(&keycmd, key_leds);
				}
				if (i == 256 + 0xfa) {
					keycmd_wait = -1;
				}
				if (i == 256 + 0xfe) {
					wait_KBC_sendready();
					io_out8(PORT_KEYDAT, keycmd_wait);
				}
				if (i == 256 + 0x3b && key_shift != 0 && task_cons->tss.ss0 != 0) { // shift + F1强制关闭
					cons = (struct CONSOLE *) *((int *) 0xfec);
					cons_putstr0(cons, "\nBreak(key).\n");
					io_cli(); // 不能再改变寄存器的值时候进行切换
					task_cons->tss.eax = (int) &(task_cons->tss.esp0);
					task_cons->tss.eip = (int) asm_end_app;
					io_sti();
				}
				if (i == 256 + 0x99 && shtctl->top > 2) {
					sheet_updown(shtctl->sheets[1], shtctl->top - 1);
				}
				/* 光标再显示 */
				if (cursor_c >= 0) {
					boxfill8(sht_win->buf, sht_win->bxsize, cursor_c, cursor_x, 28, cursor_x + 7, 43);
					}
				sheet_refresh(sht_win, cursor_x, 28, cursor_x + 8, 44);
			} else if (512 <= i && i <= 767) {
				if (mouse_decode(&mdec, i - 512) != 0) {
					// 凑齐三字节了进行输出！！
					// sprintf(s, "[lcr %4d %4d]", mdec.x, mdec.y);
					// if((mdec.btn & 0x01) != 0)
					// {
						// s[1] = 'L';
					// }
					// if((mdec.btn & 0x02) != 0)
					// {
					// 	s[3] = 'R';
					// }
					// if((mdec.btn & 0x04) != 0)
					// {
					// 	s[2] = 'C';
					// }
					// putfonts8_asc_sht(sht_back, 32, 16, COL8_FFFFFF, COL8_840084, s, 15);
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
					// sprintf(s, "(%3d, %3d)", mx, my);
					// putfonts8_asc_sht(sht_back, 0, 0, COL8_FFFFFF, COL8_840084, s, 10);
					sheet_slide(sht_mouse, mx, my); // 包含sheet_slide与sheet_refresh
					if((mdec.btn & 0x01) != 0) // 按下左键
					{
						if(mmx < 0) {
							// 按照从上到下的顺序寻找鼠标所指向的图层
							for (j = shtctl->top - 1; j > 0; j--) {
								sht = shtctl->sheets[j];
								x = mx - sht->vx0;
								y = my - sht->vy0;
								if (0 <= x && x < sht->bxsize && 0 <= y && y < sht->bysize) { // 利用差值判断鼠标是否在该图层之中
									if (sht->buf[y * sht->bxsize + x] != sht->col_inv) {
										sheet_updown(sht, shtctl->top - 1);
										if(sht != key_win) {
											cursor_c = keywin_off(key_win, sht_win, cursor_c, cursor_x);
											key_win = sht;
											cursor_c = keywin_on(key_win, sht_win, cursor_c);
										}
										if(3 <= x && x < sht->bxsize - 3 && 3 <= y && y < 21) {
											// 窗口移动模式
											mmx = mx;
											mmy = my;
										}
										if(sht->bxsize - 21 <= x && x < sht->bxsize - 5 && 5 <= y && y < 19) {

											if ((sht->flags & 0x10) != 0) {
												cons = (struct CONSOLE *) *((int *) 0x0fec);
												cons_putstr0(cons, "\nBreak(mouse) . \n");
												io_cli();
												task_cons->tss.eax = (int) & (task_cons->tss.esp0);
												task_cons->tss.eip = (int) asm_end_app;
												io_sti();
											}
										}
										break;
									}
								}
							}
				     	} else {
						    // 处于窗口移动模式
					    	x = mx - mmx; // 计算鼠标移动模式
					    	y = my - mmy;
						    sheet_slide(sht, sht->vx0 + x, sht->vy0 + y);
						    mmx = mx; // 更新为移动后的位置
						    mmy = my;
					    }
					} else {
						// 没有按下左键
						mmx = -1;
					}
				} else if (i <= 1) { // 模拟光标
					if (i != 0) {
						timer_init(timer, &fifo, 0); // 然后设置为 0
						if (cursor_c >= 0) {
							cursor_c = COL8_000000;
						}
					} else {
						timer_init(timer, &fifo, 1); // 然后设置为 1
						if (cursor_c >= 0) {
						cursor_c = COL8_FFFFFF;
						}
					}
					timer_settime(timer, 50);
					if (cursor_c >= 0) {
						boxfill8(sht_win->buf, sht_win->bxsize, cursor_c, cursor_x, 28, cursor_x + 7, 43);
						sheet_refresh(sht_win, cursor_x, 28, cursor_x + 8, 44);
					}
				}				
	   		}
		}
	}
}

// 控制窗口标题栏的颜色和task_a的光标
int keywin_off(struct SHEET *key_win, struct SHEET *sht_win, int cur_c, int cur_x)
{
	change_wtitle8(key_win, 0);
	if (key_win == sht_win) {
		cur_c = -1; /* 删除光标 */
		boxfill8(sht_win->buf, sht_win->bxsize, COL8_FFFFFF, cur_x, 28, cur_x + 7, 43);
	} else {
		if ((key_win->flags & 0x20) != 0) {
			fifo32_put(&key_win->task->fifo, 3); /* 命令行窗口光标off */
		}
	}
	return cur_c;
}

int keywin_on(struct SHEET *key_win, struct SHEET *sht_win, int cur_c)
{
	change_wtitle8(key_win, 1);
	if (key_win == sht_win) {
		cur_c = COL8_000000; /* 显示光标 */
	} else {
		if ((key_win->flags & 0x20) != 0) {
			fifo32_put(&key_win->task->fifo, 2); /* 命令行窗口光标ON */
     	}
	}
	return cur_c;
}










