
#ifndef MAIN_TUNCTION_H
#define MAIN_TUNCTION_H
#include <windows.h>
#include <iostream>
using namespace std;

void init();//初始化
void fullscreen();//全屏
void remove_scrollbar();//隐藏滚动条


//初始化
void init()
{
	fullscreen();
	mciSendString("open 背景音乐.mp3 alias bgm", NULL, 0, NULL);
	mciSendString("open 子弹.mp3 alias bullet", NULL, 0, NULL);
	mciSendString("open 金币.mp3 alias coin", NULL, 0, NULL);
	mciSendString("open 跳.mp3 alias jump", NULL, 0, NULL);
	mciSendString("open 子弹打到敌人.mp3 alias hit_enemy", NULL, 0, NULL);
	mciSendString("open 子弹撞墙.mp3 alias hit_wall", NULL, 0, NULL);
	mciSendString("open 踩敌人.mp3 alias step", NULL, 0, NULL);
	mciSendString("open 吃到武器.mp3 alias arm", NULL, 0, NULL);
	mciSendString("open 胜利.mp3 alias win", NULL, 0, NULL);
	mciSendString("open 死亡1.mp3 alias dead1", NULL, 0, NULL);
	mciSendString("open 死亡2.mp3 alias dead2", NULL, 0, NULL);

}

//隐藏滚动条
void remove_scrollbar()
{
	HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO info;
	GetConsoleScreenBufferInfo(handle, &info);
	COORD new_size =
	{
		info.srWindow.Right - info.srWindow.Left + 1,
		info.srWindow.Bottom - info.srWindow.Top + 1
	};
	SetConsoleScreenBufferSize(handle, new_size);
}


//全屏
void fullscreen()
{
	SetConsoleDisplayMode(GetStdHandle(STD_OUTPUT_HANDLE), CONSOLE_FULLSCREEN_MODE, 0);

	remove_scrollbar();
}


#endif // 