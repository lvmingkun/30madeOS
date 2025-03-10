#include "bootpack.h"

void init_gdtidt(void)
{
	struct SEGMENT_DESCRIPTOR *gdt = (struct SEGMENT_DESCRIPTOR *) 0x00270000; // 使用 0x00270000 ~ 0x0027ffff
	struct GATE_DESCRIPTOR *idt = (struct GATE_DESCRIPTOR *) 0x0026f800; // 使用 0x0026f800 ~ 0x0026ffff
	int i;

	// GDT初始化
	for (i = 0; i < 8192; i++)
	{
		set_segmdesc(gdt + i , 0, 0, 0); // 这里 gdt 由于表示的是 8 字节的结构体，因此i每增加一次，地址就 + 8
	}
	set_segmdesc(gdt + 1, 0xffffffff, 0x00000000, 0x4092); // 设定 gdt 的第一个段为全局，方便 cpu 掌管内存
	set_segmdesc(gdt + 2, 0x0007ffff, 0x00280000, 0x409a); // 设定 2 号段大小为 512 kb，用于 bootpack.hrb
	load_gdtr(0xffff, 0x00270000); // 利用汇编给GDTR寄存器存储数值

	/* IDT初始化 */
	for (i = 0; i < 256; i++) {
		set_gatedesc(idt + i, 0, 0, 0);
	}
	load_idtr(LIMIT_IDT, ADR_IDT);

	set_gatedesc(idt + 0x20, (int) asm_inthandler20, 2 * 8, AR_INTGATE32);
	set_gatedesc(idt + 0x21, (int) asm_inthandler21, 2 * 8, AR_INTGATE32);
	set_gatedesc(idt + 0x27, (int) asm_inthandler27, 2 * 8, AR_INTGATE32);
	set_gatedesc(idt + 0x2c, (int) asm_inthandler2c, 2 * 8, AR_INTGATE32);
	set_gatedesc(idt + 0x40, (int) asm_hrb_api	   , 2 * 8, AR_INTGATE32);

    return;
}

void set_segmdesc(struct SEGMENT_DESCRIPTOR *sd, unsigned int limit, int base, int ar)
{
	if (limit > 0xfffff) {
		ar |= 0x8000; /* G_bit = 1 */
		limit /= 0x1000;
	}
	sd->limit_low = limit & 0xffff;
	sd->base_low  = base & 0xffff;
	sd->base_mid = (base >> 16) & 0xff;
	sd->access_right = ar & 0xff;
	sd->limit_high = ((limit >> 16) & 0x0f) | ((ar >> 8) & 0xf0);
	sd->base_high = (base >> 24) & 0xff;
	return;
}

void set_gatedesc(struct GATE_DESCRIPTOR *gd, int offset, int selector, int ar)
{
	gd->offset_low = offset & 0xffff;
	gd->selector = selector;
	gd->dw_count = (ar >> 8) & 0xff;
	gd->access_right = ar & 0xff;
	gd->offset_high = (offset >> 16) & 0xffff;
	return;
}
