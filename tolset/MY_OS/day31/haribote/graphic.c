#include "bootpack.h"

void init_palette(void)
{
	static unsigned char table_rgb[16 * 3] = {
		0x00, 0x00, 0x00,
		0xff, 0x00, 0x00,
		0x00, 0xff, 0x00,
		0xff, 0xff, 0x00,
		0x00, 0x00, 0xff,
		0xff, 0x00, 0xff,
		0x00, 0xff, 0xff,
		0xff, 0xff, 0xff,
		0xc6, 0xc6, 0xc6,
		0x84, 0x00, 0x00,
		0x00, 0x84, 0x00,
		0x84, 0x84, 0x00,
		0x00, 0x00, 0x84,
		0x84, 0x00, 0x84,
		0x00, 0x84, 0x84,
		0x84, 0x84, 0x84
	};
	unsigned char table2[216 * 3];
	int r, g, b;
	set_palette(0, 15, table_rgb);
	for (b = 0; b < 6; b++) {
		for (g = 0; g < 6; g++) {
			for (r = 0; r < 6; r++) {
				table2[(r + g * 6 + b * 36) * 3 + 0] = r * 51;
				table2[(r + g * 6 + b * 36) * 3 + 1] = g * 51;
				table2[(r + g * 6 + b * 36) * 3 + 2] = b * 51;
			}
		}
	}
	set_palette(16, 231, table2);
	return;
}

void set_palette(int start, int end, unsigned char *rgb)
{
	int i, eflags;
	eflags = io_load_eflags();
	io_cli();
	io_out8(0x03c8, start);
	for (i = start; i <= end; i++) {
		io_out8(0x03c9, rgb[0] / 4);
		io_out8(0x03c9, rgb[1] / 4);
		io_out8(0x03c9, rgb[2] / 4);
		rgb += 3;
	}
	io_store_eflags(eflags);
	return;
}

void boxfill8(unsigned char *vram, int xsize, unsigned char c, int x0, int y0, int x1, int y1)
{
	int x, y;
	for (y = y0; y <= y1; y++) {
		for (x = x0; x <= x1; x++)
			vram[y * xsize + x] = c;
	}
	return;
}

void init_screen8(char *vram, int x, int y)
{
	boxfill8(vram, x, COL8_840084,  0,     0,      x -  1, y - 29);
	boxfill8(vram, x, COL8_C6C6C6,  0,     y - 28, x -  1, y - 28);
	boxfill8(vram, x, COL8_FFFFFF,  0,     y - 27, x -  1, y - 27);
	boxfill8(vram, x, COL8_C6C6C6,  0,     y - 26, x -  1, y -  1);

	boxfill8(vram, x, COL8_FFFFFF,  3,     y - 24, 59,     y - 24);
	boxfill8(vram, x, COL8_FFFFFF,  2,     y - 24,  2,     y -  4);
	boxfill8(vram, x, COL8_848484,  3,     y -  4, 59,     y -  4);
	boxfill8(vram, x, COL8_848484, 59,     y - 23, 59,     y -  5);
	boxfill8(vram, x, COL8_000000,  2,     y -  3, 59,     y -  3);
	boxfill8(vram, x, COL8_000000, 60,     y - 24, 60,     y -  3);

	boxfill8(vram, x, COL8_848484, x - 47, y - 24, x -  4, y - 24);
	boxfill8(vram, x, COL8_848484, x - 47, y - 23, x - 47, y -  4);
	boxfill8(vram, x, COL8_FFFFFF, x - 47, y -  3, x -  4, y -  3);
	boxfill8(vram, x, COL8_FFFFFF, x -  3, y - 24, x -  3, y -  3);
	return;
}

