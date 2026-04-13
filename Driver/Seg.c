#include <Seg.h>

unsigned char seg_dula[] = {0xc0,0xf9,0xa4,0xb0,0x99,0x92,0x82,0xf8,0x80,0x90,0xff,0x88,0xc1,0xbf};  //11A12U13-
unsigned char seg_wela[] = {0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80};

void Seg_Disp(unsigned char wela,dula,point)
{
	unsigned char temp;
	
	P0 = 0xff;
	
	// 第一次操作P2锁存器 - 打开位选锁存器
	temp = P2 & 0x1f;    // 保留P2的低5位
	temp = temp | 0xe0;  // 与0xe0进行或操作
	P2 = temp;           // 写入P2
	temp = P2 & 0x1f;    // 保留P2的低5位
	P2 = temp;           // 写入P2，关闭锁存器

	P0 = seg_wela[wela];
	
	// 第二次操作P2锁存器 - 打开段选锁存器
	temp = P2 & 0x1f;    // 保留P2的低5位
	temp = temp | 0xc0;  // 与0xc0进行或操作
	P2 = temp;           // 写入P2
	temp = P2 & 0x1f;    // 保留P2的低5位
	P2 = temp;           // 写入P2，关闭锁存器
	
	P0 = seg_dula[dula];
	if(point) P0 &= 0x7f;
	
	// 第三次操作P2锁存器 - 再次打开位选锁存器
	temp = P2 & 0x1f;    // 保留P2的低5位
	temp = temp | 0xe0;  // 与0xe0进行或操作
	P2 = temp;           // 写入P2
	temp = P2 & 0x1f;    // 保留P2的低5位
	P2 = temp;           // 写入P2，关闭锁存器
}