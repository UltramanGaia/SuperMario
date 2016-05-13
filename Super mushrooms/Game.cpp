#include <iostream>
#include <conio.h>
#include <math.h>
#include <Windows.h>
#include<string.h>
#include<time.h>
#include<cstdio>
#include <mmsystem.h>
#include "Game.h"
#pragma comment(lib,"Winmm.lib") 
using namespace std;
void fullscreen();//全屏
void remove_scrollbar();//隐藏滚动条
void hideCursor();//隐藏光标
void gotoxy(int x, int y);//移动光标到x,y
void gotoxyAndPutchar(int x, int y ,char c);//移动光标到x,y打印c
void hideCursor()
{
	CONSOLE_CURSOR_INFO cursor_info = { 1,0 };
	SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursor_info);
}
void gotoxy(int x, int y)
{
	COORD coord; // coordinates
	coord.X = x; coord.Y = y; // X and Y coordinates
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord); // moves to the coordinates
}
void gotoxyAndPutchar(int x, int y, char c)//移动光标到x,y打印c
{
	gotoxy(x, y);
	putchar(c);
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
	hideCursor();
	remove_scrollbar();
}
void PrintChar(char *ch, UINT count, UINT x, UINT y)  //在坐标(x,y)处输出字符串ch,ch里有count个字符
{
	//
	Sleep(1);//慢慢放，用于测试
	HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
	COORD pos;
	ULONG unuse;

	pos.X = x;
	pos.Y = y;

	CONSOLE_SCREEN_BUFFER_INFO bInfo; // 窗口缓冲区信息
	GetConsoleScreenBufferInfo(h, &bInfo);
	WriteConsoleOutputCharacterA(h, ch, count, pos, &unuse);
}
game::game()
{

}
game::~game()
{

}
void game::start()
{
	life = 3;
	hero.id = 1;
	hero.x = XX - 1;
	hero.y = YY;
	hero.turn = 0;
	hero.jumpStep = 0;
	win = 0;
	pause = 0;
	score = 0;
	centerLine = hero.x + MX / 2;
	fullscreen();
	init();
	mciSendString("play bgm  repeat", NULL, 0, NULL);

	while (1)
	{
		show();
		char ch;
		if (kbhit())
		{
			ch = _getch();
			control(ch);
			//ch = '\0';
		}
		judge();
		Sleep(20);
	}
}
void game::init()
{


	//加载背景
	fp = fopen("BMapAndMap.txt", "r");
	char ch;
	int i = 0, j = 0, k = 0;
	while (!feof(fp))
	{
		ch = fgetc(fp);
		if (ch != '\n')
		{
			BMap[i][k * MX + j] = ch;
			++j;
			if (j == MX)
			{
				++i;
				j = 0;
			}
			if (i == MY)
			{
				++k;
				i = 0;
			}
			if (k == MK)
			{
				break;
			}
		}
	}
/*
	for (int i = 0; i < 10; i++)
	{
		for (int j = 0; j < 16; j++)
		{
			cout << BMap[i][16*9+j] ;
		}
		cout << endl;
	}
*/
	//加载音乐
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

	//加载金币N_COIN
	coins[0].id = 1 , coins[0].x = 56 , coins[0].y = 39 , coins[0].turn = 0;
	coins[1].id = 1,  coins[1].x = 80, coins[1].y = 39, coins[1].turn = 0;
	coins[2].id = 1,  coins[2].x = 81, coins[2].y = 39, coins[2].turn = 0;
	coins[3].id = 1,  coins[3].x = 82, coins[3].y = 39, coins[3].turn = 0;
	coins[4].id = 1,  coins[4].x = 200, coins[4].y = 39, coins[4].turn = 0;


	//加载敌人N_EMERY
	emery[0].id = 1, emery[0].x = 55,  emery[0].y = 39, emery[0].turn = -1, emery[0].xLeftMost = emery[0].x - 10, emery[0].xRightMost = emery[0].x + 10;
	emery[1].id = 1, emery[1].x = 100, emery[1].y = 39, emery[1].turn = -1, emery[1].xLeftMost = emery[1].x - 10, emery[1].xRightMost = emery[1].x + 10;
	emery[2].id = 1, emery[2].x = 101, emery[2].y = 39, emery[2].turn = -1, emery[2].xLeftMost = emery[2].x - 10, emery[2].xRightMost = emery[2].x + 10;
	emery[3].id = 1, emery[3].x = 150, emery[3].y = 39, emery[3].turn = -1, emery[3].xLeftMost = emery[3].x - 10, emery[3].xRightMost = emery[3].x + 10;
	emery[4].id = 1, emery[4].x = 156, emery[4].y = 39, emery[4].turn = -1, emery[4].xLeftMost = emery[4].x - 10, emery[4].xRightMost = emery[4].x + 10;

}
void game::show()
{
	if (hero.x >= centerLine)
		centerLine++;
	if (centerLine > MX * MK - MX / 2)
		centerLine = MX * MK - MX / 2;

	char abc[MY][MX + 1];
	//显示背景
	//int i = 0, j = 0;
	for (int i = 0; i < MY; i++)
	{
		strncpy(abc[i], &BMap[i][centerLine - MX /2], MX);
		abc[i][MX] = '\0';

	}
	//显示硬币
	for (int i = 0; i < N_COIN; i++)
	{
		if (coins[i].id == 0)
			continue;
		else if(coins[i].x > centerLine - MX / 2 && coins[i].x < centerLine + MX /2)
		{
			abc[coins[i].y][MX/2 - (centerLine - coins[i].x )] = '$';
		}
	}
	for (int i = 0; i < N_EMERY; i++)
	{
		if (emery[i].id == 0)
			continue;
		else if (emery[i].x >= centerLine - MX / 2 && emery[i].x <= centerLine + MX / 2)
		//else if (emery[i].x <= centerLine + MX / 2)
		{
			abc[emery[i].y][MX/2 - (centerLine - emery[i].x) ] = 'Q';
		}
	}

	abc[hero.y][MX / 2 - (centerLine - hero.x)] = 'A';

	//处理好 一股脑传过去
	for (int i = 0; i < MY; i++)
	{
		PrintChar(abc[i], MX + 1, 0, i);
	}
	if (hero.x >= MX*MK - MX / 2)
	{
		win = 1;
		end();
	}





	//PrintChar("AAA", 3+1, 50, 10);
	//gotoxyAndPutchar(50 ,-50 ,'A');


}
void game::control( char ch)
{

	switch (ch)
	{
	case 'w':up(); break;
	case 'a':left(); break;
	case 'd':right(); break;
	case 's': hero.turn = 0; break;
	case 'p':gamePause(); break;
	}

	return;

}
void game::up()
{
	if (hero.jumpStep != 0)
		return;
	else
	{
		mciSendString("play jump from 0", NULL, 0, NULL);
		hero.jumpStep = 1;
	}
}
void game::left()
{
	hero.x--;
	hero.turn = -1;
	if (HeroisHitWall())
	{
		hero.x++;
	}
}
void game::right()
{
	hero.x++;
	hero.turn = 1;
	if (HeroisHitWall())
	{
		hero.x--;
	}
}
bool game::HeroisHitWall()
{
	//int Pointx = MX / 2 - (centerLine - hero.x);
	//int Pointy = MY;
	if (BMap[hero.y][hero.x] == '#')
		return true;
	else
		return false;

}
void game::gamePause()
{
	system("cls");
	Sleep(2000);
}
bool game::HeroisLanded()
{
	if (BMap[hero.y + 1][hero.x] == '#')
		return true;
	else
		return false;
}
bool game::HeroUpHitWall()
{
	if (BMap[hero.y][hero.x] == '#')
	{
		mciSendString("play hit_wall from 0", NULL, 0, NULL);
		return true;
	}
	else
		return false;
}
bool game::iscoin()
{
	for (int i = 0; i < N_COIN; i++)
	{
		if (coins[i].id != 0)
		{
			if (hero.x == coins[i].x && hero.y == coins[i].y)
			{
				coins[i].id = 0;
				mciSendString("play coin from 0", NULL, 0, NULL);
				return true;
			}
				
		}
	}
	return false;

}
bool game::isEmery()
{
	for (int i = 0; i < N_EMERY; i++)
	{
		if (emery[i].id != 0)
		{
			if (emery[i].x == hero.x && emery[i].y == hero.y)
			{
				return true;
			}
		}
	}
	return false;
}
void  game::judge()
{
	//移动敌人
	for (int i = 0; i < N_EMERY; i++)
	{
		if (emery[i].id != 0)
		{
			if (emery[i].x == emery[i].xLeftMost)
			{
				emery[i].turn = 1;
			}
			if (emery[i].x == emery[i].xRightMost)
			{
				emery[i].turn = -1;
			}
			emery[i].x += abs(emery[i].turn)/ emery[i].turn;
		}
	}

	int tempy = hero.y;
	if (centerLine - hero.x >= MX / 2)
		hero.x = centerLine - MX / 2;
	if (hero.jumpStep != 0)//腾空
	{
		hero.y -= abs(hero.jumpStep) / hero.jumpStep;
		if (hero.turn == -1)
			left();
		else if (hero.turn == 1)
			right();
		hero.jumpStep++;
		if (hero.jumpStep == JH)
			hero.jumpStep = -(JH - 1);
		if (HeroisLanded())
		{
			hero.jumpStep = 0;
		}
		if (HeroUpHitWall())
		{
			hero.jumpStep = -hero.jumpStep;
			hero.y++;
		}
	}
	if ((!HeroisLanded()) && hero.jumpStep == 0)
	{
		hero.jumpStep = -1;
	}
	if (iscoin())
	{
		score += 10;
	}
	//if (isEmery() || hero.y >= MY - 2 )
	if (isEmery() || hero.y >= MY )
	{
		end();

	}
	
}
void game::end()
{
	if (win == 1)
	{
		mciSendString("stop   bgm ", NULL, 0, NULL);
		mciSendString("play  win from 0", NULL, 0, NULL);
	}
	else
	{
		mciSendString("stop   bgm ", NULL, 0, NULL);
		mciSendString("play  dead2 from 0", NULL, 0, NULL);

	}
	gamePause();
	Sleep(3000);
	start();
	//game();
	//start();
	/*
	if (win == 1)
	{
		pause = 0;
		score = 0;
		life = 0;
		mciSendString("play win", NULL, 0, NULL);
		Sleep(3000);
		mciSendString("close win", NULL, 0, NULL);
	}
	else
	{
		score = 0;
		life--;
		if (life == 0)
		{
			mciSendString("play dead1", NULL, 0, NULL);
			mciSendString("close dead1", NULL, 0, NULL);
		}
		else
		{
			mciSendString("play dead2", NULL, 0, NULL);
			start();

		}
	}
	*/
}


