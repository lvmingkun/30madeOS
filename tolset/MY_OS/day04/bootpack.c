#define COL8_000000 0 //黑
#define COL8_FF0000 1 //亮红
#define COL8_00FF00 2 //亮绿
#define COL8_FFFF00 3 //亮黄
#define COL8_0000FF 4 //亮蓝
#define COL8_FF00FF 5 //亮紫
#define COL8_00FFFF 6 //浅亮蓝
#define COL8_FFFFFF 7 //白
#define COL8_C6C6C6 8 //亮灰
#define COL8_840000 9 //暗红
#define COL8_008400 10//暗绿
#define COL8_848400 11//暗黄
#define COL8_000084 12//暗青
#define COL8_840084 13//暗紫
#define COL8_008484 14//浅暗蓝
#define COL8_848484 15//暗灰

void io_hlt(void); //暂停
void io_cli(void); //设置IF标志寄存器为0
void io_out8( int port, int data ); //向端口写入数据，按rgb格式写入
int io_load_eflags(void);
void io_store_eflags(int eflags ) ;
void write_mem8(int addr, int data);  //向VARM内存显卡地址写入

void init_palette(void); //设定调色板
void set_palette(int start , int end, unsigned char *rgb); //向bios提供的调色板写入

void HariMain(void)
{
	// char *p ;//在汇编中变量p是一个地址本身是四字节的，但是其指向的内存单元的byte类型（因为char就是1字节）

    // init_palette();//设定调色板

    // p = (char *) 0xa0000;
    // boxfill8(p, 320, COL8_FF0000, 20,  20, 120, 120);
    // boxfill8(p, 320, COL8_00FF00, 70,  50, 170, 150);
    // boxfill8(p, 320, COL8_0000FF, 120, 80, 220, 180);

    char *vram;
	int xsize, ysize;

    init_palette();//设定调色板

    vram = (char *) 0xa0000 ;
    xsize = 320;
    ysize = 200;

    boxfill8(vram, xsize, COL8_008484,	 0,		0,		 	xsize-1,	ysize-29); //设置上半部分为浅暗蓝
    boxfill8(vram, xsize, COL8_C6C6C6,	 0,		ysize-28,	xsize-1,	ysize-28); //设置框的第一条线是亮灰
    boxfill8(vram, xsize, COL8_FFFFFF,	 0,		ysize-27,	xsize-1,	ysize-27); //设置框的第二条线是白
    boxfill8(vram, xsize, COL8_C6C6C6,	 0,		ysize-26,	xsize-1,	ysize- 1); //设置框的剩下部分是亮灰

    boxfill8(vram, xsize, COL8_FFFFFF,	 3,		ysize-24,	59,		ysize-24); //设置左边的矩形框的上边
    boxfill8(vram, xsize, COL8_FFFFFF,	 2,		ysize-24,	 2,		ysize- 4); //设置左边的矩形框的左边
    boxfill8(vram, xsize, COL8_848484,	 3,		ysize- 4,	59,		ysize- 4); //设置左边的矩形框的下边
    boxfill8(vram, xsize, COL8_848484,	59,		ysize-23,	59,		ysize- 5); //设置左边的矩形框的右边
    boxfill8(vram, xsize, COL8_000000,	 2,		ysize- 3,	59,		ysize- 3); //设置左边的矩形框的下边阴影
    boxfill8(vram, xsize, COL8_000000,	60,		ysize-24,	60,		ysize- 3); //设置左边的矩形框的右边阴影
 
    boxfill8(vram, xsize, COL8_848484,   xsize-47,		ysize-24,	xsize- 4,	ysize-24); //设置右边矩形框的上边
    boxfill8(vram, xsize, COL8_848484,	 xsize-47,		ysize-23,	xsize-47,	ysize- 4); //设置右边矩形框的左边
    boxfill8(vram, xsize, COL8_FFFFFF,	 xsize-47,		ysize- 3,	xsize- 4,	ysize- 3); //设置右边矩形框的下边
    boxfill8(vram, xsize, COL8_FFFFFF,	 xsize-3,		ysize-24,	xsize- 3,	ysize- 3); //设置右边矩形框的右边

	for(;;)
	{
	     io_hlt();
	    //自建的hlt函数，源目标程序在naskfunc.nas中，c语言本身没有hlt这个命
	}

}

void init_palette(void)
{
	static unsigned char table_rgb[16*3] = 
	{
		0x00, 0x00, 0x00, //0:黑色
		0xff, 0x00, 0x00, //1:亮红
		0x00, 0xff, 0x00, //2:亮绿
		0xff, 0xff, 0x00, //3:亮黄
		0x00, 0x00, 0xff, //4:亮蓝
		0xff, 0x00, 0xff, //5:亮紫
		0x00, 0xff, 0xff, //6:浅亮蓝
		0xff, 0xff, 0xff, //7:白色
		0xc6, 0xc6, 0xc6, //8:亮灰
		0x84, 0x00, 0x00, //9:暗红
		0x00, 0x84, 0x00, //10:暗绿
		0x84, 0x84, 0x00, //11:暗黄
		0x00, 0x00, 0x84, //12:暗青
		0x84, 0x00, 0x84, //13:暗紫
		0x00, 0x84, 0x84, //14:浅暗蓝
		0x84, 0x84, 0x84  //15:暗灰
	};
    set_palette(0, 15, table_rgb);
    return;
}

void boxfill8(unsigned char *vram, int xsize, unsigned char c, int x0, int y0, int x1, int y1)
{
	int x,y;
	for(y = y0; y <= y1; y++)
	{
		for(x = x0; x <= x1; x++)
			vram[y*xsize+x]=c;  //按着屏幕从左往右一行行读，则需要填入的vram = 0xa0000+x+y*320 。因为本身vram大小是320*200
	}
	return;
}

// void set_palette(int start, int end, unsigned char *rgb)
// {
//     int i, eflags ;
// 	eflags = io_load_eflags(); //记录中断许可的标志值
// 	io_cli();                  //将中断许可标志值设为0，禁止中断
// 	io_out8(0x03c8, start);
// 	for ( i = start ; i <= end ; i++)
// 	{
// 		io_out8(0x03c9,rgb[0] / 4);
// 		io_out8(0x03c9,rgb[1] / 4);
// 		io_out8(0x03c9,rgb[2] / 4);
// 		rgb = rgb+3;
// 	}
// 	io_store_eflags(eflags);   //恢复中断许可标志
// }
