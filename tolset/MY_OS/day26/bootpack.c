#include "bootpack.h"
#include <stdio.h>

#define KEYCMD_LED 0xed

void HariMain(void)
{
	struct BOOTINFO *binfo = ( struct BOOTINFO *) ADR_BOOTINFO;
	struct FIFO32 fifo, keycmd;
	struct MOUSE_DEC mdec;
	struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
	char s[40];
	int mx, my, i, j, x, y, mmx = -1, mmy = -1, mmx2 = 0;
	int new_mx = -1, new_my = 0, new_wx = 0x7fffffff, new_wy = 0;
	int fifobuf[128], keycmd_buf[32];
	unsigned int memtotal;
	struct SHTCTL *shtctl;
	struct SHEET *sht = 0, *key_win;
	struct SHEET *sht_back, *sht_mouse;
	unsigned char *buf_back, buf_mouse[256];
	struct TASK *task_a, *task;
	int key_shift = 0, key_leds = (binfo->leds >> 4) & 7, keycmd_wait = -1;
	static char keytable0[0x80] = {
	0,   0,   '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', 0x08,   0,
	'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '[', ']', 0x0a,   0,   'A', 'S',
	'D', 'F', 'G', 'H', 'J', 'K', 'L', ';', '\'', '`',     0,   '\\','Z', 'X', 'C', 'V',
	'B', 'N', 'M', ',', '.', '/', 0,   '*', 0,   ' ', 0,   0,   0,   0,   0,   0,
	0,   0,   0,   0,   0,   0,   0,   '7', '8', '9', '-', '4', '5', '6', '+', '1',
	'2', '3', '0', '.', 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	0,   0,   0,   0x5c, 0,  0,   0,   0,   0,   0,   0,   0,   0,   0x5c, 0,  0
	};
	static char keytable1[0x80] = {
	0,   0,   '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', 0x08,   0,
	'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', 0x0a,   0,   'A', 'S',
	'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~',   0,   '|', 'Z', 'X', 'C', 'V',
	'B', 'N', 'M', '<', '>', '?', 0,   '*', 0,   ' ', 0,   0,   0,   0,   0,   0,
	0,   0,   0,   0,   0,   0,   0,   '7', '8', '9', '-', '4', '5', '6', '+', '1',
	'2', '3', '0', '.', 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	0,   0,   0,   '_', 0,   0,   0,   0,   0,   0,   0,   0,   0,   '|', 0,   0
	};

	init_gdtidt();	// 初始化 gdt、idt 表
	init_pic();  // 初始化 pic 控制器
	io_sti();

	fifo32_init(&fifo, 128, fifobuf, 0);
	*((int *) 0x0fec) = (int) &fifo;
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
	key_win = open_console(shtctl, memtotal);
	/* sht_mouse */
	sht_mouse = sheet_alloc(shtctl); // sheets0[1]
	sheet_setbuf(sht_mouse, buf_mouse, 16, 16, 99); // 鼠标像素阵 16 x 16，给予最不透明度
    init_mouse_cursor8(buf_mouse, 99); // 背景色 99 号，往mouse中填入颜色和透明度
	mx = (binfo->scrnx - 16) / 2; /* 计算画面的中心坐标*/
	my = (binfo->scrny - 28 - 16) / 2;
    sheet_slide(sht_back, 0, 0); // 将背景页面不移动，直接覆盖整个画面
	sheet_slide(key_win, 80, 92);
	sheet_slide(sht_mouse, mx, my); // 移动鼠标到画面中心
	sheet_updown(sht_back, 0);
	sheet_updown(key_win, 1);
	sheet_updown(sht_mouse, 2);
	keywin_on(key_win);
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
			// fifo为空时, 当存在搁置的绘图操作时候立即执行
 			if (new_mx >= 0) {
				io_sti();
				sheet_slide(sht_mouse, new_mx, new_my);
				new_mx = -1;
			} else if (new_wx != 0x7fffffff) {
				io_sti();
				sheet_slide(sht, new_wx, new_wy);
				new_wx = 0x7fffffff;
			} else {
				task_sleep(task_a);
				io_sti();
			}
		} else {
			i = fifo32_get(&fifo);
			io_sti();
			if(key_win->flags == 0 && key_win != 0) { // 输入窗口被关闭
				if(shtctl->top == 1) { // 当画面只剩鼠标和背景时
					key_win = 0;
				} else {
					key_win = shtctl->sheets[shtctl->top - 1];
					keywin_on(key_win);
				}
			}
			if (256 <= i && i <= 511) {
				// sprintf(s, "%02X", i - 256);
				// putfonts8_asc_sht(sht_back, 0, 16, COL8_FFFFFF, COL8_840084, s, 2);
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
				if(s[0] != 0 && key_win != 0) // 一般字符
				{
					fifo32_put(&key_win->task->fifo, s[0] + 256);
				}
				if(i == 256 + 0x0f && key_win != 0) { // tab键
				    keywin_off(key_win);
				    j = key_win->height - 1;
				    if (j == 0) {
						j = shtctl->top - 1;
					}
					key_win = shtctl->sheets[j];
					keywin_on(key_win);
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
				if (i == 256 + 0x3b && key_shift != 0 && key_win != 0) { // shift + F1强制关闭
					task = key_win->task;
					if(task != 0 && task->tss.ss0 != 0) {
						cons_putstr0(task->cons, "\nBreak(key).\n");
						io_cli(); // 不能再改变寄存器的值时候进行切换
						task->tss.eax = (int) &(task->tss.esp0);
						task->tss.eip = (int) asm_end_app;
						io_sti();
						task_run(task, -1, 0);
					}
				}
				if (i == 256 + 0x3c && key_shift != 0) {	/* Shift+F2 */
					if (key_win != 0) {
						keywin_off(key_win);
					}
					keywin_off(key_win);
					key_win = open_console(shtctl, memtotal);
					sheet_slide(key_win, 92, 100);
					sheet_updown(key_win, shtctl->top);
					keywin_on(key_win);
				}
				if (i == 256 + 0x5b && shtctl->top > 2) {
					sheet_updown(shtctl->sheets[1], shtctl->top - 1);
				}
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
					new_mx = mx;
					new_my = my;
					if ((mdec.btn & 0x01) != 0) // 按下左键
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
											keywin_off(key_win);
											key_win = sht;
											keywin_on(key_win);
										}
										if(3 <= x && x < sht->bxsize - 3 && 3 <= y && y < 21) {
											// 窗口移动模式
											mmx = mx;
											mmy = my;
											mmx2 = sht->vx0;
											new_wy = sht->vy0;
										}
										if(sht->bxsize - 21 <= x && x < sht->bxsize - 5 && 5 <= y && y < 19) {

											if ((sht->flags & 0x10) != 0) {
												task = sht->task;
												cons_putstr0(task->cons, "\nBreak(mouse) . \n");
												io_cli();
												task->tss.eax = (int) & (task->tss.esp0);
												task->tss.eip = (int) asm_end_app;
												 io_sti();
												 task_run(task, -1, 0);
											} else {
												task = sht->task;
												io_cli();
												fifo32_put(&task->fifo, 4);
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
							new_wx = (mmx2 + x + 2) & ~3;
							new_wy = new_wy + y;
						    mmy = my;
					    }
					} else {
						// 没有按下左键
						mmx = -1;
						if (new_wx != 0x7fffffff) {
							sheet_slide(sht, new_wx, new_wy);
							new_wx = 0x7fffffff;
						}
					}
				}				
	   		} else if (768 <= i && i <= 1023) {
				close_console(shtctl->sheets0 + (i - 768));
			} else if (1024 <= i && i <= 2023) {
				close_constask(taskctl->tasks0 + (i - 1024));
			}
		}
	}
}