void putfont8(char *vram, int xsize, int x, int y, char c, char *font)
{
	int i;
	char *p, d /* data */;
	for (i = 0; i < 16; i++) {               // å­—ç?¦åƒç´ ç‚¹é˜µçš„è¡Œæ•°å¾?çŽ? 
		p = vram + (y + i) * xsize + x;      // å­—ç?¦åƒç´ ç‚¹é˜µçš„å¼€å§‹ä½ç½?
		d = font[i];
		if ((d & 0x80) != 0) { p[0] = c; }   // pä»£è¡¨ç€å­—ç?¦åƒç´ ç‚¹é˜µä¸€è¡Œçš„é‚? 8 ä½?
		if ((d & 0x40) != 0) { p[1] = c; }
		if ((d & 0x20) != 0) { p[2] = c; }
		if ((d & 0x10) != 0) { p[3] = c; }
		if ((d & 0x08) != 0) { p[4] = c; }
		if ((d & 0x04) != 0) { p[5] = c; }
		if ((d & 0x02) != 0) { p[6] = c; }
		if ((d & 0x01) != 0) { p[7] = c; }
	}
	return;
}

void putfonts8_asc(char *vram, int xsize, int x, int y, char c, unsigned char *s)  // sæŒ‡å‘å­—ç?¦ä¸²å¼€å¤´çš„ä½ç½®ï¼Œå› æ­¤å¯ä»¥ç›´æŽ¥ä½¿ç”¨sè¿›è?Œè?»å–å­—ç?¦ä¸²çš„æ¯ä¸€ä¸?å•ç‹¬å­—ç?¦ï¼› ä»?asciiç¼–ç 
{
	extern char hankaku[4096];
	struct TASK *task = task_now();
	char *nihongo = (char *) *((int *) 0x0fe8), *font;
	int k, t;

	if (task->langmode == 0) {
		for (; *s != 0x00; s++) {
			putfont8(vram, xsize, x, y, c, hankaku + * s * 16); // cè¯?è¨€ä¸?ï¼Œå‡½æ•°éƒ½æ˜?ä»? 0x00 ç»“å°¾çš„ï¼ï¼?
			x += 8;
		}
	}
	if (task->langmode == 1) {
		for (; *s != 0x00; s++) {
			if (task->langbyte1 == 0) {
				if ((0x81 <= *s && *s <= 0x9f) || (0xe0 <= *s && *s <= 0xfc)) {
					task->langbyte1 = *s;
				} else {
					putfont8(vram, xsize, x, y, c, nihongo + *s * 16);
				}
			} else {
				if (0x81 <= task->langbyte1 && task->langbyte1 <= 0x9f) {
					k = (task->langbyte1 - 0x81) * 2;
				} else {
					k = (task->langbyte1 - 0xe0) * 2 + 62;
				}
				if (0x40 <= *s && *s <= 0x7e) {
					t = *s - 0x40;
				} else if (0x80 <= *s && *s <= 0x9e) {
					t = *s - 0x80 + 63;
				} else {
					t = *s - 0x9f;
					k++;
				}
				task->langbyte1 = 0;
				font = nihongo + 256 * 16 + (k * 94 + t) * 32;
				putfont8(vram, xsize, x - 8, y, c, font     );
				putfont8(vram, xsize, x    , y, c, font + 16);
			}
			x += 8;
		}
	}
	if (task->langmode == 2) {
		for (; *s != 0x00; s++) {
			if (task->langbyte1 == 0) {
				if (0x81 <= *s && *s <= 0xfe) {
					task->langbyte1 = *s;
				} else {
					putfont8(vram, xsize, x, y, c, nihongo + *s * 16);
				}
			} else {
				k = task->langbyte1 - 0xa1;
				t = *s - 0xa1;
				task->langbyte1 = 0;
				font = nihongo + 256 * 16 + (k * 94 + t) * 32;
				putfont8(vram, xsize, x - 8, y, c, font     );
				putfont8(vram, xsize, x    , y, c, font + 16);
			}
			x += 8;
		}
	}
	if (task->langmode == 3) {
		for (; *s != 0x00; s++) {
			if (task->langbyte1 == 0) {
				if (0xa1 <= *s && *s <= 0xfe) {
					task->langbyte1 = *s;
				} else {
					putfont8(vram, xsize, x, y, c, hankaku + *s * 16); // Ö»ÒªÊÇ°ë½Ç¾ÍÊ¹ÓÃhankakuÀïÃæµÄ×Ö·û
				}
			} else {
				k = task->langbyte1 - 0xa1;
				t = *s - 0xa1;
				task->langbyte1 = 0;
				font = nihongo + (k * 94 + t) * 32;
				putfont32(vram,xsize, x - 8, y, c, font, font + 16);
			}
			x += 8;
		}
	}
	return;
}

