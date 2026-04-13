/*头文件声明区*/
#include <STC15F2K60S2.H>
#include <Init.h>
#include "led.h"
#include "key.h"
#include <Seg.h>
#include <onewire.h>
#include <ds1302.h>

/* 变量声明区 */
unsigned char Key_Val,Key_Down,Key_Old,Key_Up,Key_Interrupt_Val;//按键专用变量
unsigned char Key_Slow_Down;//按键减速专用变量
unsigned char Seg_Buf[8] = {10,10,10,10,10,10,10,10};//数码管显示数据存放数组
unsigned char Seg_Pos;//数码管扫描专用变量
idata unsigned char Seg_Disp_Mode = 0;   //模式0温度显示  模式1时钟显示  模式2参数界面
idata unsigned int Seg_Slow_Down;//数码管减速专用变量
idata unsigned char ucLed[8] = {0,0,0,0,0,0,0,0};//Led显示数据存放数组
idata float temperature; //实时温度变量
idata unsigned char  temperature_patram = 23;     //10-99  
idata unsigned int Temperature_Slow_Down;//数码管减速专用变量
pdata unsigned char ucRtc[3] = {12,59,55};
idata bit Rtc_Disp_Mode = 0;  //0-小时分钟 1-分钟秒钟
idata bit Mode_Control = 0; // 0-温度控制 1-时钟控制
idata bit Relay_0_Flag;     //温度控制标志位
idata bit Relay_1_Flag;				//时间控制标志位
idata int Rtc_5000ms;
idata int flash_100ms;
idata bit Led_Control = 0;

/* 键盘处理函数 */
void Key_Proc()
{
    if(Key_Slow_Down<10) return;
    Key_Slow_Down = 0;//键盘减速程序

    Key_Val = Key_Read();//实时读取键码值
    Key_Down = Key_Val & (Key_Old ^ Key_Val);//捕捉按键下降沿
    Key_Up = ~Key_Val & (Key_Old ^ Key_Val);//捕捉按键上降沿
    Key_Old = Key_Val;//辅助扫描变量
	
	switch(Key_Down)
	{
		case 12:
			if(++Seg_Disp_Mode == 3){Seg_Disp_Mode = 0;}
			break;
		case 13:
			Mode_Control ^= 1;   
			break;
  //s16加参数
		case 16:
			if(Seg_Disp_Mode == 2 && (++temperature_patram == 100)){temperature_patram = 99;}break;
	//s17减参数
		case 17:
			if(Seg_Disp_Mode == 2 && (--temperature_patram == 9)){temperature_patram = 10;}break;
	}
	//减参数，判断Rtc模式
	if(Key_Val == 17)
	{Rtc_Disp_Mode = 1;}
	else 
	{Rtc_Disp_Mode = 0;}
	
	}	

	
/* 信息处理函数 */
void Seg_Proc()
{
    if(Seg_Slow_Down<100) return;
    Seg_Slow_Down = 0;//数码管减速程序


	/*信息显示函数*/
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
/* 其他显示函数 */
void Led_Proc()
{
	//温度控制模式继电器在温度大于参数时继电器咬合
	if(Mode_Control == 0)
	{Relay_0_Flag = temperature > temperature_patram;
		Relay(Relay_0_Flag);
	}
		//时间控制模式时间为整点时继电器咬合
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
/*温度获取函数*/
void Get_temperatrue()
{
	if(Temperature_Slow_Down < 500) return;
	else Temperature_Slow_Down = 0;
	
	temperature = rd_temperature();

}

/*时间获取函数*/
void Get_time()
{
	Read_Rtc(ucRtc);

}


/* 定时器0中断初始化函数 */
void Timer0Init(void)		//1毫秒@12.000MHz
{
    AUXR &= 0x7F;		//定时器时钟12T模式
    TMOD &= 0xF0;		//设置定时器模式
    TL0 = 0x18;		//设置定时初始值
    TH0 = 0xFC;		//设置定时初始值
    TF0 = 0;		//清除TF0标志
    TR0 = 1;		//定时器0开始计时
    ET0 = 1;    //定时器中断0打开
    EA = 1;     //总中断打开
}

/* 定时器0中断服务函数 */
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
    if(++Seg_Pos == 8) Seg_Pos = 0;//数码管显示专用
     // 数码管显示处理
    if (Seg_Buf[Seg_Pos] > 20)
        Seg_Disp(Seg_Pos, Seg_Buf[Seg_Pos] - ',', 1); // 带小数点
    else
        Seg_Disp(Seg_Pos, Seg_Buf[Seg_Pos], 0); // 无小数点
    
   
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