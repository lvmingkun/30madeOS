#include "bootpack.h"

#define EFLAGS_AC_BIT		0x00040000
#define CR0_CACHE_DISABLE	0x60000000

/* 如果是 486 cpu 则elags寄存器的最高位是AC标志；如果是 386 则无这个标志，那么将一直是 0;
   对cr0寄存器的第 31、30位设置为 1，因此就用 0x600000000 进行与，此时可以禁止缓存
*/

unsigned int memtest(unsigned int start, unsigned int end)
{
	char flg486 = 0 ;
	unsigned int eflg, cr0, i;
	// 确认cpu是 386 还是 486，486 ~ 100 mhz
	eflg = io_load_eflags();
	eflg |= EFLAGS_AC_BIT; /* AC-bit = 1 */
	io_store_eflags(eflg);
	eflg = io_load_eflags();
	if((eflg & EFLAGS_AC_BIT) != 0 )// 386 的话即使设定AC等于 1，其值还是会自动回到 0
	{
		flg486 = 1;
	} 
	eflg &= ~EFLAGS_AC_BIT; /* AC-bit = 0, ~EFLAGS_AC_BIT = 0xfffbffff */
	io_store_eflags(eflg);

	if( flg486 != 0)
	{
		cr0 = load_cr0();
		cr0 |= CR0_CACHE_DISABLE; // 禁止缓存
		store_cr0(cr0);
	}

	i = memtest_sub(start, end);

	if( flg486 != 0)
	{
		cr0 = load_cr0();
		cr0 &= ~CR0_CACHE_DISABLE; // 允许缓存
		store_cr0(cr0);
	}

	return i;
}

void memman_init(struct MEMMAN *man)
{
	man->frees = 0; // 可用信息数目
	man->maxfrees =0; // 用于观察可用状况：frees的最大值
	man->lostsize = 0; // 释放失败的内存的大小总和
	man->losts = 0; // 释放失败的次数
	return;
}

unsigned int memman_total(struct MEMMAN *man)
{	// 报告空余内存大小的合计
	unsigned int i, t = 0;
	for(i = 0; i < man->frees; i++)
	{
		 t += man->free[i].size;
	}
	return t;
}

unsigned int memman_alloc(struct MEMMAN *man, unsigned int size)
{	// 分配
	unsigned int i, a;
	for(i = 0; i < man->frees; i++)
	{
		if(man->free[i].size >= size) // 如果可以找到足够大的内存
		{
			a = man->free[i].addr;
			man->free[i].addr += size;
			man->free[i].size -= size;
			if(man->free[i].size == 0)
			{	// 如果free[i] 变成了 0 ，就减掉一个可用信息
				man->frees--;
				for(; i < man->frees; i++)
				{
					man->free[i] = man->free[i+1]; // 带入结构体
				}
			}
			return a;
		}
	}
	return 0; // 没有可用空间
}

int memman_free(struct MEMMAN *man, unsigned int addr, unsigned int size)
{
	// 释放
  	int i, j; 
  	/* 为了便于归纳内存，将free[]按照addr的顺序排列，递增
    所以，下面先决定应该放在哪里
  	*/ 	
  	for(i = 0; i < man->frees; i++)
  	{ 
  		if(man->free[i].addr > addr)
  		{
  			break;
  		}
  	}
 	// free[i-1].addr < addr < free[i].addr
  	if(i > 0)
  	{
  		// 前面有可用内存
  		if(man->free[i-1].addr + man->free[i-1].size == addr)
  		{	// 与前面可用内存放在一起
  			man->free[i-1].size += size;
  			if(i < man->frees)
  			{	// 后面也有
  				if(addr + size == man->free[i].addr)
  				{	// 与后面可用内存放在一起
  					man->free[i-1].size += man->free[i].size;
  					// 删去man->free[i]
  					// free[i]变成 0 后归纳到前面
  					man->frees--;
  					for(; i < man->frees; i++)
				  	{
						man->free[i] = man->free[i+1]; // 带入结构体
				  	}
  				}
  			}
  			return 0; // 成功完成
  		}
  	}
  	// 不能与前面的可用空间合在一起
  	if(i < man->frees)
  	{ // 后面还有
  		if(addr + size == man->free[i].addr)
  		{	// 可以与后面内容归纳在一起
  			man->free[i].addr = addr;
  			man->free[i].size += size;
  			return 0; // 完成
  		}
  	}
  	// 既不能与前面归纳，也不能与后面归纳
  	if(man->frees < MEMMAN_FREES)
  	{	// free[i]之后的，向后移动，腾出空间
  		for(j = man->frees; j > i; j--) // 如果还有可用数量的话，则往后移动
  		{
  			man->free[j] = man->free[j-1];
  		}
  		man->frees++;
  		if(man->maxfrees < man->frees)
  		{
  			man->maxfrees = man->frees; // 更新最大值
  		}
  		man->free[i].addr = addr;
  		man->free[i].size = size;
  		return 0; // 成功完成
  	}
  	// 不能后移
  	man->losts++;
  	man->lostsize += size;
  	return -1; // 失败
}

unsigned int memman_alloc_4k(struct MEMMAN *man, unsigned int size)
{
	unsigned int a;
	size = (size + 0xfff) & 0xfffff000; // 向上取整，确保为是以 4k 为单位的，不够 4k 的按 4k 算
	a = memman_alloc(man, size);
	return a; 
}

int memman_free_4k(struct MEMMAN *man, unsigned int addr, unsigned int size)
{
	int i;
	size = (size + 0xfff) & 0xfffff000;
	i = memman_free(man, addr, size);
	return i;
}

