/*ͷ�ļ�������*/
#include <STC15F2K60S2.H>
#include <Init.h>
#include "led.h"
#include "key.h"
#include <Seg.h>
#include <onewire.h>
#include <ds1302.h>

/* ���������� */
unsigned char Key_Val,Key_Down,Key_Old,Key_Up,Key_Interrupt_Val;//����ר�ñ���
unsigned char Key_Slow_Down;//��������ר�ñ���
unsigned char Seg_Buf[8] = {10,10,10,10,10,10,10,10};//�������ʾ���ݴ������
unsigned char Seg_Pos;//�����ɨ��ר�ñ���
idata unsigned char Seg_Disp_Mode = 0;   //ģʽ0�¶���ʾ  ģʽ1ʱ����ʾ  ģʽ2��������
idata unsigned int Seg_Slow_Down;//����ܼ���ר�ñ���
idata unsigned char ucLed[8] = {0,0,0,0,0,0,0,0};//Led��ʾ���ݴ������
idata float temperature; //ʵʱ�¶ȱ���
idata unsigned char  temperature_patram = 23;     //10-99  
idata unsigned int Temperature_Slow_Down;//����ܼ���ר�ñ���
pdata unsigned char ucRtc[3] = {12,59,55};
idata bit Rtc_Disp_Mode = 0;  //0-Сʱ���� 1-��������
idata bit Mode_Control = 0; // 0-�¶ȿ��� 1-ʱ�ӿ���
idata bit Relay_0_Flag;     //�¶ȿ��Ʊ�־λ
idata bit Relay_1_Flag;				//ʱ����Ʊ�־λ
idata int Rtc_5000ms;
idata int flash_100ms;
idata bit Led_Control = 0;

/* ���̴������� */
void Key_Proc()
{
    if(Key_Slow_Down<10) return;
    Key_Slow_Down = 0;//���̼��ٳ���

    Key_Val = Key_Read();//ʵʱ��ȡ����ֵ
    Key_Down = Key_Val & (Key_Old ^ Key_Val);//��׽�����½���
    Key_Up = ~Key_Val & (Key_Old ^ Key_Val);//��׽�����Ͻ���
    Key_Old = Key_Val;//����ɨ�����
	
	switch(Key_Down)
	{
		case 12:
			if(++Seg_Disp_Mode == 3){Seg_Disp_Mode = 0;}
			break;
		case 13:
			Mode_Control ^= 1;   
			break;
  //s16�Ӳ���
		case 16:
			if(Seg_Disp_Mode == 2 && (++temperature_patram == 100)){temperature_patram = 99;}break;
	//s17������
		case 17:
			if(Seg_Disp_Mode == 2 && (--temperature_patram == 9)){temperature_patram = 10;}break;
	}
	//���������ж�Rtcģʽ
	if(Key_Val == 17)
	{Rtc_Disp_Mode = 1;}
	else 
	{Rtc_Disp_Mode = 0;}
	
	}	

	
/* ��Ϣ�������� */
void Seg_Proc()
{
    if(Seg_Slow_Down<100) return;
    Seg_Slow_Down = 0;//����ܼ��ٳ���


	/*��Ϣ��ʾ����*/
	Seg_Buf[0] = 12;
	Seg_Buf[1] = Seg_Disp_Mode + 1;
	
	switch (Seg_Disp_Mode)
	{
		case 0 : 	
			Seg_Buf[5] = (int)temperature /10;
			Seg_Buf[6] = (int)temperature % 10 + ',';
			Seg_Buf[7] = (int)(temperature * 10 ) % 10;
		break;
		
		case 1 :
			Seg_Buf[5] = 13;
			if(Rtc_Disp_Mode)
			{
				Seg_Buf[3] = ucRtc[1] / 10 % 10;
				Seg_Buf[4] = ucRtc[1] % 10;
					
				Seg_Buf[6] = ucRtc[2] / 10 % 10;
				Seg_Buf[7] = ucRtc[2] % 10;
			}
			else 
			{
				Seg_Buf[3] = ucRtc[0] / 10 %10;
				Seg_Buf[4] = ucRtc[0] % 10;
					
				Seg_Buf[6] = ucRtc[1] / 10 % 10;
				Seg_Buf[7] = ucRtc[1] % 10;
			}
				
		break;
			
		case 2 :
			Seg_Buf[3] = 10;
			Seg_Buf[4] = 10;	
		
			Seg_Buf[5] = 10;
			Seg_Buf[6] = temperature_patram / 10;
			Seg_Buf[7] = temperature_patram % 10;
			break;
		
	}
	
	

}
/* ������ʾ���� */
void Led_Proc()
{
	//�¶ȿ���ģʽ�̵������¶ȴ��ڲ���ʱ�̵���ҧ��
	if(Mode_Control == 0)
	{Relay_0_Flag = temperature > temperature_patram;
		Relay(Relay_0_Flag);
	}
		//ʱ�����ģʽʱ��Ϊ����ʱ�̵���ҧ��
	else 
	{	if( ucRtc[1] == 0 && ucRtc[2] == 0)
				{Relay_1_Flag = 1;}
				Relay(Relay_1_Flag);
	}
	Led_Disp(ucLed);
	if(ucRtc[1] == 0 && ucRtc[2]==0)
	{ucLed[0] = 1 ;}
	
	ucLed[1] = !Mode_Control;

}
/*�¶Ȼ�ȡ����*/
void Get_temperatrue()
{
	if(Temperature_Slow_Down < 500) return;
	else Temperature_Slow_Down = 0;
	
	temperature = rd_temperature();

}

/*ʱ���ȡ����*/
void Get_time()
{
	Read_Rtc(ucRtc);

}



void Timer0Init(void)		//1����@12.000MHz
{
    AUXR &= 0x7F;		//��ʱ��ʱ��12Tģʽ
    TMOD &= 0xF0;		//���ö�ʱ��ģʽ
    TL0 = 0x18;		//���ö�ʱ��ʼֵ
    TH0 = 0xFC;		//���ö�ʱ��ʼֵ
    TF0 = 0;		//���TF0��־
    TR0 = 1;		//��ʱ��0��ʼ��ʱ
    ET0 = 1;    //��ʱ���ж�0��
    EA = 1;     //���жϴ�
}

/* ��ʱ��0�жϷ����� */
void Timer0Server() interrupt 1
{  
    Key_Slow_Down ++;
    Seg_Slow_Down ++;
	  Temperature_Slow_Down ++;
	
	if(Relay_1_Flag == 1 && (++Rtc_5000ms == 5000))
	{
		Relay_1_Flag = 0;
		Rtc_5000ms = 0;
		ucLed[0] = 0 ;
	}
	if( Relay_0_Flag == 1||Relay_1_Flag == 1)
	{
		if(++flash_100ms == 100 )
		{
			flash_100ms = 1;
			Led_Control ^= 1 ;
		}	
	}
	else 
	{Led_Control = 0;}
		ucLed[2] = Led_Control;
    if(++Seg_Pos == 8) Seg_Pos = 0;//�������ʾר��
     // �������ʾ����
    if (Seg_Buf[Seg_Pos] > 20)
        Seg_Disp(Seg_Pos, Seg_Buf[Seg_Pos] - ',', 1); // ��С����
    else
        Seg_Disp(Seg_Pos, Seg_Buf[Seg_Pos], 0); // ��С����
    
   
}


void main()
{
	Set_Rtc(ucRtc);
	System_Init();
	Timer0Init();
	 rd_temperature();
	while(1)
	{
		Key_Proc();
		Seg_Proc();
		Led_Proc();
		Get_temperatrue();
		Get_time();
	}

}