void init_mouse_cursor8(char *mouse, char bc)
{
	static char cursor[16][16] = {
		"**************..",
		"*OOOOOOOOOOO*...",
		"*OOOOOOOOOO*....",
		"*OOOOOOOOO*.....",
		"*OOOOOOOO*......",
		"*OOOOOOO*.......",
		"*OOOOOOO*.......",
		"*OOOOOOOO*......",
		"*OOOO**OOO*.....",
		"*OOO*..*OOO*....",
		"*OO*....*OOO*...",
		"*O*......*OOO*..",
		"**........*OOO*.",
		"*..........*OOO*",
		"............*OO*",
		".............***"
	};
	int x, y;

	for (y = 0; y < 16; y++) {
		for (x = 0; x < 16; x++) {
			if (cursor[y][x] == '*') {
				mouse[y * 16 + x] = COL8_000000;
			}
			if (cursor[y][x] == 'O') {
				mouse[y * 16 + x] = COL8_FFFFFF;
			}
			if (cursor[y][x] == '.') {
				mouse[y * 16 + x] = bc;
			}
		}
	}
	return;
}

void putblock8_8(char *vram, int vxsize, int pxsize, int pysize, int px0, int py0, char *buf, int bxsize)
 	  /* å°†é¼ æ ‡çš„èƒŒæ™¯è‰²æ˜¾ç¤ºå‡ºæ¥ï¼›
	  vramä¸Žvxsizeæ˜?å…³äºŽVRAMä¿¡æ¯çš„ï¼Œå€¼åˆ†åˆ?æ˜? 0xa000 ä¸? 320;
	  pxsizeä¸Žpysizeæ˜?æ˜¾ç¤ºå›¾å½¢çš„å¤§å°ï¼Œå³é¼ æ ‡åƒç´ é˜µï¼?16 x 16;
	  px0ä¸Žpy0æ˜?æŒ‡å®šå›¾å½¢åœ¨ç”»é?ä¸Šæ˜¾ç¤ºçš„ä½ç½®;
	  bufä¸Žbxsizeæ˜?æŒ‡å®šå›¾å½¢çš„å­˜æ”¾åœ°å€å’Œæ¯ä¸€è¡Œå«æœ‰çš„åƒç´ æ•°ï¼ˆä¸ºåŽé¢å‡†å¤‡ï¼Œè¿™é‡Œæ˜?ä¸? pxsize ä¸€æ ·ï¼‰
	  */
{
	int x, y;
	for (y = 0; y < pysize; y++) {
		for (x = 0; x < pxsize; x++) {
			vram[(py0 + y) * vxsize + (px0 + x)] = buf[y * bxsize + x];
		}
	}
	return;
}


