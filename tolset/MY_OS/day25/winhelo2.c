int api_openwin(char *buf, int xsiz, int ysiz, int col_inv, char *title);
void api_putstrwin(int win, int x, int y, int col, int len, char *str);
void api_boxfilwin(int win, int x0, int y0, int x1, int y1, int col);
void api_end(void);
int api_getkey(int mode);
void api_closewin(int win);

char buf[150 * 50];

void HariMain(void)
{
	int win;
	win = api_openwin(buf, 150, 50, -1, "window2");
	api_boxfilwin(win,  8, 36, 141, 43, 3 /* 黄色 */);
	api_putstrwin(win, 28, 28, 0 /* 黑色 */, 12, "hello, world");
	for(;;) {
		if (api_getkey(1) == 0x0a) // api_getkey里可以通过EAX进行将值返回给应用程序的！0x0a 就是回车符
		{
			break;
		}
	}
	api_closewin(win);
	api_end();
}

