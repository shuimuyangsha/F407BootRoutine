						 
globalptr	RN r10 ;//=wram_globals* ptr
;//cpu_zpage	RN r11 ;=CPU_RAM
;----------------------------------------------------------------------------


 MAP 0,globalptr		  ;//MAP ���ڶ���һ���ṹ�����ڴ����׵�ַ
				;//6502.s	  ;//�����ڴ����׵�ַΪglobalptr
opz #   4              ;opz # 256*4       ;//������ַ					  
readmem_tbl # 8*4			  ;//8*4
writemem_tbl # 8*4			  ;//8*4
memmap_tbl # 8*4			 ;//�洢��ӳ�� ram+rom
cpuregs # 7*4				 ;//1208���6502�Ĵ�������Ŀ�ʼ��ַ
m6502_s # 4					 ;//
lastbank # 4				;//6502PC�� ROM�����ƫ����
nexttimeout # 4

rombase # 4			;//ROM��ʼ��ַ
romnumber # 4		 ;// ROM��С  
rommask # 4		   ;//ROM��Ĥ	rommask=romsize-1

joy0data # 4	   ;//��������
joy1data # 4	   ;//�ֱ�1��������

clocksh # 4    ;//ִ�е�ʱ���� apu��
cpunmif # 4      ;cpu�жϱ�־
cpuirqf # 4      ;cpu�жϱ�־ 
;------------------------------------------------------------------------------------------


;// # 2 ;align					 

		END
