#include "bootpack.h"

#define SHEET_USE		1

//初始化图层管理表的地址
struct SHTCTL *shtctl_init(struct MEMMAN *memman, unsigned char *vram, int xsize, int ysize)
{
	struct SHTCTL *ctl;
	int i;
	//返回一个空闲的addr地址,用sizeof计算该变量的地址空间大小，然后进行分配对应的内存空间大小
	ctl = (struct SHTCTL *) memman_alloc_4k(memman, sizeof (struct SHTCTL));
	if(ctl == 0)
	{
		goto err;
	}
	ctl->map = (unsigned char *) memman_alloc_4k(memman, xsize * ysize);
	if(ctl->map == 0) {
		memman_free_4k(memman, (int) ctl, sizeof(struct SHTCTL));
		goto err;
	}
	ctl->vram  = vram; //导入vram的地址和分辨率
	ctl->xsize = xsize;
	ctl->ysize = ysize;
	ctl->top = -1;//此时，一个SHEET都没有，也就是说没有图层在最上面
	for(i = 0; i <  MAX_SHEETS; i++)
	{
		ctl->sheets0[i].flags = 0;//让所有的图层都标记未使用
		ctl->sheets0[i].ctl = ctl; //记录所属
	}
err:
    return ctl;
}

//取得新生成的未使用的图层
struct SHEET *sheet_alloc(struct SHTCTL *ctl)
{
	struct SHEET *sht; //建立一个新的图层，以便进行后续的返回
	int i;
	for(i = 0; i < MAX_SHEETS; i++) //开始循环，直到找到可用的图层为止
	{
		if(ctl->sheets0[i].flags == 0)
		{
			sht = &ctl->sheets0[i];//将该可用图层的地址传入给sht
			sht->flags = SHEET_USE; //标记为正在使用
			sht->height = -1; //隐藏，先将高度设为-1，置于底层
			return sht;
		}
	}
	return 0; //所有的sheet都在使用当中
}

//设定图层的缓冲区大小和透明色的函数
void sheet_setbuf(struct SHEET *sht, unsigned char *buf, int xsize, int ysize, int col_inv)
{//将图层和图层在画面中的大小以及
	sht->buf = buf;//记录该图层所描绘图形的地址
	sht->bxsize = xsize;//图层整体大小
	sht->bysize = ysize;
	sht->col_inv = col_inv;//给予透明色号
	return;
}

//指定了vx0~vy1 的刷新范围
void sheet_refreshsub(struct SHTCTL *ctl, int vx0, int vy0, int vx1, int vy1, int h0, int h1)
{
	int h, bx, by, vx, vy, bx0, by0, bx1, by1;
	unsigned char *buf, c, *vram = ctl->vram, *map = ctl->map, sid;
	struct SHEET *sht;//创建一个临时图层
	if (vx0 < 0) { vx0 = 0;}
	if (vy0 < 0) { vy0 = 0;}
	if (vx1 > ctl->xsize) { vx1 = ctl->xsize;}
	if (vy1 > ctl->ysize) { vy1 = ctl->ysize;}
	for( h = h0; h <= h1; h++)//从下往上,所有图层都描绘一遍
	{
		sht = ctl->sheets[h];//记录该层的图层信息
		buf = sht->buf;//记录图层上描绘图画的地址
		//使用vx0~vy1,对比bx0~by1进行倒推
		sid = sht - ctl->sheets0;
		bx0 = vx0 - sht->vx0;
		by0 = vy0 - sht->vy0;
		bx1 = vx1 - sht->vx0;
		by1 = vy1 - sht->vy0;
		if(bx0 < 0) {bx0 = 0;} //这里的bx0、1都是相对距离，即相对于sht->vx0，也就是刷新的部分=图层在画面上的的起始地址+相对于起始地址的距离
		if(by0 < 0) {by0 = 0;}
		if(bx1 > sht->bxsize) {bx1 = sht->bxsize;}
		if(by1 > sht->bysize) {by1 = sht->bysize;}
		for(by = by0; by < by1; by++)
		{
			vy = sht->vy0 + by;
			for (bx = bx0; bx < bx1; bx++)
			{
				vx = sht->vx0 + bx;//vy0 vx0代表图层在VRAM上画面的相对地址，用这个来让图层进行移动
				if (map[vy * ctl->xsize + vx] == sid) //按照map中的内容判断是否写入刷新
				{
					vram[vy * ctl->xsize + vx] = buf[by * sht->bxsize + bx];//写入vram中
				}
			}	

		}
	}
	return;
	
}

