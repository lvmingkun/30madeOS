; haribote-ipl
; TAB=4

CYLS	EQU		30				; ����CYLS = 30

		ORG		0x7c00			; ָ������װ�ص�ַ

; ��׼FAT12��ʽ����ר�õĴ��� Stand FAT12 format floppy code

		JMP		entry
		DB		0x90
		DB		"HARIBOTE"		; �����������ƣ�8 �ֽڣ�
		DW		512				; ÿ��������sector����С������ 512 �ֽڣ�
		DB		1				; �أ�cluster����С������Ϊ 1 ��������
		DW		1				; FAT��ʼλ�ã�һ��Ϊ��һ��������
		DB		2				; FAT����������Ϊ 2��
		DW		224				; ��Ŀ¼��С��һ��Ϊ 224 �
		DW		2880			; �ô��̴�С������Ϊ 2880 ����1440 * 1024 / 512��
		DB		0xf0			; �������ͣ�����Ϊ0xf0��
		DW		9				; FAT�ĳ��ȣ��� 9 ������
		DW		18				; һ���ŵ���track���м�������������Ϊ 18��
		DW		2				; ��ͷ������ 2��
		DD		0				; ��ʹ�÷����������� 0
		DD		2880			; ��дһ�δ��̴�С
		DB		0,0,0x29		; ���岻�����̶���
		DD		0xffffffff		; �������ǣ�������
		DB		"HARIBOTEOS "	; ���̵����ƣ�����Ϊ 11 �֣�������ո�
		DB		"FAT12   "		; ���̸�ʽ���ƣ��� 8 �֣�������ո�
		RESB	18				; �ȿճ� 18 �ֽ�

; ��������

entry:
		MOV		AX, 0			; ��ʼ���Ĵ���
		MOV		SS, AX
		MOV		SP, 0x7c00
		MOV		DS, AX

; ��ȡ����

		MOV		AX, 0x0820
		MOV		ES, AX
		MOV		CH, 0			; ���� 0
		MOV		DH, 0			; ��ͷ 0
		MOV		CL, 2			; ���� 2
		MOV		BX,18 * 2 * CYLS - 1	; Ҫ��ȡ�ĺϼ�������
		CALL	readfast		; ���߶�ȡ

; ��ȡ��ϣ���ת��haribote.sysִ�У�
		MOV		[0x0ff0], CH		; ��¼IPLʵ�ʶ�ȡ�˶�������
		JMP		0xc200

error:
		MOV		AX, 0
		MOV		ES, AX
		MOV		SI, msg
putloop:
		MOV		AL, [SI] 
		ADD		SI, 1			; ��SI�� 1
		CMP		AL, 0
		JE		fin
		MOV		AH, 0x0e			; ��ʾһ������
		MOV		BX, 15			; ָ���ַ���ɫ
		INT		0x10			; �����Կ�BIOS
		JMP		putloop
fin:
		HLT						; ��CPUֹͣ���ȴ�ָ��
		JMP		fin				; ����ѭ��
msg:
		DB		0x0a, 0x0a		; ��������
		DB		"load error"
		DB		0x0a			; ����
		DB		0

readfast:	; ʹ��AL����һ���Զ�ȡ���� �Ӵ˿�ʼ
; ES:��ȡ��ַ, CH:����, DH:��ͷ, CL:����, BX:��ȡ������

		MOV		AX,ES			; < ͨ��ES����AL�����ֵ >
		SHL		AX, 3			; ��AX���� 32�����������AH��SHL������λָ�
		AND		AH, 0x7f		; AH��AH���� 128 ���õ�������512 * 128 = 64 K��
		MOV		AL, 128		; AL = 128 - AH; AH��AH���� 128 ���õ�������512 * 128 = 64 K��
		SUB		AL,AH

		MOV		AH, BL			; < ͨ��BX����AL�����ֵ������AH >
		CMP		BH, 0			; if (BH != 0) { AH = 18; }
		JE		.skip1
		MOV		AH, 18
.skip1:
		CMP		AL, AH			; if (AL > AH) { AL = AH; }
		JBE		.skip2
		MOV		AL, AH
.skip2:

		MOV		AH, 19			; < ͨ��CL����AL�����ֵ������AH >
		SUB		AH, CL			; AH = 19 - CL;
		CMP		AL, AH			; if (AL > AH) { AL = AH; }
		JBE		.skip3
		MOV		AL, AH
.skip3:

		PUSH	BX
		MOV		SI, 0			; ����ʧ�ܴ����ļĴ���
retry:
		MOV		AH, 0x02			; AH = 0x02 : ��ȡ����
		MOV		BX, 0
		MOV		DL, 0x00			; A��
		PUSH	ES
		PUSH	DX
		PUSH	CX
		PUSH	AX
		INT		0x13			; ���ô���BIOS
		JNC		next			; û�г���Ļ�����ת��next
		ADD		SI, 1			; ��SI�� 1
		CMP		SI, 5			; ��SI�� 5 �Ƚ�
		JAE		error			; SI >= 5����ת��error
		MOV		AH, 0x00
		MOV		DL, 0x00		; A��
		INT		0x13			; ����������
		POP		AX
		POP		CX
		POP		DX
		POP		ES
		JMP		retry
next:
		POP		AX
		POP		CX
		POP		DX
		POP		BX				; ��ES�����ݴ���BX
		SHR		BX, 5			; ��BX�� 16 �ֽ�Ϊ��λת��Ϊ 512 �ֽ�Ϊ��λ
		MOV		AH, 0
		ADD		BX, AX			; BX += AL;
		SHL		BX, 5			; ��BX��512�ֽ�Ϊ��λת��Ϊ 16 �ֽ�Ϊ��λ
		MOV		ES, BX			; �൱��EX += AL * 0x20;
		POP		BX
		SUB		BX, AX
		JZ		.ret
		ADD		CL, AL			; ��CL����AL
		CMP		CL, 18			; ��CL�� 18 �Ƚ�
		JBE		readfast	; CL <= 18����ת��readfast
		MOV		CL, 1
		ADD		DH, 1
		CMP		DH, 2
		JB		readfast	; DH < 2 ����ת��readfast
		MOV		DH, 0
		ADD		CH, 1
		JMP		readfast
.ret:
		RET

		RESB	0x7dfe-$	; ��0x7dfeΪֹ��0x00����ָ��

		DB		0x55, 0xaa

