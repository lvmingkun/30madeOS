int api_openwin(char *buf, int xsiz, int ysiz, int col_inv, char *title);
void api_boxfilwin(int win, int x0, int y0, int x1, int y1, int col);
void api_initmalloc(void);
char *api_malloc(int size);
void api_point(int win, int x, int y, int col);
void api_end(void);
int api_getkey(int mode);
void api_closewin(int win);

void HariMain(void)
{
	char *buf;
	int win;
	api_initmalloc();
	buf = api_malloc(150 * 100);
	win = api_openwin(buf, 150, 100, -1, "star1");
	api_boxfilwin(win,  6, 26, 143, 93, 0 /* 黑色 */);
	api_point(win, 75, 59, 3 /* 黄色 */);
	for(;;) {
		if (api_getkey(1) == 0x0a) // api_getkey里可以通过EAX进行将值返回给应用程序的！0x0a 就是回车符
		{
			break;
		}
	}
	api_closewin(win);
	api_end();
}

