#include "bootpack.h"
#include <string.h>
#include <stdio.h>

void console_task(struct SHEET *sheet, unsigned int memtotal)
{
	struct TIMER *timer;
	struct TASK *task = task_now();
	struct CONSOLE cons;
	struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
	int i, fifobuf[128], *fat = (int *) memman_alloc_4k(memman, 4 * 2880);
	char cmdline[30];
	cons.sht = sheet;
	cons.cur_x = 8;
	cons.cur_y = 28;
	cons.cur_c = -1;
	*((int *) 0x0fec) = (int) &cons;

	fifo32_init(&task->fifo, 128, fifobuf, task);
	timer = timer_alloc();
	timer_init(timer, &task->fifo, 1);
	timer_settime(timer, 50);
	file_readfat(fat, (unsigned char *) (ADR_DISKIMG + 0x000200));

    // 显示提示符
	cons_putchar(&cons, '>', 1);

	for(;;) {
		io_cli();
		if (fifo32_status(&task->fifo) == 0){
			task_sleep(task);
			io_sti();
		} else {
			i = fifo32_get(&task->fifo);
			io_sti();
			if (i <= 1) { // 光标用的定时器
			    if (i != 0) {
					timer_init(timer, &task->fifo, 0); // 下次置于 0
					if (cons.cur_c >= 0) {
						cons.cur_c = COL8_FFFFFF;
					}
				} else {
					timer_init(timer, &task->fifo, 1); // 下次置于 1
					if (cons.cur_c >= 0) {
						cons.cur_c = COL8_000000;
					}					
				}
				timer_settime(timer, 50);
			}
			if (i == 2) {
				cons.cur_c = COL8_FFFFFF;
			}
			if (i == 3) {
				boxfill8(sheet->buf, sheet->bxsize, COL8_000000, cons.cur_x, cons.cur_y, cons.cur_x + 7, cons.cur_y + 15);
				cons.cur_c = -1;
			}
			if (256 <= i && i <= 511) { // 键盘数据通过任务a
				if (i == 8 + 256) { // 退格键
					if(cons.cur_x > 16) {
						// 用空白擦除光标后，光标前移一位
						cons_putchar(&cons, ' ', 0);
						cons.cur_x -= 8;
					}
				} else if (i == 10 + 256) { /* 回车键 */
					cons_putchar(&cons, ' ', 0);
					cmdline[cons.cur_x / 8 - 2] = 0;
					cons_newline(&cons);
					cons_runcmd(cmdline, &cons, fat, memtotal, task); // 执行命令
					cons_putchar(&cons, '>', 1);
				} else {
					// 一般字符
					if(cons.cur_x < 752) {
						// 显示一个字符后光标后移
						cmdline[cons.cur_x / 8 - 2] = i - 256;
						cons_putchar(&cons, i - 256, 1);
					}
				}
			}
			if (cons.cur_c >= 0) {
				boxfill8(sheet->buf, sheet->bxsize, cons.cur_c, cons.cur_x, cons.cur_y, cons.cur_x + 7, cons.cur_y + 15);
			}
			sheet_refresh(sheet, cons.cur_x, cons.cur_y, cons.cur_x + 8, cons.cur_y + 16);
		}
	}
}

void cons_putchar(struct CONSOLE *cons, int chr, char move) {
	char s[2];
	s[0] = chr;
	s[1] = 0;
	if (s[0] == 0x09) {	/* 制表符 */
		for (;;) {
			putfonts8_asc_sht(cons->sht, cons->cur_x, cons->cur_y, COL8_FFFFFF, COL8_000000, " ", 1);
			cons->cur_x += 8;
			if (cons->cur_x == 8 + 752) {
				cons_newline(cons);
			}
			if (((cons->cur_x - 8) & 0x1f) == 0) { // 减 8 是因为边框还要 8 个宽度，每个制表位相隔 4 个字符
				break;	/* 被 32 整除则break*/
			}
		}
	} else if (s[0] == 0x0a) { // 换行
		cons_newline(cons);
	} else if (s[0] == 0x0d) { 
		// 回车，则不做任何操作
	} else {
		putfonts8_asc_sht(cons->sht, cons->cur_x, cons->cur_y, COL8_FFFFFF, COL8_000000, s, 1);
		if (move != 0) {
			cons->cur_x += 8;
			if (cons->cur_x == 8 + 752) {
				cons_newline(cons);
			}
		}
	}
	return;
}