void putfont32(char *vram, int xsize, int x, int y, char c, char *font1, char *font2)
{
	int i, k, j, f;
	char *p;
	j = 0;
	p = vram + (y + j) * xsize + x;
	j++;
	// ÉÏ°ë²¿·Ö
	for(i = 0; i < 16; i++)
	{
		for(k = 0; k < 8; k++)
		{
			if(font1[i] & (0x80 >> k))
			{
				p[k + (i % 2) * 8] = c;
			}
		}
		for(k = 0; k < 8 / 2; k++)
		{
			f = p[k + (i % 2) * 8];
			p[k + (i % 2) * 8] = p[8 - 1 - k + (i % 2) * 8];
			p[8 - 1 - k + (i % 2) * 8] = f;
		}
		if(i % 2)
		{
			p = vram + (y + j) * xsize + x;
			j++;
		}
	}
	// ÏÂ°ë²¿·Ö
	for(i = 0; i < 16; i++)
	{
		for(k = 0; k < 8; k++)
		{
			if(font2[i] & (0x80 >> k))
			{
				p[k + (i % 2) * 8] = c;
			}
		}
		for(k = 0; k < 8 / 2; k++)
		{
			f = p[k + (i % 2) * 8];
			p[k + (i % 2) * 8] = p[8 - 1 - k + (i % 2) * 8];
			p[8 - 1 - k + (i % 2) * 8] = f;
		}
		if(i % 2)
		{
			p = vram + (y + j) * xsize + x;
			j++;
		}
	}
	return;
}

void font_init(unsigned char mode)
{
	struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
	int *fat;
	int i;
	unsigned char *nihongo, *hzk;
	struct FILEINFO *finfo;
	extern char hankaku[4096];
    if( mode == 3)
	{
		// ÔØÈëHZK16, ÖÐÎÄ×Ö·û¼¯
		fat = (int *) memman_alloc_4k(memman, 4 * 2880);
		file_readfat(fat, (unsigned char *) (ADR_DISKIMG + 0x000200));
		finfo = file_search("HZK16.fnt", (struct FILEINFO *) (ADR_DISKIMG + 0x002600), 224);
		if (finfo != 0) {
			i = finfo->size;
			hzk = file_loadfile2(finfo->clustno, &i, fat);
		} else {
			hzk = (unsigned char *) memman_alloc_4k(memman, 0x5d5d * 32);
			for (i = 0; i < 16 * 256; i++) {
			hzk[i] = hankaku[i]; /* Ã»ÓÐ×Ö·û¿â£¬ÔòÖ±½Ó¸³ÖµÇ°ÃæµÄÓ¢ÎÄ×Ö·û¿â */
			}
			for (i = 16 * 256; i < 0x5d5d * 32; i++) {
			hzk[i] = 0xff; /* Ê£ÏÂ²¿·ÖÌî³ä0xff */
			}
		}
		*((int *) 0x0fe8) = (int) hzk; // ÓÃ0xfe8´æ×Ö¿âµØÖ·
		memman_free_4k(memman, (int) fat, 4 * 2880);
	} else if( mode >= 1 || mode <= 2)
	{
		fat = (int *) memman_alloc_4k(memman, 4 * 2880);
		file_readfat(fat, (unsigned char *) (ADR_DISKIMG + 0x000200));
		finfo = file_search("nihongo.fnt", (struct FILEINFO *) (ADR_DISKIMG + 0x002600), 224);
		if (finfo != 0) {
			i = finfo->size;
			nihongo = file_loadfile2(finfo->clustno, &i, fat);
		} else {
			nihongo = (unsigned char *) memman_alloc_4k(memman, 16 * 256 + 32 * 94 * 47);
			for (i = 0; i < 16 * 256; i++) {
				nihongo[i] = hankaku[i]; /* Ã»ÓÐ×Ö·û¿â£¬ÔòÖ±½Ó¸³ÖµÇ°ÃæµÄÓ¢ÎÄ×Ö·û¿â */
			}
			for (i = 16 * 256; i < 16 * 256 + 32 * 94 * 47; i++) {
				nihongo[i] = 0xff; /* Ê£ÏÂ²¿·ÖÌî³ä0xff */
			}
		}
		*((int *) 0x0fe8) = (int) nihongo; // ÓÃ0xfe8´æ×Ö¿âµØÖ·
		memman_free_4k(memman, (int) fat, 4 * 2880);
	}
}

