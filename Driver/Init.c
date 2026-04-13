#include <Init.h>

void System_Init()
{
	unsigned char temp;
	
	P0 = 0xff;
	
	// 第一次操作P2锁存器
	temp = P2 & 0x1f;    // 保留P2的低5位
	temp = temp | 0x80;  // 与0x80进行或操作
	P2 = temp;           // 写入P2
	temp = P2 & 0x1f;    // 保留P2的低5位
	P2 = temp;           // 写入P2，关闭锁存器
	
	P0 = 0x00;
	
	// 第二次操作P2锁存器
	temp = P2 & 0x1f;    // 保留P2的低5位
	temp = temp | 0xa0;  // 与0xa0进行或操作
	P2 = temp;           // 写入P2
	temp = P2 & 0x1f;    // 保留P2的低5位
	P2 = temp;           // 写入P2，关闭锁存器
}