// 用于换行和滚动的
void cons_newline(struct CONSOLE *cons)
{
	int x, y;
	struct SHEET *sheet = cons->sht;
	if (cons->cur_y < 28 + 496) {
		cons->cur_y += 16; // 换行
	} else {
		/* 滚动 */
		for (y = 28; y < 28 + 496; y++) {
			for (x = 8; x < 8 + 752; x++) {
				sheet->buf[x + y * sheet->bxsize] = sheet->buf[x + (y + 16) * sheet->bxsize];
			}
		}
		for (y = 28 + 496; y < 28 + 512; y++) {
			for (x = 8; x < 8 + 752; x++) {
				sheet->buf[x + y * sheet->bxsize] = COL8_000000;
			}
		}
		sheet_refresh(sheet, 8, 28, 8 + 752, 28 + 512);
	}
	cons->cur_x = 8;
	return;
}

void cons_runcmd(char *cmdline, struct CONSOLE *cons, int *fat, unsigned int memtotal, struct TASK *task)
{
	if (strcmp(cmdline, "mem") == 0) {
		cmd_mem(cons, memtotal);
	} else if (strcmp(cmdline, "cls") == 0) {
		cmd_cls(cons);
	} else if (strcmp(cmdline, "ls") == 0) {
		cmd_ls(cons);
	} else if (strcmp(cmdline, "task") == 0) {
		cmd_task(cons, task);
	} else if (strncmp(cmdline, "cat ", 4) == 0) {
		cmd_cat(cons, fat, cmdline);
	} else if (cmdline[0] != 0) {
		if (cmd_app(cons, fat, cmdline) == 0) {
		// 不是命令也不是空行
		cons_putstr0(cons, "Bad command.\n\n");
		}
	}
	return;
}

void cmd_mem(struct CONSOLE *cons, unsigned int memtotal) {
	// mem命令
	struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
	char s[60];
	sprintf(s, "total   %dMB\nfree %dKB\n\n", memtotal / (1024 * 1024), memman_total(memman) / 1024);
	cons_putstr0(cons, s);
	return;
}

void cmd_cls(struct CONSOLE *cons) {
	int x, y;
	struct SHEET *sheet = cons->sht;
	for (y = 28; y < 28 + 520; y++) {
		for (x = 8; x < 8 + 752; x++) {
			sheet->buf[x + y * sheet->bxsize] = COL8_000000;
		}
	}
	sheet_refresh(sheet, 8, 28, 8 + 752, 28 + 520);
	cons->cur_y = 28;
	return;
}

void cmd_ls(struct CONSOLE *cons) {
	// ls
	struct FILEINFO *finfo = (struct FILEINFO *) (ADR_DISKIMG + 0x002600);
	int i, j;
	char s[30];
	for (i = 0; i < 244; i++) { // 最多有 244 个文件名信息
		if (finfo[i].name[0] == 0x00) { // 无任何文件信息时
			break;
		}
		if (finfo[i].name[0] != 0xe5) { // 如果文件没被删除
			if ((finfo[i].type & 0x18) == 0) { // 只要不是非文件信息或者目录就显示！！
				sprintf(s, "filename.ext   %7d KB\n", finfo[i].size);
				for (j = 0; j < 8; j++) {
					s[j] = finfo[i].name[j];
				}
				s[9] = finfo[i].ext[0];
				s[10] = finfo[i].ext[1];
				s[11] = finfo[i].ext[2];
				cons_putstr0(cons, s);
			}
		}
	}
	cons_newline(cons);
	return;
}

void cmd_cat(struct CONSOLE *cons, int *fat, char *cmdline) {
	struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
	struct FILEINFO *finfo = file_search(cmdline + 4, (struct FILEINFO *) (ADR_DISKIMG + 0x002600), 224);
	char *p;
	if (finfo != 0) {
	// 找到文件
		p = (char *) memman_alloc_4k(memman, finfo->size);
		file_loadfile(finfo->clustno, finfo->size, p, fat, (char *) (ADR_DISKIMG + 0x003e00));
		cons_putstr1(cons, p, finfo->size);
		memman_free_4k(memman, (int) p, finfo->size);
	} else {
		// 没找到的情况
		cons_putstr0(cons, "File can't be found.\n");
	}
	cons_newline(cons);
	return;
}

