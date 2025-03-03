#include "apilib.h"

void HariMain(void)
{
	int i, timer;
	timer = api_alloctimer();
	api_inittimer(timer, 128);
	for (i = 20000000; i >= 20000; i -= i / 100) {
		// 20 KHz ~ 20 HZ, 人类能听到的范围
        // i以 1 % 速度递减
		api_beep(i);
		api_settimer(timer, 1);		/* 0.01 */
		if (api_getkey(1) != 128) {
			break;
		}
	}
	api_beep(0);
	api_end();
}