//设定底板的高度
void sheet_updown(struct SHEET *sht, int height)
{
	struct SHTCTL *ctl = sht->ctl;
	int h, old = sht->height; //存储之前的高度信息

	//如果指定的高度过高或者过低，则进行修正
	if(height > ctl->top+1)
	{
		height = ctl->top+1;
	}
	if(height < -1)
	{
		height = -1;
	}
	sht->height = height; //设定高度

	//进行sheets[]的重新排列，也就是按高度顺序写入sheets中
	if(old > height)
	{//比以前低
		if(height >= 0) //把中间的往上提
		{
			for(h = old; h > height; h--)
			{
				ctl->sheets[h] = ctl->sheets[h-1];
				ctl->sheets[h]->height = h;
			}
			ctl->sheets[height] = sht;
			sheet_refreshmap(ctl, sht->vx0, sht->vy0, sht->vx0 + sht->bxsize, sht->vy0 + sht->bysize, height + 1); 
			sheet_refreshsub(ctl, sht->vx0, sht->vy0, sht->vx0 + sht->bxsize, sht->vy0 + sht->bysize, height + 1, old); 
		}else{// 隐藏
			if(ctl->top > old)//把上面的降下来
			{
				for(h = old; h < ctl->top; h++)
				{
					ctl->sheets[h] = ctl->sheets[h+1];
					ctl->sheets[h]->height = h;
				}
			}
			ctl->top--; //由于中间图层减少一个，所以最上面的图层高度下降
		}
		sheet_refreshmap(ctl, sht->vx0, sht->vy0, sht->vx0 + sht->bxsize, sht->vy0 + sht->bysize, 0); 
		sheet_refreshsub(ctl, sht->vx0, sht->vy0, sht->vx0 + sht->bxsize, sht->vy0 + sht->bysize, 0, old - 1); //按新的图层信息重新绘制画面
	}else if(old < height)//比以前高
	{
		if(old > 0)
		{
		//把中间的拉下去
		for(h = old; h < height; h++)
		{
			ctl->sheets[h] = ctl->sheets[h+1];
			ctl->sheets[h]->height = h;
		}
		ctl->sheets[height] = sht;
		}else{//由隐藏状态转化为显示状态
			//将已在上面的提上来
			for(h = ctl->top; h >= height ; h--)
			{
				ctl->sheets[h+1] = ctl->sheets[h];
				ctl->sheets[h+1]->height =h+1;
			}
			ctl->sheets[height] = sht;
			ctl->top++;//由于已显示的图层增加一个，所以最上面的图层高度增加
		}
		sheet_refreshmap(ctl, sht->vx0, sht->vy0, sht->vx0 + sht->bxsize, sht->vy0 + sht->bysize, height); 
		sheet_refreshsub(ctl, sht->vx0, sht->vy0, sht->vx0 + sht->bxsize, sht->vy0 + sht->bysize, height, height);//按新图层重新绘制画面
	}
	return;
}
//从下往上，将透明以外的所有像素都复制到vram中
void sheet_refresh(struct SHEET *sht, int bx0, int by0, int bx1, int by1)
{
	if (sht->height >= 0) { /* 如果正在显示，则按新图层进行刷新 */
		sheet_refreshsub(sht->ctl, sht->vx0 + bx0, sht->vy0 + by0, sht->vx0 + bx1, sht->vy0 + by1, sht->height, sht->height);
	}
	return;
}

//进行上下左右移动
void sheet_slide(struct SHEET *sht, int vx0, int vy0)
{
	struct SHTCTL *ctl = sht->ctl;
	int old_vx0 = sht->vx0, old_vy0 = sht->vy0;
	sht->vx0 = vx0;
	sht->vy0 = vy0;
	if(sht->height >= 0)//如果正在显示//按图层信息刷新画面
	{
		/*由于鼠标图层的地址已经改变了，因此以下两条语句:（因为sub这个是只刷新指定范围内的）
		如果只执行第一句的话，那么新鼠标无法显示（执行完后，因为鼠标图层改变了，不再原来位置了，因此图层只会刷新原先地方存在的图层的）
		如果只执行第二句的话，那么可能会显示不完整。
		这里，由于画面移动了，因此在其之下的之前的图层会露出来，因此从最下面开始刷新；
		然后在移动的目标处，比移动来的图层要低的图层没有说明变化，只是隐藏起来了因此直接刷移来的图层及其之上的即可
		*/
		sheet_refreshmap(ctl, old_vx0, old_vy0, old_vx0 + sht->bxsize, old_vy0 + sht->bysize, 0);
		sheet_refreshmap(ctl, vx0, vy0, vx0 + sht->bxsize, vy0 + sht->bysize, sht->height);
		sheet_refreshsub(ctl, old_vx0, old_vy0, old_vx0 + sht->bxsize, old_vy0 + sht->bysize, 0, sht->height - 1);
		sheet_refreshsub(ctl, vx0, vy0, vx0 + sht->bxsize, vy0 + sht->bysize, sht->height, sht->height);
	}
	return;
}
//释放已使用的图层
void sheet_free(struct SHEET *sht)
{
	if(sht->height >= 0)
	{
		sheet_updown(sht, -1);//处于显示状态，先设定隐藏
	}
	sht->flags = 0; //为使用标志
	return;
}

void sheet_refreshmap(struct SHTCTL *ctl, int vx0, int vy0, int vx1, int vy1, int h0)
{
	int h, bx, by, vx, vy, bx0, by0, bx1, by1;
	unsigned char *buf, sid, *map = ctl->map;
	struct SHEET *sht;
	if (vx0 < 0) { vx0 = 0; }
	if (vy0 < 0) { vy0 = 0; }
	if (vx1 > ctl->xsize) { vx1 = ctl->xsize; }
	if (vy1 > ctl->ysize) { vy1 = ctl->ysize; }
	for (h = h0; h <= ctl->top; h++) {
		sht = ctl->sheets[h];
		sid = sht - ctl->sheets0; //将进行了减法计算的地址作为图层号码
		buf = sht->buf;
		bx0 = vx0 - sht->vx0;
		by0 = vy0 - sht->vy0;
		bx1 = vx1 - sht->vx0;
		by1 = vy1 - sht->vy0;
		if (bx0 < 0) { bx0 = 0; }
		if (by0 < 0) { by0 = 0; }
		if (bx1 > sht->bxsize) { bx1 = sht->bxsize; }
		if (by1 > sht->bysize) { by1 = sht->bysize; }
		for (by = by0; by < by1; by++) {
			vy = sht->vy0 + by;
			for (bx = bx0; bx < bx1; bx++) {
				vx = sht->vx0 + bx;
				if (buf[by * sht->bxsize + bx] != sht->col_inv) {
					map[vy * ctl->xsize + vx] = sid;
				}
			}
		}
	}
	return;
}
