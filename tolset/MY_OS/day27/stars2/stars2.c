#include "apilib.h"

int rand(void);

void HariMain(void)
{
	char *buf;
	int win, i, x, y;
	api_initmalloc();
	buf = api_malloc(150 * 100);
	win = api_openwin(buf, 150, 100, -1, "stars2");
	api_boxfilwin(win + 1,  6, 26, 143, 93, 0);
	for (i = 0; i < 50; i++) {
		x = (rand() % 137) + 6;
		y = (rand() % 67) + 26;
		api_point(win + 1, x, y, 3);
	}
	api_refreshwin(win, 6, 26, 144, 94);
	for(;;) {
		if (api_getkey(1) == 0x0a) // api_getkey里可以通过EAX进行将值返回给应用程序的！0x0a 就是回车符
		{
			break;
		}
	}
	api_closewin(win);
	api_end();
}

