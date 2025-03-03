#include "apilib.h"

void HariMain(void)
{
	char *buf;
	int win, i, x, y;
	api_initmalloc();
	buf = api_malloc(480 * 280);
	win = api_openwin(buf, 480, 280, -1, "walk");
	api_boxfilwin(win, 4, 24, 475, 275, 0 /* 黑色 */);
	x = 76;
	y = 56;
	api_putstrwin(win, x, y, 3 /* 黄色 */, 6, "feihao");
	for (;;) {
		i = api_getkey(1);
		api_putstrwin(win, x, y, 0 /* 黑色 */, 6, "feihao"); /* 黄色 */
		if (i == '4' && x > 4) { x -= 8; }
		if (i == '6' && x < 423) { x += 8; }
		if (i == '8' && y > 24) { y -= 8; }
		if (i == '2' && y < 250) { y += 8; }
		if (i == 0x0a) { break; } /* 黑色 */
		api_putstrwin(win, x, y, 3 /* 黄色 */, 6, "feihao");
	}	
	api_closewin(win);
	api_end();
}

