int api_openwin(char *buf, int xsiz, int ysiz, int col_inv, char *title);
void api_end(void);
int api_getkey(int mode);
void api_closewin(int win);

char buf[150 * 50];

void HariMain(void)
{
	int win;
	win = api_openwin(buf, 150, 50, -1, "window_FH");
	for(;;) {
		if (api_getkey(1) == 0x0a) // api_getkey里可以通过EAX进行将值返回给应用程序的！0x0a 就是回车符
		{
			break;
		}
	}
	api_closewin(win);
	api_end();
}
