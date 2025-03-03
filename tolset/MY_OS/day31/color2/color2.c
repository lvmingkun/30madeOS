#include "apilib.h"

unsigned char rgb2pal(int r, int g, int b, int x, int y);

void HariMain(void)
{
	char *buf;
	int win, x, y;
	api_initmalloc();
	buf = api_malloc(144 * 164);
	win = api_openwin(buf, 144, 164, -1, "color2");
	for (y = 0; y < 128; y++) {
		for (x = 0; x < 128; x++) {
			buf[(x + 8) + (y + 28) * 144] = rgb2pal(x * 2, y * 2, 0, x, y);
		}
	}
	api_refreshwin(win, 8, 28, 136, 156);
	api_getkey(1); /* 等待按下任意键 */
	api_end();
}

unsigned char rgb2pal(int r, int g, int b, int x, int y)
{
	static int table[4] = { 3, 1, 0, 2 };
	int i;
	x &= 1; /* 偶数还是奇数 */
	y &= 1;
	i = table[x + y * 2];	/* 生成中间色的常量 */
	r = (r * 21) / 256;	/* r = 0 ~ 21；  这里是除开第一个和最后一个像素点外，每隔 6 个像素点就换一个色号 */
	g = (g * 21) / 256;
	b = (b * 21) / 256;
	r = (r + i) / 4; /* r = 0 ~ 5； 这里的为了服务那 21 中色号的，就利用我们所说的原理，用初始的 6 个色号进行排列，以此显示出 21 中不同的色号 */
	g = (g + i) / 4;
	b = (b + i) / 4;
	return 16 + r + g * 6 + b * 36;
}