// 控制窗口标题栏的颜色和task_a的光标
void keywin_off(struct SHEET *key_win)
{
	change_wtitle8(key_win, 0);
	if ((key_win->flags & 0x20) != 0) {
		fifo32_put(&key_win->task->fifo, 3); /* 命令行窗口光标off */
	}
	return;
}

void keywin_on(struct SHEET *key_win)
{
	change_wtitle8(key_win, 1);
	if ((key_win->flags & 0x20) != 0) {
		fifo32_put(&key_win->task->fifo, 2); /* 命令行窗口光标ON */
    }
	return;
}

struct TASK *open_constask(struct SHEET *sht, unsigned int memtotal)
{
	struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
	struct TASK *task = task_alloc();
	int *cons_fifo = (int *) memman_alloc_4k(memman, 128 * 4);
	task->cons_stack = memman_alloc_4k(memman, 64 * 1024);
	task->tss.esp = task->cons_stack + 64 * 1024 - 12;
	task->tss.eip = (int) &console_task;
	task->tss.es = 1 * 8;
	task->tss.cs = 2 * 8;
	task->tss.ss = 1 * 8;
	task->tss.ds = 1 * 8;
	task->tss.fs = 1 * 8;
	task->tss.gs = 1 * 8;
	*((int *) (task->tss.esp + 4)) = (int) sht;
	*((int *) (task->tss.esp + 8)) = memtotal;
	task_run(task, 2, 2); /* level = 2, priority = 2 */
	fifo32_init(&task->fifo, 128, cons_fifo, task);
	return task;
}

struct SHEET *open_console(struct SHTCTL *shtctl, unsigned int memtotal)
{
	struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
	struct SHEET *sht = sheet_alloc(shtctl);
	unsigned char *buf = (unsigned char *) memman_alloc_4k(memman, 768 * 560);
	struct TASK *task = task_alloc();
	int *cons_fifo = (int *) memman_alloc_4k(memman, 128 * 4);
	sheet_setbuf(sht, buf, 768, 560, -1); /* 无透明色 */
	make_window8(buf, 768, 560, "console", 0);
	make_textbox8(sht, 8, 28, 752, 520, COL8_000000);
	sht->task = open_constask(sht, memtotal);
	sht->flags |= 0x20;	/* 有光标 */
	return sht;
}

void close_constask(struct TASK *task)
{
	struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
	task_sleep(task);
	memman_free_4k(memman, task->cons_stack, 64 * 1024);
	memman_free_4k(memman, (int) task->fifo.buf, 128 * 4);
	task->flags = 0; // 用于代替task_free
	return;
}

void close_console(struct SHEET *sht)
{
	struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
	struct TASK *task = sht->task;
	memman_free_4k(memman, (int) sht->buf, 768 * 560);
	sheet_free(sht);
	close_constask(task);
	return;
}








