[FORMAT "WCOFF"]				; 生成对象文件的模式	
[INSTRSET "i486p"]				; 使用 486 兼容模式
[BITS 32]						; 使用 32 位模式机器语言
[FILE "api004.nas"]				; 源文件信息名

    GLOBAL _api_end

[SECTION .text]
_api_end:    	; void api_end(void);
	MOV EDX, 4
    INT 0x40