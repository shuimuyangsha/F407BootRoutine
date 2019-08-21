#include "spblcd.h"
#include "spb.h"
#include "dma.h"
#include "lcd.h"
#include "delay.h"
#include "malloc.h"
#include "usart.h"
//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEK STM32F407������
//SPBLCD ��������	   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//��������:2014/7/20
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2014-2024
//All rights reserved									  
////////////////////////////////////////////////////////////////////////////////// 	


u16 *sramlcdbuf;							//SRAM LCD BUFFER,����ͼƬ�Դ��� 

//��ָ��λ�û���.
//x,y:����
//color:��ɫ.
void slcd_draw_point(u16 x,u16 y,u16 color)
{	 
	sramlcdbuf[y+x*spbdev.spbheight+spbdev.frame*spbdev.spbheight*spbdev.spbwidth]=color;
}
//��ȡָ��λ�õ����ɫֵ
//x,y:����
//����ֵ:��ɫ
u16 slcd_read_point(u16 x,u16 y)
{
	return sramlcdbuf[y+x*spbdev.spbheight+spbdev.frame*spbdev.spbheight*spbdev.spbwidth];
} 
//�����ɫ
//x,y:��ʼ����
//width��height����Ⱥ͸߶�
//*color����ɫ����
void slcd_fill_color(u16 x,u16 y,u16 width,u16 height,u16 *color)
{   
	u16 i,j; 
 	for(i=0;i<height;i++)
	{
		for(j=0;j<width;j++)
		{
			slcd_draw_point(x+j,y+i,*color++);
		}	
	}	
} 
//SRAM --> LCD_RAM dma����
//16λ,�ⲿSRAM���䵽LCD_RAM. 
void slcd_dma_init(void)
{  
	DMA_InitTypeDef  DMA_InitStructure;
	
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2,ENABLE);//DMA2ʱ��ʹ�� 
 
	while (DMA_GetCmdStatus(DMA2_Stream0) != DISABLE){}//�ȴ�DMA2_Stream1������ 	
	
	DMA_DeInit(DMA2_Stream0); 
	  /* ���� DMA Stream */
  DMA_InitStructure.DMA_Channel = DMA_Channel_0;  //ͨ��0
  DMA_InitStructure.DMA_Memory0BaseAddr = (u32)&LCD->LCD_RAM;;//DMA �洢��0��ַ
	DMA_InitStructure.DMA_PeripheralBaseAddr=0;
  DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToMemory;//�洢�����洢��ģʽ
  DMA_InitStructure.DMA_BufferSize = 0;//���ݴ����� 
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Enable;//��������ģʽ
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Disable;//�洢��������ģʽ
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;//�������ݳ���:16λ
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;//�洢�����ݳ��� 16λ
  DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;// ʹ����ͨģʽ 
  DMA_InitStructure.DMA_Priority = DMA_Priority_High;//�����ȼ�
  DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable; //FIFOģʽ������       
  DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;//����ͻ�����δ���
  DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;//�洢��ͻ�����δ���
  DMA_Init(DMA2_Stream0, &DMA_InitStructure);//��ʼ��DMA Stream

} 

//����һ��SPI��LCD��DMA�Ĵ���
//x:��ʼ�����ַ���(0~480)
void slcd_dma_enable(u32 x)
{	  
	u32 lcdsize=spbdev.spbwidth*spbdev.spbheight;
	u32 dmatransfered=0;
	while(lcdsize)
	{ 
		DMA_Cmd(DMA2_Stream0,DISABLE);//�ر�DMA2,Stream0
	  while (DMA_GetCmdStatus(DMA2_Stream0) != DISABLE){}//�ȴ�DMA2_Stream0������ 	
		DMA_ClearFlag(DMA2_Stream0,DMA_FLAG_TCIF0);//�����������ж�
		if(lcdsize>SLCD_DMA_MAX_TRANS)
		{
			lcdsize-=SLCD_DMA_MAX_TRANS;
			DMA_SetCurrDataCounter(DMA2_Stream0,SLCD_DMA_MAX_TRANS);//���ô��䳤��
		}else
		{
			DMA_SetCurrDataCounter(DMA2_Stream0,lcdsize);//���ô��䳤��
			lcdsize=0;
		}	
		DMA2_Stream0->PAR=(u32)(sramlcdbuf+x*spbdev.spbheight+dmatransfered);	
		dmatransfered+=SLCD_DMA_MAX_TRANS;		
		DMA_Cmd(DMA2_Stream0,ENABLE);//����DMA2,Stream0
		while(DMA_GetFlagStatus(DMA2_Stream0,DMA_FLAG_TCIF0)!=SET);//�ȴ�������� 
	} 
	DMA_Cmd(DMA2_Stream0,DISABLE);//�ر�DMA2,Stream0
}
//��ʾһ֡,������һ��spi��lcd����ʾ.
//x:����ƫ����
void slcd_frame_show(u32 x)
{  
	LCD_Scan_Dir(U2D_L2R);		//����ɨ�跽��  
	if(lcddev.id==0X9341||lcddev.id==0X5310||lcddev.id==0X5510||lcddev.id==0X6804)
	{
		LCD_Set_Window(spbdev.stabarheight,0,spbdev.spbheight,spbdev.spbwidth);
		LCD_SetCursor(spbdev.stabarheight,0);//���ù��λ�� 
	}else
	{
		LCD_Set_Window(0,spbdev.stabarheight,spbdev.spbwidth,spbdev.spbheight);
		if(lcddev.id!=0X1963)LCD_SetCursor(0,spbdev.stabarheight);//���ù��λ�� 		
	}
	LCD_WriteRAM_Prepare();     //��ʼд��GRAM	
	slcd_dma_enable(x);
	LCD_Scan_Dir(DFT_SCAN_DIR);	//�ָ�Ĭ�Ϸ���
	LCD_Set_Window(0,0,lcddev.width,lcddev.height);//�ָ�Ĭ�ϴ��ڴ�С
}
 





