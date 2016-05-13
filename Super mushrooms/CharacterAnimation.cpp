//播放字符动画
#include<iostream>
#include<string.h>
#include<cstdio>
#include<time.h>
#include <conio.h>
#include<windows.h>
#include <mmsystem.h>
#include "CharacterAnimation.h"

typedef BOOL(WINAPI *PROCSETCONSOLEFONT)(HANDLE, DWORD);//
PROCSETCONSOLEFONT SetConsoleFont;						//设置字体大小要用


#pragma comment(lib,"winmm.lib")
using namespace std;
MCIERROR mciError;

//设置字体大小模式
void setFontSizeMode(int n);


//隐藏滚动条
void remove_scrollbar();


//全屏
void fullscreen()
{
	SetConsoleDisplayMode(GetStdHandle(STD_OUTPUT_HANDLE), CONSOLE_FULLSCREEN_MODE, 0);
	setFontSizeMode(7);
	system("mode con cols=1920 lines=1080");
	remove_scrollbar();

}


void playMusic()
{
	//MCIERROR mciError;
	//mciError = mciSendString(TEXT("open BadApple.mp3 alias mysong"), NULL, 0, NULL);

	if (mciError)
	{
		exit(1);
	}

	mciSendString(TEXT("play mySong from 0"), NULL, 0, NULL);
}


void gotoxy(int x, int y);


void PrintChar(char *ch, UINT count, UINT x, UINT y)  //在坐标(x,y)处输出字符串ch,ch里有count个字符
{
	//Sleep(10);//慢慢放，用于测试
	HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
	COORD pos;
	ULONG unuse;

	pos.X = x;
	pos.Y = y;

	CONSOLE_SCREEN_BUFFER_INFO bInfo; // 窗口缓冲区信息
	GetConsoleScreenBufferInfo(h, &bInfo);
	WriteConsoleOutputCharacterA(h, ch, count, pos, &unuse);
}



CharacterAnimation::CharacterAnimation(char * txt ,char * music ,int xx ,int yy,int fps)
	:XX0(xx),
	YY0(yy)
{
	fp = fopen(txt, "r");
	char Music[50] = "open "; 
	char tt[] = " alias mysong ";
	strcat(Music, music);
	strcat(Music, tt);
	mciError = mciSendString(TEXT(Music), NULL, 0, NULL);
	frame = 0;
	caf = fps;
}
CharacterAnimation::~CharacterAnimation()
{

}


void CharacterAnimation::recursur()
{
	HANDLE hout;
	COORD coord;
	coord.X = 0;
	coord.Y = 0;
	hout = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleCursorPosition(hout, coord);
}


void CharacterAnimation::display()
{
	fullscreen();
	const int XX = XX0;
	const int YY = YY0;
	playMusic();
	
	fseek(fp , 0L, SEEK_SET);            /*定位文件读写指针*/
	int count = 0;
	char Line[1925];
	int flag = 0;                       //1时就应该退出了
	while (!feof(fp))
	{
		if (flag == 1)
		{
			mciSendString(TEXT("stop mySong "), NULL, 0, NULL);
			mciSendString(TEXT("close mySong "), NULL, 0, NULL);
			break;
		}

		//caf = 26;
		for (int loop = 0; loop < YY; loop++)
		{
			if (_kbhit())//播放途中，按键ENTER W S则退出
			{
				char ch = _getch();
				if (ch == 13 || ch == 'w' || ch == 's')
				{
					system("cls");
					flag = 1;
					break;
				}

			}
			if (fgets(Line, XX + 2, fp) == NULL)
			{
				break;
			}
			Line[XX + 1] = '\0';
			PrintChar(Line, XX + 1, 0, loop);
		}
		Sleep(caf);
		count++;
	}
	mciSendString(TEXT("stop mySong "), NULL, 0, NULL);
	mciSendString(TEXT("close mySong "), NULL, 0, NULL);
}


