#include "apilib.h"

void HariMain(void)
{
	char *buf;
	int win, i;
	api_initmalloc();
	buf = api_malloc(160 * 100);
	win = api_openwin(buf, 160, 100, -1, "lines");
	for (i = 0; i < 8; i++) {
		api_linewin(win + 1,  8, 26, 77, i * 9 + 26, i);
		api_linewin(win + 1, 88, 26, i * 9 + 88, 89, i);
	}
	api_refreshwin(win, 6, 26, 154, 90);
	for(;;) {
		if (api_getkey(1) == 0x0a) // api_getkey里可以通过EAX进行将值返回给应用程序的！0x0a 就是回车符
		{
			break;
		}
	}
	api_closewin(win);
	api_end();
}