int cmd_app(struct CONSOLE *cons, int *fat, char *cmdline) {
	struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
	struct FILEINFO *finfo;
	struct SEGMENT_DESCRIPTOR *gdt = (struct SEGMENT_DESCRIPTOR *) ADR_GDT;
	int i;
	char name[18], *p, *q;
	struct TASK *task = task_now();

	/* 根据命令行生成文件名 */
	for (i = 0; i < 13; i++) {
		if (cmdline[i] <= ' ') {
			break;
		}
		name[i] = cmdline[i];
	}
	name[i] = 0; /* 暂且将文件名的后面置为 0 */

	/* 寻找文件 */
	finfo = file_search(name, (struct FILEINFO *) (ADR_DISKIMG + 0x002600), 224);
	if (finfo == 0 && name[i - 1] != '.') {
		/* 由于找不到文件，故在文件名后面加上 “.hrb” 后重新寻找 */
		name[i] = '.';
		name[i + 1] = 'H';
		name[i + 2] = 'R';
		name[i + 3] = 'B';
		name[i + 4] = 0;
		finfo = file_search(name, (struct FILEINFO *) (ADR_DISKIMG + 0x002600), 224);
	}

	if (finfo != 0) {
		/* 找到文件的情况 */
		p = (char *) memman_alloc_4k(memman, finfo->size);
		q = (char *) memman_alloc_4k(memman, 64 * 1024);// 分配 64 KB给应用程序
		*((int *) 0xfe8) = (int) p;
		file_loadfile(finfo->clustno, finfo->size, p, fat, (char *) (ADR_DISKIMG + 0x003e00));
		set_segmdesc(gdt + 1003, finfo->size - 1, (int) p, AR_CODE32_ER + 0x60);
		set_segmdesc(gdt + 1004,    64 * 1024 -1, (int) q, AR_DATA32_RW + 0x60);
		if (finfo->size >= 8 && strncmp(p + 4, "Hari", 4) == 0) {
			p[0] = 0xe8;
			p[1] = 0x16;
			p[2] = 0x00;
			p[3] = 0x00;
			p[4] = 0x00;
			p[5] = 0xcb;
		}
		start_app(0, 1003 * 8, 64 * 1024, 1004 * 8, &(task->tss.esp0));
		memman_free_4k(memman, (int) p, finfo->size);
		memman_free_4k(memman, (int) q, 64 * 1024);
		cons_newline(cons);
		return 1;
	}
	/* 没有找到文件的情况 */
	return 0;
}

void cmd_task(struct CONSOLE *cons, struct TASK *task) {
	//level
	char s[30];
	sprintf(s, "level:%d", task->level);
	putfonts8_asc_sht(cons->sht, 8, cons->cur_y, COL8_FFFFFF, COL8_000000, s, 7);
	cons_newline(cons);
	return;
}

void cons_putstr0(struct CONSOLE *cons, char *s)
{
	for (; *s != 0; s++) {
		cons_putchar(cons, *s, 1);
	}
	return;
}

void cons_putstr1(struct CONSOLE *cons, char *s, int l)
{
	int i;
	for (i = 0; i < l; i++) {
		cons_putchar(cons, s[i], 1);
	}
	return;
}

// 用edx来指向是哪一号的子功能
int *hrb_api(int edi, int esi, int ebp, int esp, int ebx, int edx, int ecx, int eax)
{
	int cs_base = *((int *) 0xfe8);
	struct TASK *task = task_now();
	struct CONSOLE *cons = (struct CONSOLE *) *((int *) 0x0fec); // 前面讲cons的内容保存到了 0xfec 当中
	if (edx == 1) {
		cons_putchar(cons, eax & 0xff, 1);
	} else if (edx == 2) {
		cons_putstr0(cons, (char *) ebx + cs_base);
	} else if (edx == 3) {
		cons_putstr1(cons, (char *) ebx + cs_base, ecx);
	} else if (edx == 4) {
		return &(task->tss.esp0);
	}
	return 0;
}

int *inthandler0d(int *esp)
{
	struct CONSOLE *cons = (struct CONSOLE *) *((int *) 0x0fec);
	struct TASK *task = task_now();
	cons_putstr0(cons, "\nINT 0D :\n General Protected Exception.\n");
	return &(task->tss.esp0);	/* 强制结束程序 */
}




