#pragma runtime_checks( "", off )
#include <iostream>
#include <conio.h>
#include <math.h>
#include <Windows.h>
#include<string.h>
#include<time.h>
#include <fstream>
#include<cstdio>
#include <mmsystem.h>
#include "SuperMushRoom.h"
#include "CharacterAnimation.h"
#include "duel.h"
#pragma comment(lib,"Winmm.lib") 
using namespace std;



HANDLE hStdout, hNewScreenBuffer;                                    //两个句柄 , 对应两个缓冲区
CHAR_INFO chiBuffer[MY*MX];                                          //[MY][MX];用于储存从标准缓冲区到新缓冲区的字符
void remove_scrollbar();                                             //隐藏滚动条
void hideCursor();                                                   //隐藏光标
void setFontSizeMode(int n);                                         //设置字体大小模式
void gotoxy(int x, int y);                                           //移动光标到x,y
void gotoxyAndPutchar(int x, int y, char c, int color = 0x07);       //移动光标到x,y打印c,颜色color



//隐藏光标
void hideCursor()
{
	CONSOLE_CURSOR_INFO cursor_info = { 1,0 };
	SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursor_info);
	SetConsoleCursorInfo(hNewScreenBuffer, &cursor_info);
}

//移动光标到x,y
void gotoxy(int x, int y)
{
	COORD coord;			  // 坐标
	coord.X = x; coord.Y = y; // 坐标变量赋x , y 
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord); //移动光标到坐标(x,y) 
}

//移动光标到x,y打印c,颜色color
void gotoxyAndPutchar(int x, int y, char c, int color)//移动光标到x,y打印c
{
	if (x >= 0 && x <= MX && y >= 0 && y <= MY)
	{
		gotoxy(x, y);
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
		putchar(c);
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 0x07);
	}

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
	//
	SetConsoleScreenBufferSize(hNewScreenBuffer, new_size);
}

//设置字体大小模式
void setFontSizeMode(int n)
{
	typedef BOOL(WINAPI *PROCSETCONSOLEFONT)(HANDLE, DWORD);//
	PROCSETCONSOLEFONT SetConsoleFont;						//设置字体大小要用

	HANDLE hConsole;
	hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	HMODULE hKernel32 = GetModuleHandle("kernel32");
	SetConsoleFont = (PROCSETCONSOLEFONT)GetProcAddress(hKernel32, "SetConsoleFont");
	SetConsoleFont(hConsole, n);

	/*字体大小模式如下
	x?  y? 模式
	3   3  0
	4   4  1
	5   5  2
	6   6  3
	8   8  4
	16  16 5
	5   5  6
	6   6  7
	7   7  8
	8   8  9
	16  16 10
	*/
}


SuperMushRoom::SuperMushRoom()
{

	chapter = 1;																	   //第一关
	life = 3;                                                                          //生命数
	hero.id = 1;
	hero.x = XX;																	   //人物出现位置
	hero.y = YY;
	hero.turn = 0;                                                                     //人物朝向
	hero.state = 0;
	hero.jump = 0;                                                                     //不跳跃

	v0 = 0;																				//角色跳跃的初速度
	h = 0;																				//角色跳跃的高度
	t = 0;																				//角色跳跃的时间

	//
	arm = 0;                                                                            //没有装备枪
	n_bullet = 0;                                                                       //第几颗炮弹
	score = 0;                                                                          //得分
	centerLine = MX / 2;                                                                //初始中线
	GameState = 0;																		//游戏进行到什么状态
																						//0     普通版
																						//1     逆袭版
																						//2     决斗版
																						//-1     退出

}

SuperMushRoom::~SuperMushRoom()
{
	//使显示的恢复为标准缓冲区	 
	SetConsoleActiveScreenBuffer(hStdout);
}

//游戏开始
void SuperMushRoom::start()
{

	while (GameState != -1)
	{
		if (chapter <= 2)
			init();//初始化,地图等
		else
			randInit();// 随机初始化
		if (GameState == 0)
			mciSendString("play bgm  repeat", NULL, 0, NULL);//重复播放背景音乐
		else if (GameState == 1)
			mciSendString("play Sbgm  repeat", NULL, 0, NULL);//重复播放背景音乐


		while (1)
		{
			if (GameState == -1)
			{
				while (_kbhit())
					char ch = _getch();
				break;
			}


			show();
			control();
			if (GameState == 10||GameState == 20)//正常返回
			{
				GameState = GameState / 10 - 1;
				break;
			}
			if (GameState == -1)//退出
				break;
			judge();
			Sleep(20);
		}
	}
}

//初始化,地图等
void SuperMushRoom::init()
{
	//设置窗口
	setWindows();

	//加载背景
	loadBackGround();

	//加载音乐
	loadMusic();

	//加载金币N_COIN
	loadCoins();

	//加载敌人N_EMERY
	loadEnemy();

	//初始化子弹
	for (int i = 0; i < 20; i++)
	{
		bullet[i].id = 0;
		bullet[i].x = -1;
		bullet[i].y = -1;

	}

	//获得武器的花
	switch (chapter)
	{
	case 1:weapon.id = 1, weapon.x = 357, weapon.y = 10; break;
	case 2:weapon.id = 1, weapon.x = 610, weapon.y = 23; break;
	}

}

// 随机初始化
void SuperMushRoom::randInit()
{
	//设置窗口
	setWindows();

	//基础地图
	fp = fopen(".\\res\\chapter2plus.txt", "r");

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


	srand(unsigned int(time(0)));//时间种子


	struct
	{
		int x;
		int length;
	}xuanYa[10], firstStair[10], secondStair[10];//储存悬崖，一级楼梯，二级楼梯的结构体

	//悬崖
	int numOfXuanYa = 3 + rand() % 7;//3--7处悬崖
	for (int i = 0; i < numOfXuanYa; i++)
	{
		xuanYa[i].x = (rand() % 7 + 1) * 260 + rand() % 260;
		xuanYa[i].length = rand() % 30 + 30;

		for (int p = 70; p < 80; p++)
			for (int q = xuanYa[i].x; q <= xuanYa[i].x + xuanYa[i].length; q++)
				BMap[p][q] = ' ';
	}

	//一级阶梯

	int numOfFirstStair = 0;
	int numOfSecondStair = 0;
	//如果悬崖过于宽，一级阶梯
	for (int i = 0; i < numOfXuanYa; i++)
	{
		if (xuanYa[i].length > 30)
		{
			firstStair[numOfFirstStair].x = xuanYa[i].x + 10;
			firstStair[numOfFirstStair].length = rand() % 20 + 5;

			for (int p = 47; p < 47 + 3; p++)
				for (int q = firstStair[numOfFirstStair].x; q <= firstStair[numOfFirstStair].x + firstStair[numOfFirstStair].length; q++)
					BMap[p][q] = '#';
			numOfFirstStair++;

			//是否产生二级阶梯
			if (rand() % 5 == 0)
			{
				secondStair[numOfSecondStair].x = firstStair[numOfFirstStair - 1].x + rand() % 15 - 5;
				secondStair[numOfSecondStair].length = rand() % 10 + 5;

				for (int p = 24; p < 24 + 3; p++)
					for (int q = secondStair[numOfSecondStair].x; q <= secondStair[numOfSecondStair].x + secondStair[numOfSecondStair].length; q++)
						BMap[p][q] = '#';

				numOfSecondStair++;
			}

		}
	}

	//正式随机产生一级阶梯
	int temp = rand() % 5 + numOfFirstStair;
	while (numOfFirstStair < temp)
	{
		firstStair[numOfFirstStair].x = (rand() % 7 + 1) * 260 + rand() % 260;
		firstStair[numOfFirstStair].length = rand() % 40 + 5;

		for (int p = 47; p < 47 + 3; p++)
			for (int q = firstStair[numOfFirstStair].x; q <= firstStair[numOfFirstStair].x + firstStair[numOfFirstStair].length; q++)
				BMap[p][q] = '#';
		numOfFirstStair++;

		//是否产生二级阶梯
		if (rand() % 5 == 0)
		{
			secondStair[numOfSecondStair].x = firstStair[numOfFirstStair - 1].x + rand() % 15 - 5;
			secondStair[numOfSecondStair].length = rand() % 10 + 5;

			for (int p = 24; p < 24 + 3; p++)
				for (int q = secondStair[numOfSecondStair].x; q <= secondStair[numOfSecondStair].x + secondStair[numOfSecondStair].length; q++)
					BMap[p][q] = '#';

			numOfSecondStair++;

		}
	}


	//加载音乐
	loadMusic();

	//加载金币N_COIN
	for (int i = 0; i < N_COIN; i++)
	{
		if (rand() % 4 == 0 && numOfSecondStair != 0)//落在第二级阶梯
		{
			coins[i].id = 1;
			coins[i].x = secondStair[rand() % numOfSecondStair].x + rand() % 20;
			coins[i].y = 42 - 23;
			coins[i].turn = 0;
		}
		else//落在第一级阶梯
		{
			coins[i].id = 1;
			coins[i].x = firstStair[rand() % numOfFirstStair].x + rand() % 20;
			coins[i].y = 42;
			coins[i].turn = 0;
		}

	}

	//加载敌人N_EMERY
	for (int i = 0; i < N_EMERY; i++)
	{
		enemy[i].id = 1;
		enemy[i].x = (rand() % 7 + 1) * 260 + rand() % 260;
		switch (rand() % 8)
		{
		case 0:enemy[i].y = 23; break;
		case 1:
		case 2:
		case 3:enemy[i].y = 37; break;
		default:enemy[i].y = 60; break;
		}

		enemy[i].turn = (rand() % 2 == 0) ? 1 : 0;
		enemy[i].xLeftMost = enemy[i].x - rand() % 50;
		enemy[i].xRightMost = enemy[i].x + rand() % 50;
	}

	//初始化子弹
	for (int i = 0; i < 20; i++)
	{
		bullet[i].id = 0;
		bullet[i].x = -1;
		bullet[i].y = -1;

	}

	//获得武器的花的位置
	weapon.id = 1;
	weapon.x = (rand() % 5 + 1) * 260 + rand() % 260;
	weapon.y = (rand() % 4 == 0) ? 19 : 56;


}

//设置窗口
void SuperMushRoom::setWindows()
{
	SetConsoleDisplayMode(GetStdHandle(STD_OUTPUT_HANDLE), CONSOLE_FULLSCREEN_MODE, 0);//全屏
	setFontSizeMode(7);                                                                //设置字体模式7
	system("mode con cols=300 lines=300");                                             //控制台行列
	remove_scrollbar();                                                               //隐藏滚动条

	// 创建一个句柄 指向STDOUT ，从那里copy数据到新建的缓冲区
	hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	// 创建一个新缓冲区  , 数据copy到这里 
	hNewScreenBuffer = CreateConsoleScreenBuffer(
		GENERIC_READ |           // 读/写 权限 
		GENERIC_WRITE,
		FILE_SHARE_READ |
		FILE_SHARE_WRITE,        // 共享
		NULL,                    // 默认安全属性
		CONSOLE_TEXTMODE_BUFFER, // must be TEXTMODE 
		NULL);                   // reserved; must be NULL 

	//使显示的是新建的缓冲区	 
	SetConsoleActiveScreenBuffer(hNewScreenBuffer);
	//隐藏光标
	hideCursor();
}

//加载背景到数组中
void SuperMushRoom::loadBackGround()
{
	//加载背景
	switch (chapter)
	{
	case 1:fp = fopen(".\\res\\chapter1_background.txt", "r"); break;
	case 2:fp = fopen(".\\res\\chapter2_background.txt", "r"); break;
	}
	if (fp == NULL)
	{
		cerr << "Open Background error" << endl;
		exit(1);
	}
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
}

//加载音乐
void SuperMushRoom::loadMusic()
{

	mciSendString("open .\\res\\背景音乐.mp3 alias bgm", NULL, 0, NULL);
	mciSendString("open .\\res\\子弹.mp3 alias bullet", NULL, 0, NULL);
	mciSendString("open .\\res\\金币.mp3 alias coin", NULL, 0, NULL);
	mciSendString("open .\\res\\跳.mp3 alias jump", NULL, 0, NULL);
	mciSendString("open .\\res\\子弹打到敌人.mp3 alias hit_enemy", NULL, 0, NULL);
	mciSendString("open .\\res\\子弹撞墙.mp3 alias hit_wall", NULL, 0, NULL);
	mciSendString("open .\\res\\撞击砖块.mp3 alias hit_brick", NULL, 0, NULL);
	mciSendString("open .\\res\\踩敌人.mp3 alias step", NULL, 0, NULL);
	mciSendString("open .\\res\\吃到武器.mp3 alias arm", NULL, 0, NULL);
	mciSendString("open .\\res\\胜利.mp3 alias win", NULL, 0, NULL);
	mciSendString("open .\\res\\死亡1.mp3 alias dead1", NULL, 0, NULL);
	mciSendString("open .\\res\\游戏结束.mp3 alias gameover", NULL, 0, NULL);

	mciSendString("open .\\res\\魂斗罗背景.mp3 alias Sbgm", NULL, 0, NULL);
	mciSendString("open .\\res\\魂斗罗shoot.mp3 alias Sbullet", NULL, 0, NULL);
	mciSendString("open .\\res\\魂斗罗boom.mp3 alias Shit_enemy", NULL, 0, NULL);
	mciSendString("open .\\res\\魂斗罗boom.mp3 alias Sstep", NULL, 0, NULL);
	mciSendString("open .\\res\\魂斗罗吃到武器.mp3 alias Sarm", NULL, 0, NULL);
	mciSendString("open .\\res\\魂斗罗死亡1.mp3 alias Sdead1", NULL, 0, NULL);


}

//加载金币N_COIN
void SuperMushRoom::loadCoins()
{
	ifstream fcoin;
	switch (chapter)
	{
	case 1:fcoin.open(".\\res\\chapter1_coins.txt"); break;
	case 2:fcoin.open(".\\res\\chapter2_coins.txt"); break;
	}
	if (!fcoin)
	{
		cerr << "open files of coins error" << endl;
		exit(1);
	}
	for (int i = 0; i < N_COIN; i++)
	{
		fcoin >> coins[i].id >> coins[i].x >> coins[i].y >> coins[i].turn;
	}
	fcoin.close();

}

//加载敌人N_EMERY
void SuperMushRoom::loadEnemy()
{
	ifstream fenemy;
	switch (chapter)
	{
	case 1:fenemy.open(".\\res\\chapter1_enemy.txt"); break;
	case 2:fenemy.open(".\\res\\chapter2_enemy.txt"); break;
	}
	if (!fenemy)
	{
		cerr << "open files of enemy error" << endl;
		exit(1);
	}
	for (int i = 0; i < N_EMERY; i++)
	{
		fenemy >> enemy[i].id >> enemy[i].x >> enemy[i].y >> enemy[i].turn >> enemy[i].xLeftMost >> enemy[i].xRightMost;
	}
	fenemy.close();

}

//显示画面
void SuperMushRoom::show()
{
	if (hero.x >= centerLine)          //右移屏幕
		centerLine += STEP;
	if (centerLine > MX * MK - MX / 2)//到达最后一张图后半部分，不移动
		centerLine = MX * MK - MX / 2;


	//显示背景
	showBackground();

	//显示金币
	showCoins();

	//显示子弹
	showBullet();

	//显示获取武器
	showWeapon();

	//显示敌人
	showEnemy();

	//显示英雄
	showHero();

	//显示分数
	showScore();

	//显示关卡
	showChapter();

	//显示生命
	showLife();

	//传输标准缓冲区中的画面到显示的缓冲区
	copyToBuffer(hStdout, 0, 0, MY, 260, hNewScreenBuffer, 0, 0, MY, 260);
}

//显示背景
void SuperMushRoom::showBackground()
{
	char abc;
	gotoxy(0, 0);
	for (int i = 0; i < MY; i++)
	{
		abc = BMap[i][centerLine - MX / 2 + MX];
		BMap[i][centerLine - MX / 2 + MX] = '\0';
		puts(&BMap[i][centerLine - MX / 2]);
		BMap[i][centerLine - MX / 2 + MX] = abc;
	}
}

//显示金币
void SuperMushRoom::showCoins()
{
	for (int i = 0; i < N_COIN; i++)
	{
		if (coins[i].id == 0)
			continue;
		else if (coins[i].x > centerLine - MX / 2 && coins[i].x < centerLine + MX / 2)
		{
			gotoxyAndPutchar(MX / 2 - (centerLine - coins[i].x) + 1, coins[i].y - 2, '$', 0x06);
			gotoxyAndPutchar(MX / 2 - (centerLine - coins[i].x) + 2, coins[i].y - 2, '$', 0x06);

			gotoxyAndPutchar(MX / 2 - (centerLine - coins[i].x) + 0, coins[i].y - 1, '$', 0x06);
			gotoxyAndPutchar(MX / 2 - (centerLine - coins[i].x) + 3, coins[i].y - 1, '$', 0x06);

			gotoxyAndPutchar(MX / 2 - (centerLine - coins[i].x) + 0, coins[i].y - 0, '$', 0x06);
			gotoxyAndPutchar(MX / 2 - (centerLine - coins[i].x) + 3, coins[i].y - 0, '$', 0x06);

			gotoxyAndPutchar(MX / 2 - (centerLine - coins[i].x) + 1, coins[i].y + 1, '$', 0x06);
			gotoxyAndPutchar(MX / 2 - (centerLine - coins[i].x) + 2, coins[i].y + 1, '$', 0x06);
		}
	}
}

//显示子弹
void SuperMushRoom::showBullet()
{
	for (int i = 0; i < 20; i++)
	{
		if (bullet[i].id == 0)
			continue;
		else if (bullet[i].x >= centerLine - MX / 2 && bullet[i].x <= centerLine + MX / 2)
		{
			if (bullet[i].turn == 1)
			{
			
			gotoxyAndPutchar(MX / 2 - (centerLine - bullet[i].x) - 1, bullet[i].y - 1, 'O', 0x0f);
			gotoxyAndPutchar(MX / 2 - (centerLine - bullet[i].x), bullet[i].y, 'O', 0x0f);
			gotoxyAndPutchar(MX / 2 - (centerLine - bullet[i].x) - 1, bullet[i].y + 1, 'O', 0x0f);
			}
			else
			{
				gotoxyAndPutchar(MX / 2 - (centerLine - bullet[i].x) + 1, bullet[i].y - 1, 'O', 0x0f);
				gotoxyAndPutchar(MX / 2 - (centerLine - bullet[i].x), bullet[i].y, 'O', 0x0f);
				gotoxyAndPutchar(MX / 2 - (centerLine - bullet[i].x) + 1, bullet[i].y + 1, 'O', 0x0f);
			}
		}
	}
}

//显示获取武器
void SuperMushRoom::showWeapon()
{
	if (weapon.id == 1)
	{
		gotoxyAndPutchar(MX / 2 - (centerLine - weapon.x) + 4, weapon.y + 1, 'Q', 14);
		gotoxyAndPutchar(MX / 2 - (centerLine - weapon.x) + 5, weapon.y + 1, 'Q', 14);
		gotoxyAndPutchar(MX / 2 - (centerLine - weapon.x) + 6, weapon.y + 1, 'Q', 14);
		gotoxyAndPutchar(MX / 2 - (centerLine - weapon.x) + 7, weapon.y + 1, 'Q', 14);
		gotoxyAndPutchar(MX / 2 - (centerLine - weapon.x) + 8, weapon.y + 1, 'Q', 14);
		gotoxyAndPutchar(MX / 2 - (centerLine - weapon.x) + 9, weapon.y + 1, 'Q', 14);
		gotoxyAndPutchar(MX / 2 - (centerLine - weapon.x) + 10, weapon.y + 1, 'Q', 14);
		gotoxyAndPutchar(MX / 2 - (centerLine - weapon.x) + 11, weapon.y + 1, 'Q', 14);

		gotoxyAndPutchar(MX / 2 - (centerLine - weapon.x) + 2, weapon.y + 2, 'Q', 14);
		gotoxyAndPutchar(MX / 2 - (centerLine - weapon.x) + 3, weapon.y + 2, 'Q', 14);
		gotoxyAndPutchar(MX / 2 - (centerLine - weapon.x) + 4, weapon.y + 2, 'Q', 14);
		gotoxyAndPutchar(MX / 2 - (centerLine - weapon.x) + 5, weapon.y + 2, 'Q', 6);
		gotoxyAndPutchar(MX / 2 - (centerLine - weapon.x) + 6, weapon.y + 2, 'Q', 6);
		gotoxyAndPutchar(MX / 2 - (centerLine - weapon.x) + 7, weapon.y + 2, 'Q', 6);
		gotoxyAndPutchar(MX / 2 - (centerLine - weapon.x) + 8, weapon.y + 2, 'Q', 6);
		gotoxyAndPutchar(MX / 2 - (centerLine - weapon.x) + 9, weapon.y + 2, 'Q', 6);
		gotoxyAndPutchar(MX / 2 - (centerLine - weapon.x) + 10, weapon.y + 2, 'Q', 6);
		gotoxyAndPutchar(MX / 2 - (centerLine - weapon.x) + 11, weapon.y + 2, 'Q', 14);
		gotoxyAndPutchar(MX / 2 - (centerLine - weapon.x) + 12, weapon.y + 2, 'Q', 14);
		gotoxyAndPutchar(MX / 2 - (centerLine - weapon.x) + 13, weapon.y + 2, 'Q', 14);

		gotoxyAndPutchar(MX / 2 - (centerLine - weapon.x) + 1, weapon.y + 3, 'Q', 14);
		gotoxyAndPutchar(MX / 2 - (centerLine - weapon.x) + 2, weapon.y + 3, 'Q', 14);
		gotoxyAndPutchar(MX / 2 - (centerLine - weapon.x) + 3, weapon.y + 3, 'Q', 6);
		gotoxyAndPutchar(MX / 2 - (centerLine - weapon.x) + 4, weapon.y + 3, 'Q', 6);
		gotoxyAndPutchar(MX / 2 - (centerLine - weapon.x) + 5, weapon.y + 3, 'Q', 6);
		gotoxyAndPutchar(MX / 2 - (centerLine - weapon.x) + 7, weapon.y + 3, 'Q', 15);
		gotoxyAndPutchar(MX / 2 - (centerLine - weapon.x) + 8, weapon.y + 3, 'Q', 15);
		gotoxyAndPutchar(MX / 2 - (centerLine - weapon.x) + 10, weapon.y + 3, 'Q', 6);
		gotoxyAndPutchar(MX / 2 - (centerLine - weapon.x) + 11, weapon.y + 3, 'Q', 6);
		gotoxyAndPutchar(MX / 2 - (centerLine - weapon.x) + 12, weapon.y + 3, 'Q', 6);
		gotoxyAndPutchar(MX / 2 - (centerLine - weapon.x) + 13, weapon.y + 3, 'Q', 14);
		gotoxyAndPutchar(MX / 2 - (centerLine - weapon.x) + 14, weapon.y + 3, 'Q', 14);

		gotoxyAndPutchar(MX / 2 - (centerLine - weapon.x) + 1, weapon.y + 4, 'Q', 14);
		gotoxyAndPutchar(MX / 2 - (centerLine - weapon.x) + 2, weapon.y + 4, 'Q', 14);
		gotoxyAndPutchar(MX / 2 - (centerLine - weapon.x) + 3, weapon.y + 4, 'Q', 6);
		gotoxyAndPutchar(MX / 2 - (centerLine - weapon.x) + 4, weapon.y + 4, 'Q', 6);
		gotoxyAndPutchar(MX / 2 - (centerLine - weapon.x) + 5, weapon.y + 4, 'Q', 6);
		gotoxyAndPutchar(MX / 2 - (centerLine - weapon.x) + 7, weapon.y + 4, 'Q', 6);
		gotoxyAndPutchar(MX / 2 - (centerLine - weapon.x) + 8, weapon.y + 4, 'Q', 6);
		gotoxyAndPutchar(MX / 2 - (centerLine - weapon.x) + 10, weapon.y + 4, 'Q', 6);
		gotoxyAndPutchar(MX / 2 - (centerLine - weapon.x) + 11, weapon.y + 4, 'Q', 6);
		gotoxyAndPutchar(MX / 2 - (centerLine - weapon.x) + 12, weapon.y + 4, 'Q', 6);
		gotoxyAndPutchar(MX / 2 - (centerLine - weapon.x) + 13, weapon.y + 4, 'Q', 14);
		gotoxyAndPutchar(MX / 2 - (centerLine - weapon.x) + 14, weapon.y + 4, 'Q', 14);

		gotoxyAndPutchar(MX / 2 - (centerLine - weapon.x) + 1, weapon.y + 5, 'Q', 14);
		gotoxyAndPutchar(MX / 2 - (centerLine - weapon.x) + 2, weapon.y + 5, 'Q', 14);
		gotoxyAndPutchar(MX / 2 - (centerLine - weapon.x) + 3, weapon.y + 5, 'Q', 14);
		gotoxyAndPutchar(MX / 2 - (centerLine - weapon.x) + 4, weapon.y + 5, 'Q', 14);
		gotoxyAndPutchar(MX / 2 - (centerLine - weapon.x) + 5, weapon.y + 5, 'Q', 6);
		gotoxyAndPutchar(MX / 2 - (centerLine - weapon.x) + 6, weapon.y + 5, 'Q', 6);
		gotoxyAndPutchar(MX / 2 - (centerLine - weapon.x) + 7, weapon.y + 5, 'Q', 6);
		gotoxyAndPutchar(MX / 2 - (centerLine - weapon.x) + 8, weapon.y + 5, 'Q', 6);
		gotoxyAndPutchar(MX / 2 - (centerLine - weapon.x) + 9, weapon.y + 5, 'Q', 6);
		gotoxyAndPutchar(MX / 2 - (centerLine - weapon.x) + 10, weapon.y + 5, 'Q', 6);
		gotoxyAndPutchar(MX / 2 - (centerLine - weapon.x) + 11, weapon.y + 5, 'Q', 14);
		gotoxyAndPutchar(MX / 2 - (centerLine - weapon.x) + 12, weapon.y + 5, 'Q', 14);
		gotoxyAndPutchar(MX / 2 - (centerLine - weapon.x) + 13, weapon.y + 5, 'Q', 14);
		gotoxyAndPutchar(MX / 2 - (centerLine - weapon.x) + 14, weapon.y + 5, 'Q', 14);

		gotoxyAndPutchar(MX / 2 - (centerLine - weapon.x) + 2, weapon.y + 6, 'Q', 14);
		gotoxyAndPutchar(MX / 2 - (centerLine - weapon.x) + 3, weapon.y + 6, 'Q', 14);
		gotoxyAndPutchar(MX / 2 - (centerLine - weapon.x) + 4, weapon.y + 6, 'Q', 14);
		gotoxyAndPutchar(MX / 2 - (centerLine - weapon.x) + 5, weapon.y + 6, 'Q', 14);
		gotoxyAndPutchar(MX / 2 - (centerLine - weapon.x) + 6, weapon.y + 6, 'Q', 14);
		gotoxyAndPutchar(MX / 2 - (centerLine - weapon.x) + 7, weapon.y + 6, 'Q', 14);
		gotoxyAndPutchar(MX / 2 - (centerLine - weapon.x) + 8, weapon.y + 6, 'Q', 14);
		gotoxyAndPutchar(MX / 2 - (centerLine - weapon.x) + 9, weapon.y + 6, 'Q', 14);
		gotoxyAndPutchar(MX / 2 - (centerLine - weapon.x) + 10, weapon.y + 6, 'Q', 14);
		gotoxyAndPutchar(MX / 2 - (centerLine - weapon.x) + 11, weapon.y + 6, 'Q', 14);
		gotoxyAndPutchar(MX / 2 - (centerLine - weapon.x) + 12, weapon.y + 6, 'Q', 14);
		gotoxyAndPutchar(MX / 2 - (centerLine - weapon.x) + 13, weapon.y + 6, 'Q', 14);

		gotoxyAndPutchar(MX / 2 - (centerLine - weapon.x) + 4, weapon.y + 7, 'Q', 14);
		gotoxyAndPutchar(MX / 2 - (centerLine - weapon.x) + 5, weapon.y + 7, 'Q', 14);
		gotoxyAndPutchar(MX / 2 - (centerLine - weapon.x) + 6, weapon.y + 7, 'Q', 14);
		gotoxyAndPutchar(MX / 2 - (centerLine - weapon.x) + 7, weapon.y + 7, 'Q', 14);
		gotoxyAndPutchar(MX / 2 - (centerLine - weapon.x) + 8, weapon.y + 7, 'Q', 14);
		gotoxyAndPutchar(MX / 2 - (centerLine - weapon.x) + 9, weapon.y + 7, 'Q', 14);
		gotoxyAndPutchar(MX / 2 - (centerLine - weapon.x) + 10, weapon.y + 7, 'Q', 14);
		gotoxyAndPutchar(MX / 2 - (centerLine - weapon.x) + 11, weapon.y + 7, 'Q', 14);

		gotoxyAndPutchar(MX / 2 - (centerLine - weapon.x) + 7, weapon.y + 8, 'Q', 2);
		gotoxyAndPutchar(MX / 2 - (centerLine - weapon.x) + 8, weapon.y + 8, 'Q', 2);

		gotoxyAndPutchar(MX / 2 - (centerLine - weapon.x) + 1, weapon.y + 9, 'Q', 10);
		gotoxyAndPutchar(MX / 2 - (centerLine - weapon.x) + 2, weapon.y + 9, 'Q', 10);
		gotoxyAndPutchar(MX / 2 - (centerLine - weapon.x) + 7, weapon.y + 9, 'Q', 2);
		gotoxyAndPutchar(MX / 2 - (centerLine - weapon.x) + 8, weapon.y + 9, 'Q', 2);
		gotoxyAndPutchar(MX / 2 - (centerLine - weapon.x) + 13, weapon.y + 9, 'Q', 10);
		gotoxyAndPutchar(MX / 2 - (centerLine - weapon.x) + 14, weapon.y + 9, 'Q', 10);

		gotoxyAndPutchar(MX / 2 - (centerLine - weapon.x) + 1, weapon.y + 10, 'Q', 10);
		gotoxyAndPutchar(MX / 2 - (centerLine - weapon.x) + 2, weapon.y + 10, 'Q', 2);
		gotoxyAndPutchar(MX / 2 - (centerLine - weapon.x) + 3, weapon.y + 10, 'Q', 10);
		gotoxyAndPutchar(MX / 2 - (centerLine - weapon.x) + 4, weapon.y + 10, 'Q', 10);
		gotoxyAndPutchar(MX / 2 - (centerLine - weapon.x) + 7, weapon.y + 10, 'Q', 2);
		gotoxyAndPutchar(MX / 2 - (centerLine - weapon.x) + 8, weapon.y + 10, 'Q', 2);
		gotoxyAndPutchar(MX / 2 - (centerLine - weapon.x) + 11, weapon.y + 10, 'Q', 10);
		gotoxyAndPutchar(MX / 2 - (centerLine - weapon.x) + 12, weapon.y + 10, 'Q', 10);
		gotoxyAndPutchar(MX / 2 - (centerLine - weapon.x) + 13, weapon.y + 10, 'Q', 2);
		gotoxyAndPutchar(MX / 2 - (centerLine - weapon.x) + 14, weapon.y + 10, 'Q', 10);

		gotoxyAndPutchar(MX / 2 - (centerLine - weapon.x) + 1, weapon.y + 11, 'Q', 2);
		gotoxyAndPutchar(MX / 2 - (centerLine - weapon.x) + 2, weapon.y + 11, 'Q', 10);
		gotoxyAndPutchar(MX / 2 - (centerLine - weapon.x) + 3, weapon.y + 11, 'Q', 2);
		gotoxyAndPutchar(MX / 2 - (centerLine - weapon.x) + 4, weapon.y + 11, 'Q', 10);
		gotoxyAndPutchar(MX / 2 - (centerLine - weapon.x) + 5, weapon.y + 11, 'Q', 10);
		gotoxyAndPutchar(MX / 2 - (centerLine - weapon.x) + 7, weapon.y + 11, 'Q', 2);
		gotoxyAndPutchar(MX / 2 - (centerLine - weapon.x) + 8, weapon.y + 11, 'Q', 2);
		gotoxyAndPutchar(MX / 2 - (centerLine - weapon.x) + 10, weapon.y + 11, 'Q', 10);
		gotoxyAndPutchar(MX / 2 - (centerLine - weapon.x) + 11, weapon.y + 11, 'Q', 10);
		gotoxyAndPutchar(MX / 2 - (centerLine - weapon.x) + 12, weapon.y + 11, 'Q', 2);
		gotoxyAndPutchar(MX / 2 - (centerLine - weapon.x) + 13, weapon.y + 11, 'Q', 10);
		gotoxyAndPutchar(MX / 2 - (centerLine - weapon.x) + 14, weapon.y + 11, 'Q', 2);

		gotoxyAndPutchar(MX / 2 - (centerLine - weapon.x) + 2, weapon.y + 12, 'Q', 2);
		gotoxyAndPutchar(MX / 2 - (centerLine - weapon.x) + 3, weapon.y + 12, 'Q', 2);
		gotoxyAndPutchar(MX / 2 - (centerLine - weapon.x) + 4, weapon.y + 12, 'Q', 2);
		gotoxyAndPutchar(MX / 2 - (centerLine - weapon.x) + 5, weapon.y + 12, 'Q', 10);
		gotoxyAndPutchar(MX / 2 - (centerLine - weapon.x) + 7, weapon.y + 12, 'Q', 2);
		gotoxyAndPutchar(MX / 2 - (centerLine - weapon.x) + 8, weapon.y + 12, 'Q', 2);
		gotoxyAndPutchar(MX / 2 - (centerLine - weapon.x) + 10, weapon.y + 12, 'Q', 10);
		gotoxyAndPutchar(MX / 2 - (centerLine - weapon.x) + 11, weapon.y + 12, 'Q', 2);
		gotoxyAndPutchar(MX / 2 - (centerLine - weapon.x) + 12, weapon.y + 12, 'Q', 2);
		gotoxyAndPutchar(MX / 2 - (centerLine - weapon.x) + 13, weapon.y + 12, 'Q', 2);

		gotoxyAndPutchar(MX / 2 - (centerLine - weapon.x) + 4, weapon.y + 13, 'Q', 2);
		gotoxyAndPutchar(MX / 2 - (centerLine - weapon.x) + 5, weapon.y + 13, 'Q', 2);
		gotoxyAndPutchar(MX / 2 - (centerLine - weapon.x) + 6, weapon.y + 13, 'Q', 2);
		gotoxyAndPutchar(MX / 2 - (centerLine - weapon.x) + 7, weapon.y + 13, 'Q', 2);
		gotoxyAndPutchar(MX / 2 - (centerLine - weapon.x) + 8, weapon.y + 13, 'Q', 2);
		gotoxyAndPutchar(MX / 2 - (centerLine - weapon.x) + 9, weapon.y + 13, 'Q', 2);
		gotoxyAndPutchar(MX / 2 - (centerLine - weapon.x) + 10, weapon.y + 13, 'Q', 2);
		gotoxyAndPutchar(MX / 2 - (centerLine - weapon.x) + 11, weapon.y + 13, 'Q', 2);
	}
}

//显示主角
void SuperMushRoom::showHero()
{
	if (GameState == 0)
	{
		if (hero.jump != 0)//跳跃
		{
			if (hero.turn != -1)//向右
			{
				/*
					 000
					00000000
				   12122212
				   12112221222
					 11112
				  111100011
				222 110200001122
				22  00000000  1
				 11000   000011
				 111
				*/
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 5, hero.y + 0, 'Q', 0x04);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 6, hero.y + 0, 'Q', 0x04);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 7, hero.y + 0, 'Q', 0x04);

				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 4, hero.y + 1, 'Q', 0x04);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 5, hero.y + 1, 'Q', 0x04);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 6, hero.y + 1, 'Q', 0x04);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 7, hero.y + 1, 'Q', 0x04);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 8, hero.y + 1, 'Q', 0x04);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 9, hero.y + 1, 'Q', 0x04);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 10, hero.y + 1, 'Q', 0x04);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 11, hero.y + 1, 'Q', 0x04);

				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 3, hero.y + 2, 'Q', 0x06);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 4, hero.y + 2, 'Q', 0x0e);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 5, hero.y + 2, 'Q', 0x06);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 6, hero.y + 2, 'Q', 0x0e);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 7, hero.y + 2, 'Q', 0x0e);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 8, hero.y + 2, 'Q', 0x0e);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 9, hero.y + 2, 'Q', 0x06);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 10, hero.y + 2, 'Q', 0x0e);

				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 3, hero.y + 3, 'Q', 0x06);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 4, hero.y + 3, 'Q', 0x0e);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 5, hero.y + 3, 'Q', 0x06);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 6, hero.y + 3, 'Q', 0x06);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 7, hero.y + 3, 'Q', 0x0e);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 8, hero.y + 3, 'Q', 0x0e);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 9, hero.y + 3, 'Q', 0x0e);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 10, hero.y + 3, 'Q', 0x06);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 11, hero.y + 3, 'Q', 0x0e);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 12, hero.y + 3, 'Q', 0x0e);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 13, hero.y + 3, 'Q', 0x0e);

				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 5, hero.y + 4, 'Q', 0x06);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 6, hero.y + 4, 'Q', 0x06);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 7, hero.y + 4, 'Q', 0x06);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 8, hero.y + 4, 'Q', 0x06);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 9, hero.y + 4, 'Q', 0x0e);

				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 2, hero.y + 5, 'Q', 0x06);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 3, hero.y + 5, 'Q', 0x06);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 4, hero.y + 5, 'Q', 0x06);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 5, hero.y + 5, 'Q', 0x06);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 6, hero.y + 5, 'Q', 0x04);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 7, hero.y + 5, 'Q', 0x04);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 8, hero.y + 5, 'Q', 0x04);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 9, hero.y + 5, 'Q', 0x06);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 10, hero.y + 5, 'Q', 0x06);

				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 0, hero.y + 6, 'Q', 0x0e);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 1, hero.y + 6, 'Q', 0x0e);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 2, hero.y + 6, 'Q', 0x0e);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 4, hero.y + 6, 'Q', 0x06);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 5, hero.y + 6, 'Q', 0x06);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 6, hero.y + 6, 'Q', 0x04);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 7, hero.y + 6, 'Q', 0x0e);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 8, hero.y + 6, 'Q', 0x04);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 9, hero.y + 6, 'Q', 0x04);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 10, hero.y + 6, 'Q', 0x04);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 11, hero.y + 6, 'Q', 0x04);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 12, hero.y + 6, 'Q', 0x06);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 13, hero.y + 6, 'Q', 0x06);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 14, hero.y + 6, 'Q', 0x0e);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 15, hero.y + 6, 'Q', 0x0e);

				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 0, hero.y + 7, 'Q', 0x0e);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 1, hero.y + 7, 'Q', 0x0e);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 4, hero.y + 7, 'Q', 0x04);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 5, hero.y + 7, 'Q', 0x04);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 6, hero.y + 7, 'Q', 0x04);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 7, hero.y + 7, 'Q', 0x04);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 8, hero.y + 7, 'Q', 0x04);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 9, hero.y + 7, 'Q', 0x04);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 10, hero.y + 7, 'Q', 0x04);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 11, hero.y + 7, 'Q', 0x04);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 14, hero.y + 7, 'Q', 0x06);

				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 1, hero.y + 8, 'Q', 0x06);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 2, hero.y + 8, 'Q', 0x06);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 3, hero.y + 8, 'Q', 0x04);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 4, hero.y + 8, 'Q', 0x04);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 5, hero.y + 8, 'Q', 0x04);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 9, hero.y + 8, 'Q', 0x04);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 10, hero.y + 8, 'Q', 0x04);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 11, hero.y + 8, 'Q', 0x04);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 12, hero.y + 8, 'Q', 0x04);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 13, hero.y + 8, 'Q', 0x06);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 14, hero.y + 8, 'Q', 0x06);

				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 1, hero.y + 9, 'Q', 0x06);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 2, hero.y + 9, 'Q', 0x06);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 3, hero.y + 9, 'Q', 0x06);
			}
			else//向左
			{
				/*
							000
						00000000
						 21222121
					  22212221121
						  21111
						 110001111
					221100002011 222
					 1  00000000  22
					 110000   00011
								111
				*/

				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 8, hero.y + 0, 'Q', 0x04);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 9, hero.y + 0, 'Q', 0x04);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 10, hero.y + 0, 'Q', 0x04);

				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 4, hero.y + 1, 'Q', 0x04);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 5, hero.y + 1, 'Q', 0x04);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 6, hero.y + 1, 'Q', 0x04);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 7, hero.y + 1, 'Q', 0x04);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 8, hero.y + 1, 'Q', 0x04);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 9, hero.y + 1, 'Q', 0x04);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 10, hero.y + 1, 'Q', 0x04);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 11, hero.y + 1, 'Q', 0x04);

				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 5, hero.y + 2, 'Q', 0x0e);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 6, hero.y + 2, 'Q', 0x06);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 7, hero.y + 2, 'Q', 0x0e);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 8, hero.y + 2, 'Q', 0x0e);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 9, hero.y + 2, 'Q', 0x0e);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 10, hero.y + 2, 'Q', 0x06);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 11, hero.y + 2, 'Q', 0x0e);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 12, hero.y + 2, 'Q', 0x06);

				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 2, hero.y + 3, 'Q', 0x0e);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 3, hero.y + 3, 'Q', 0x0e);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 4, hero.y + 3, 'Q', 0x0e);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 5, hero.y + 3, 'Q', 0x06);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 6, hero.y + 3, 'Q', 0x0e);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 7, hero.y + 3, 'Q', 0x0e);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 8, hero.y + 3, 'Q', 0x0e);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 9, hero.y + 3, 'Q', 0x06);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 10, hero.y + 3, 'Q', 0x06);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 11, hero.y + 3, 'Q', 0x0e);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 12, hero.y + 3, 'Q', 0x06);

				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 6, hero.y + 4, 'Q', 0x0e);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 7, hero.y + 4, 'Q', 0x06);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 8, hero.y + 4, 'Q', 0x06);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 9, hero.y + 4, 'Q', 0x06);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 10, hero.y + 4, 'Q', 0x06);

				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 5, hero.y + 5, 'Q', 0x06);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 6, hero.y + 5, 'Q', 0x06);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 7, hero.y + 5, 'Q', 0x04);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 8, hero.y + 5, 'Q', 0x04);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 9, hero.y + 5, 'Q', 0x04);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 10, hero.y + 5, 'Q', 0x06);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 11, hero.y + 5, 'Q', 0x06);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 12, hero.y + 5, 'Q', 0x06);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 13, hero.y + 5, 'Q', 0x06);

				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 0, hero.y + 6, 'Q', 0x0e);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 1, hero.y + 6, 'Q', 0x0e);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 2, hero.y + 6, 'Q', 0x06);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 3, hero.y + 6, 'Q', 0x06);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 4, hero.y + 6, 'Q', 0x04);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 5, hero.y + 6, 'Q', 0x04);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 6, hero.y + 6, 'Q', 0x04);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 7, hero.y + 6, 'Q', 0x04);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 8, hero.y + 6, 'Q', 0x0e);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 9, hero.y + 6, 'Q', 0x04);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 10, hero.y + 6, 'Q', 0x06);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 11, hero.y + 6, 'Q', 0x06);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 13, hero.y + 6, 'Q', 0x0e);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 14, hero.y + 6, 'Q', 0x0e);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 15, hero.y + 6, 'Q', 0x0e);

				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 1, hero.y + 7, 'Q', 0x06);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 4, hero.y + 7, 'Q', 0x04);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 5, hero.y + 7, 'Q', 0x04);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 6, hero.y + 7, 'Q', 0x04);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 7, hero.y + 7, 'Q', 0x04);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 8, hero.y + 7, 'Q', 0x04);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 9, hero.y + 7, 'Q', 0x04);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 10, hero.y + 7, 'Q', 0x04);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 11, hero.y + 7, 'Q', 0x04);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 14, hero.y + 7, 'Q', 0x0e);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 15, hero.y + 7, 'Q', 0x0e);

				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 1, hero.y + 8, 'Q', 0x06);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 2, hero.y + 8, 'Q', 0x06);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 3, hero.y + 8, 'Q', 0x04);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 4, hero.y + 8, 'Q', 0x04);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 5, hero.y + 8, 'Q', 0x04);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 6, hero.y + 8, 'Q', 0x04);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 10, hero.y + 8, 'Q', 0x04);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 11, hero.y + 8, 'Q', 0x04);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 12, hero.y + 8, 'Q', 0x04);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 13, hero.y + 8, 'Q', 0x06);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 14, hero.y + 8, 'Q', 0x06);

				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 12, hero.y + 9, 'Q', 0x06);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 13, hero.y + 9, 'Q', 0x06);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 14, hero.y + 9, 'Q', 0x06);
			}
		}
		else if (hero.turn == -1)//站立向左
		{
			hero.state++;
			int ch;
			if (hero.state % 5 != 0 || (ch = GetCommand()) == 0)//5帧中有一次变换，略显动感
			{
				/*
						   000
					  000000000
						22122212
					  12212221121
						 12222
					   111011011
					  11110000111
					  221020020122
						000  000
					  1111    1111
				*/

				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 7, hero.y + 0, 'Q', 0x04);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 8, hero.y + 0, 'Q', 0x04);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 9, hero.y + 0, 'Q', 0x04);

				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 2, hero.y + 1, 'Q', 0x04);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 3, hero.y + 1, 'Q', 0x04);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 4, hero.y + 1, 'Q', 0x04);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 5, hero.y + 1, 'Q', 0x04);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 6, hero.y + 1, 'Q', 0x04);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 7, hero.y + 1, 'Q', 0x04);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 8, hero.y + 1, 'Q', 0x04);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 9, hero.y + 1, 'Q', 0x04);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 10, hero.y + 1, 'Q', 0x04);

				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 4, hero.y + 2, 'Q', 0x0e);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 5, hero.y + 2, 'Q', 0x0e);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 6, hero.y + 2, 'Q', 0x06);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 7, hero.y + 2, 'Q', 0x0e);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 8, hero.y + 2, 'Q', 0x0e);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 9, hero.y + 2, 'Q', 0x0e);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 10, hero.y + 2, 'Q', 0x06);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 11, hero.y + 2, 'Q', 0x0e);

				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 2, hero.y + 3, 'Q', 0x06);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 3, hero.y + 3, 'Q', 0x0e);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 4, hero.y + 3, 'Q', 0x0e);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 5, hero.y + 3, 'Q', 0x06);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 6, hero.y + 3, 'Q', 0x0e);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 7, hero.y + 3, 'Q', 0x0e);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 8, hero.y + 3, 'Q', 0x0e);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 9, hero.y + 3, 'Q', 0x06);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 10, hero.y + 3, 'Q', 0x06);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 11, hero.y + 3, 'Q', 0x0e);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 12, hero.y + 3, 'Q', 0x06);

				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 5, hero.y + 4, 'Q', 0x06);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 6, hero.y + 4, 'Q', 0x0e);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 7, hero.y + 4, 'Q', 0x0e);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 8, hero.y + 4, 'Q', 0x0e);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 9, hero.y + 4, 'Q', 0x0e);

				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 3, hero.y + 5, 'Q', 0x06);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 4, hero.y + 5, 'Q', 0x06);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 5, hero.y + 5, 'Q', 0x06);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 6, hero.y + 5, 'Q', 0x04);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 7, hero.y + 5, 'Q', 0x06);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 8, hero.y + 5, 'Q', 0x06);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 9, hero.y + 5, 'Q', 0x04);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 10, hero.y + 5, 'Q', 0x06);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 11, hero.y + 5, 'Q', 0x06);

				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 2, hero.y + 6, 'Q', 0x06);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 3, hero.y + 6, 'Q', 0x06);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 4, hero.y + 6, 'Q', 0x06);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 5, hero.y + 6, 'Q', 0x06);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 6, hero.y + 6, 'Q', 0x04);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 7, hero.y + 6, 'Q', 0x04);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 8, hero.y + 6, 'Q', 0x04);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 9, hero.y + 6, 'Q', 0x04);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 10, hero.y + 6, 'Q', 0x06);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 11, hero.y + 6, 'Q', 0x06);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 12, hero.y + 6, 'Q', 0x06);

				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 2, hero.y + 7, 'Q', 0x0e);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 3, hero.y + 7, 'Q', 0x0e);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 4, hero.y + 7, 'Q', 0x06);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 5, hero.y + 7, 'Q', 0x04);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 6, hero.y + 7, 'Q', 0x0e);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 7, hero.y + 7, 'Q', 0x04);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 8, hero.y + 7, 'Q', 0x04);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 9, hero.y + 7, 'Q', 0x0e);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 10, hero.y + 7, 'Q', 0x04);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 11, hero.y + 7, 'Q', 0x06);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 12, hero.y + 7, 'Q', 0x0e);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 13, hero.y + 7, 'Q', 0x0e);

				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 4, hero.y + 8, 'Q', 0x04);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 5, hero.y + 8, 'Q', 0x04);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 6, hero.y + 8, 'Q', 0x04);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 9, hero.y + 8, 'Q', 0x04);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 10, hero.y + 8, 'Q', 0x04);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 11, hero.y + 8, 'Q', 0x04);

				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 2, hero.y + 9, 'Q', 0x06);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 3, hero.y + 9, 'Q', 0x06);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 4, hero.y + 9, 'Q', 0x06);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 5, hero.y + 9, 'Q', 0x06);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 10, hero.y + 9, 'Q', 0x06);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 11, hero.y + 9, 'Q', 0x06);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 12, hero.y + 9, 'Q', 0x06);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 13, hero.y + 9, 'Q', 0x06);
			}
			else
			{
				/*
						000
				   000000000
					 2122111
				   1221222121
					  11011
					11001111
				   000222110
					0002210
					111000
				   1111111
				*/

				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 8, hero.y + 0, 'Q', 0x04);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 9, hero.y + 0, 'Q', 0x04);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 10, hero.y + 0, 'Q', 0x04);

				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 3, hero.y + 1, 'Q', 0x04);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 4, hero.y + 1, 'Q', 0x04);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 5, hero.y + 1, 'Q', 0x04);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 6, hero.y + 1, 'Q', 0x04);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 7, hero.y + 1, 'Q', 0x04);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 8, hero.y + 1, 'Q', 0x04);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 9, hero.y + 1, 'Q', 0x04);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 10, hero.y + 1, 'Q', 0x04);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 11, hero.y + 1, 'Q', 0x04);

				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 5, hero.y + 2, 'Q', 0x0e);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 6, hero.y + 2, 'Q', 0x06);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 7, hero.y + 2, 'Q', 0x0e);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 8, hero.y + 2, 'Q', 0x0e);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 9, hero.y + 2, 'Q', 0x06);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 10, hero.y + 2, 'Q', 0x06);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 11, hero.y + 2, 'Q', 0x06);

				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 3, hero.y + 3, 'Q', 0x06);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 4, hero.y + 3, 'Q', 0x0e);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 5, hero.y + 3, 'Q', 0x0e);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 6, hero.y + 3, 'Q', 0x06);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 7, hero.y + 3, 'Q', 0x0e);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 8, hero.y + 3, 'Q', 0x0e);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 9, hero.y + 3, 'Q', 0x0e);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 10, hero.y + 3, 'Q', 0x06);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 11, hero.y + 3, 'Q', 0x0e);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 12, hero.y + 3, 'Q', 0x06);

				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 6, hero.y + 4, 'Q', 0x06);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 7, hero.y + 4, 'Q', 0x06);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 8, hero.y + 4, 'Q', 0x04);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 9, hero.y + 4, 'Q', 0x06);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 10, hero.y + 4, 'Q', 0x06);

				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 4, hero.y + 5, 'Q', 0x06);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 5, hero.y + 5, 'Q', 0x06);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 6, hero.y + 5, 'Q', 0x04);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 7, hero.y + 5, 'Q', 0x04);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 8, hero.y + 5, 'Q', 0x06);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 9, hero.y + 5, 'Q', 0x06);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 10, hero.y + 5, 'Q', 0x06);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 11, hero.y + 5, 'Q', 0x06);

				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 3, hero.y + 6, 'Q', 0x04);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 4, hero.y + 6, 'Q', 0x04);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 5, hero.y + 6, 'Q', 0x04);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 6, hero.y + 6, 'Q', 0x0e);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 7, hero.y + 6, 'Q', 0x0e);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 8, hero.y + 6, 'Q', 0x0e);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 9, hero.y + 6, 'Q', 0x06);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 10, hero.y + 6, 'Q', 0x06);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 11, hero.y + 6, 'Q', 0x04);

				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 4, hero.y + 7, 'Q', 0x04);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 5, hero.y + 7, 'Q', 0x04);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 6, hero.y + 7, 'Q', 0x04);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 7, hero.y + 7, 'Q', 0x0e);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 8, hero.y + 7, 'Q', 0x0e);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 9, hero.y + 7, 'Q', 0x06);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 10, hero.y + 7, 'Q', 0x04);

				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 4, hero.y + 8, 'Q', 0x06);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 5, hero.y + 8, 'Q', 0x06);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 6, hero.y + 8, 'Q', 0x06);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 7, hero.y + 8, 'Q', 0x04);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 8, hero.y + 8, 'Q', 0x04);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 9, hero.y + 8, 'Q', 0x04);

				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 3, hero.y + 9, 'Q', 0x06);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 4, hero.y + 9, 'Q', 0x06);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 5, hero.y + 9, 'Q', 0x06);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 6, hero.y + 9, 'Q', 0x06);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 7, hero.y + 9, 'Q', 0x06);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 8, hero.y + 9, 'Q', 0x06);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 9, hero.y + 9, 'Q', 0x06);
			}

		}
		else //站立向右
		{
			int ch;
			hero.state++;
			if (hero.turn == 0 || hero.state % 5 != 0 || (ch = GetCommand()) == 0)//5帧中有一次变换，略显动感
			{
				/*
					  000
					 000000000
					21222122
				   12112221221
					  22221
					110110111
				   11100001111
				  221020020122
					000  000
				  1111    1111
				*/
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 6, hero.y + 0, 'Q', 0x04);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 7, hero.y + 0, 'Q', 0x04);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 8, hero.y + 0, 'Q', 0x04);

				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 5, hero.y + 1, 'Q', 0x04);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 6, hero.y + 1, 'Q', 0x04);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 7, hero.y + 1, 'Q', 0x04);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 8, hero.y + 1, 'Q', 0x04);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 9, hero.y + 1, 'Q', 0x04);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 10, hero.y + 1, 'Q', 0x04);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 11, hero.y + 1, 'Q', 0x04);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 12, hero.y + 1, 'Q', 0x04);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 13, hero.y + 1, 'Q', 0x04);

				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 4, hero.y + 2, 'Q', 0x0e);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 5, hero.y + 2, 'Q', 0x06);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 6, hero.y + 2, 'Q', 0x0e);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 7, hero.y + 2, 'Q', 0x0e);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 8, hero.y + 2, 'Q', 0x0e);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 9, hero.y + 2, 'Q', 0x06);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 10, hero.y + 2, 'Q', 0x0e);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 11, hero.y + 2, 'Q', 0x0e);

				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 3, hero.y + 3, 'Q', 0x06);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 4, hero.y + 3, 'Q', 0x0e);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 5, hero.y + 3, 'Q', 0x06);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 6, hero.y + 3, 'Q', 0x06);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 7, hero.y + 3, 'Q', 0x0e);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 8, hero.y + 3, 'Q', 0x0e);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 9, hero.y + 3, 'Q', 0x0e);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 10, hero.y + 3, 'Q', 0x06);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 11, hero.y + 3, 'Q', 0x0e);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 12, hero.y + 3, 'Q', 0x0e);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 13, hero.y + 3, 'Q', 0x06);

				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 6, hero.y + 4, 'Q', 0x0e);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 7, hero.y + 4, 'Q', 0x0e);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 8, hero.y + 4, 'Q', 0x0e);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 9, hero.y + 4, 'Q', 0x0e);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 10, hero.y + 4, 'Q', 0x06);

				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 4, hero.y + 5, 'Q', 0x06);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 5, hero.y + 5, 'Q', 0x06);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 6, hero.y + 5, 'Q', 0x04);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 7, hero.y + 5, 'Q', 0x06);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 8, hero.y + 5, 'Q', 0x06);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 9, hero.y + 5, 'Q', 0x04);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 10, hero.y + 5, 'Q', 0x06);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 11, hero.y + 5, 'Q', 0x06);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 12, hero.y + 5, 'Q', 0x06);

				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 3, hero.y + 6, 'Q', 0x06);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 4, hero.y + 6, 'Q', 0x06);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 5, hero.y + 6, 'Q', 0x06);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 6, hero.y + 6, 'Q', 0x04);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 7, hero.y + 6, 'Q', 0x04);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 8, hero.y + 6, 'Q', 0x04);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 9, hero.y + 6, 'Q', 0x04);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 10, hero.y + 6, 'Q', 0x06);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 11, hero.y + 6, 'Q', 0x06);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 12, hero.y + 6, 'Q', 0x06);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 13, hero.y + 6, 'Q', 0x06);

				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 2, hero.y + 7, 'Q', 0x0e);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 3, hero.y + 7, 'Q', 0x0e);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 4, hero.y + 7, 'Q', 0x06);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 5, hero.y + 7, 'Q', 0x04);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 6, hero.y + 7, 'Q', 0x0e);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 7, hero.y + 7, 'Q', 0x04);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 8, hero.y + 7, 'Q', 0x04);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 9, hero.y + 7, 'Q', 0x0e);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 10, hero.y + 7, 'Q', 0x04);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 11, hero.y + 7, 'Q', 0x06);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 12, hero.y + 7, 'Q', 0x0e);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 13, hero.y + 7, 'Q', 0x0e);

				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 4, hero.y + 8, 'Q', 0x04);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 5, hero.y + 8, 'Q', 0x04);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 6, hero.y + 8, 'Q', 0x04);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 9, hero.y + 8, 'Q', 0x04);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 10, hero.y + 8, 'Q', 0x04);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 11, hero.y + 8, 'Q', 0x04);

				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 2, hero.y + 9, 'Q', 0x06);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 3, hero.y + 9, 'Q', 0x06);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 4, hero.y + 9, 'Q', 0x06);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 5, hero.y + 9, 'Q', 0x06);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 10, hero.y + 9, 'Q', 0x06);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 11, hero.y + 9, 'Q', 0x06);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 12, hero.y + 9, 'Q', 0x06);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 13, hero.y + 9, 'Q', 0x06);
			}
			else
			{
				/*
					 000
					000000000
					1112212
				   1212221221
					 11011
					11110011
					011222000
					 0122000
					  000111
					  1111111
					  1111
				*/

				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 5, hero.y + 0, 'Q', 0x04);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 6, hero.y + 0, 'Q', 0x04);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 7, hero.y + 0, 'Q', 0x04);

				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 4, hero.y + 1, 'Q', 0x04);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 5, hero.y + 1, 'Q', 0x04);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 6, hero.y + 1, 'Q', 0x04);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 7, hero.y + 1, 'Q', 0x04);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 8, hero.y + 1, 'Q', 0x04);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 9, hero.y + 1, 'Q', 0x04);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 10, hero.y + 1, 'Q', 0x04);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 11, hero.y + 1, 'Q', 0x04);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 12, hero.y + 1, 'Q', 0x04);

				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 4, hero.y + 2, 'Q', 0x06);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 5, hero.y + 2, 'Q', 0x06);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 6, hero.y + 2, 'Q', 0x06);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 7, hero.y + 2, 'Q', 0x0e);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 8, hero.y + 2, 'Q', 0x0e);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 9, hero.y + 2, 'Q', 0x06);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 10, hero.y + 2, 'Q', 0x0e);

				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 3, hero.y + 3, 'Q', 0x06);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 4, hero.y + 3, 'Q', 0x0e);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 5, hero.y + 3, 'Q', 0x06);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 6, hero.y + 3, 'Q', 0x0e);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 7, hero.y + 3, 'Q', 0x0e);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 8, hero.y + 3, 'Q', 0x0e);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 9, hero.y + 3, 'Q', 0x06);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 10, hero.y + 3, 'Q', 0x0e);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 11, hero.y + 3, 'Q', 0x0e);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 12, hero.y + 3, 'Q', 0x06);

				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 5, hero.y + 4, 'Q', 0x06);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 6, hero.y + 4, 'Q', 0x06);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 7, hero.y + 4, 'Q', 0x04);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 8, hero.y + 4, 'Q', 0x06);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 9, hero.y + 4, 'Q', 0x06);

				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 4, hero.y + 5, 'Q', 0x06);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 5, hero.y + 5, 'Q', 0x06);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 6, hero.y + 5, 'Q', 0x06);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 7, hero.y + 5, 'Q', 0x06);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 8, hero.y + 5, 'Q', 0x04);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 9, hero.y + 5, 'Q', 0x04);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 10, hero.y + 5, 'Q', 0x06);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 11, hero.y + 5, 'Q', 0x06);

				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 4, hero.y + 6, 'Q', 0x04);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 5, hero.y + 6, 'Q', 0x06);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 6, hero.y + 6, 'Q', 0x06);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 7, hero.y + 6, 'Q', 0x0e);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 8, hero.y + 6, 'Q', 0x0e);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 9, hero.y + 6, 'Q', 0x0e);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 10, hero.y + 6, 'Q', 0x04);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 11, hero.y + 6, 'Q', 0x04);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 12, hero.y + 6, 'Q', 0x04);

				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 5, hero.y + 7, 'Q', 0x04);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 6, hero.y + 7, 'Q', 0x06);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 7, hero.y + 7, 'Q', 0x0e);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 8, hero.y + 7, 'Q', 0x0e);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 9, hero.y + 7, 'Q', 0x04);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 10, hero.y + 7, 'Q', 0x04);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 11, hero.y + 7, 'Q', 0x04);

				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 6, hero.y + 8, 'Q', 0x04);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 7, hero.y + 8, 'Q', 0x04);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 8, hero.y + 8, 'Q', 0x04);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 9, hero.y + 8, 'Q', 0x06);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 10, hero.y + 8, 'Q', 0x06);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 11, hero.y + 8, 'Q', 0x06);

				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 6, hero.y + 9, 'Q', 0x06);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 7, hero.y + 9, 'Q', 0x06);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 8, hero.y + 9, 'Q', 0x06);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 9, hero.y + 9, 'Q', 0x06);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 10, hero.y + 9, 'Q', 0x06);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 11, hero.y + 9, 'Q', 0x06);
				gotoxyAndPutchar(MX / 2 - centerLine + hero.x + 12, hero.y + 9, 'Q', 0x06);

			}
		}
	}
	else if (GameState == 1)
	{
		if (hero.turn == -1)//向左
		{
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 5), hero.y + 0, 'Q', 0x04);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 6), hero.y + 0, 'Q', 0x04);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 7), hero.y + 0, 'Q', 0x04);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 8), hero.y + 0, 'Q', 0x04);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 9), hero.y + 0, 'Q', 0x04);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 10), hero.y + 0, 'Q', 0x04);

			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 4), hero.y + 1, 'Q', 0x04);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 5), hero.y + 1, 'Q', 0x04);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 6), hero.y + 1, 'Q', 0x04);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 7), hero.y + 1, 'Q', 0x04);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 8), hero.y + 1, 'Q', 0x04);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 9), hero.y + 1, 'Q', 0x04);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 10), hero.y + 1, 'Q', 0x04);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 11), hero.y + 1, 'Q', 0x04);

			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 2), hero.y + 2, 'Q', 0x04);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 3), hero.y + 2, 'Q', 0x06);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 4), hero.y + 2, 'Q', 0x06);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 5), hero.y + 2, 'Q', 0x04);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 6), hero.y + 2, 'Q', 0x04);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 7), hero.y + 2, 'Q', 0x04);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 8), hero.y + 2, 'Q', 0x04);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 9), hero.y + 2, 'Q', 0x04);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 10), hero.y + 2, 'Q', 0x04);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 11), hero.y + 2, 'Q', 0x06);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 12), hero.y + 2, 'Q', 0x06);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 13), hero.y + 2, 'Q', 0x04);

			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 1), hero.y + 3, 'Q', 0x04);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 2), hero.y + 3, 'Q', 0x04);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 3), hero.y + 3, 'Q', 0x04);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 4), hero.y + 3, 'Q', 0x0e);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 5), hero.y + 3, 'Q', 0x06);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 6), hero.y + 3, 'Q', 0x06);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 7), hero.y + 3, 'Q', 0x06);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 8), hero.y + 3, 'Q', 0x06);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 9), hero.y + 3, 'Q', 0x06);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 10), hero.y + 3, 'Q', 0x06);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 11), hero.y + 3, 'Q', 0x0e);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 12), hero.y + 3, 'Q', 0x04);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 13), hero.y + 3, 'Q', 0x04);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 14), hero.y + 3, 'Q', 0x04);

			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 0), hero.y + 4, 'Q', 0x04);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 1), hero.y + 4, 'Q', 0x04);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 2), hero.y + 4, 'Q', 0x04);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 3), hero.y + 4, 'Q', 0x04);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 4), hero.y + 4, 'Q', 0x0e);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 5), hero.y + 4, 'Q', 0x06);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 6), hero.y + 4, 'Q', 0x0e);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 7), hero.y + 4, 'Q', 0x04);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 8), hero.y + 4, 'Q', 0x04);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 9), hero.y + 4, 'Q', 0x0e);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 10), hero.y + 4, 'Q', 0x06);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 11), hero.y + 4, 'Q', 0x0e);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 12), hero.y + 4, 'Q', 0x04);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 13), hero.y + 4, 'Q', 0x04);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 14), hero.y + 4, 'Q', 0x04);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 15), hero.y + 4, 'Q', 0x04);

			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 0), hero.y + 5, 'Q', 0x04);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 1), hero.y + 5, 'Q', 0x04);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 2), hero.y + 5, 'Q', 0x04);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 3), hero.y + 5, 'Q', 0x04);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 4), hero.y + 5, 'Q', 0x0e);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 5), hero.y + 5, 'Q', 0x0e);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 6), hero.y + 5, 'Q', 0x0e);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 7), hero.y + 5, 'Q', 0x04);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 8), hero.y + 5, 'Q', 0x04);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 9), hero.y + 5, 'Q', 0x0e);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 10), hero.y + 5, 'Q', 0x0e);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 11), hero.y + 5, 'Q', 0x0e);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 12), hero.y + 5, 'Q', 0x04);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 13), hero.y + 5, 'Q', 0x04);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 14), hero.y + 5, 'Q', 0x04);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 15), hero.y + 5, 'Q', 0x04);

			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 0), hero.y + 6, 'Q', 0x04);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 1), hero.y + 6, 'Q', 0x04);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 2), hero.y + 6, 'Q', 0x04);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 3), hero.y + 6, 'Q', 0x04);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 4), hero.y + 6, 'Q', 0x04);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 5), hero.y + 6, 'Q', 0x04);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 6), hero.y + 6, 'Q', 0x04);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 7), hero.y + 6, 'Q', 0x04);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 8), hero.y + 6, 'Q', 0x04);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 9), hero.y + 6, 'Q', 0x04);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 10), hero.y + 6, 'Q', 0x04);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 11), hero.y + 6, 'Q', 0x04);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 12), hero.y + 6, 'Q', 0x04);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 13), hero.y + 6, 'Q', 0x04);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 14), hero.y + 6, 'Q', 0x04);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 15), hero.y + 6, 'Q', 0x04);

			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 1), hero.y + 7, 'Q', 0x04);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 2), hero.y + 7, 'Q', 0x04);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 3), hero.y + 7, 'Q', 0x04);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 4), hero.y + 7, 'Q', 0x04);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 5), hero.y + 7, 'Q', 0x0e);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 6), hero.y + 7, 'Q', 0x0e);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 7), hero.y + 7, 'Q', 0x0e);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 8), hero.y + 7, 'Q', 0x0e);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 9), hero.y + 7, 'Q', 0x0e);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 10), hero.y + 7, 'Q', 0x0e);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 11), hero.y + 7, 'Q', 0x04);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 12), hero.y + 7, 'Q', 0x04);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 13), hero.y + 7, 'Q', 0x04);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 14), hero.y + 7, 'Q', 0x04);

			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 4), hero.y + 8, 'Q', 0x0e);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 5), hero.y + 8, 'Q', 0x0e);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 6), hero.y + 8, 'Q', 0x0e);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 7), hero.y + 8, 'Q', 0x0e);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 8), hero.y + 8, 'Q', 0x0e);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 9), hero.y + 8, 'Q', 0x0e);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 10), hero.y + 8, 'Q', 0x0e);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 11), hero.y + 8, 'Q', 0x0e);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 12), hero.y + 8, 'Q', 0x06);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 13), hero.y + 8, 'Q', 0x06);

			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 3), hero.y + 9, 'Q', 0x06);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 4), hero.y + 9, 'Q', 0x06);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 5), hero.y + 9, 'Q', 0x06);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 9), hero.y + 9, 'Q', 0x06);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 10), hero.y + 9, 'Q', 0x06);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 11), hero.y + 9, 'Q', 0x06);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 12), hero.y + 9, 'Q', 0x06);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 13), hero.y + 9, 'Q', 0x06);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 14), hero.y + 9, 'Q', 0x06);
		}
		else
		{
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 5), hero.y + 0, 'Q', 0x04);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 6), hero.y + 0, 'Q', 0x04);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 7), hero.y + 0, 'Q', 0x04);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 8), hero.y + 0, 'Q', 0x04);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 9), hero.y + 0, 'Q', 0x04);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 10), hero.y + 0, 'Q', 0x04);

			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 4), hero.y + 1, 'Q', 0x04);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 5), hero.y + 1, 'Q', 0x04);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 6), hero.y + 1, 'Q', 0x04);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 7), hero.y + 1, 'Q', 0x04);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 8), hero.y + 1, 'Q', 0x04);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 9), hero.y + 1, 'Q', 0x04);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 10), hero.y + 1, 'Q', 0x04);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 11), hero.y + 1, 'Q', 0x04);

			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 2), hero.y + 2, 'Q', 0x04);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 3), hero.y + 2, 'Q', 0x06);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 4), hero.y + 2, 'Q', 0x06);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 5), hero.y + 2, 'Q', 0x04);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 6), hero.y + 2, 'Q', 0x04);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 7), hero.y + 2, 'Q', 0x04);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 8), hero.y + 2, 'Q', 0x04);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 9), hero.y + 2, 'Q', 0x04);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 10), hero.y + 2, 'Q', 0x04);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 11), hero.y + 2, 'Q', 0x06);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 12), hero.y + 2, 'Q', 0x06);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 13), hero.y + 2, 'Q', 0x04);

			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 1), hero.y + 3, 'Q', 0x04);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 2), hero.y + 3, 'Q', 0x04);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 3), hero.y + 3, 'Q', 0x04);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 4), hero.y + 3, 'Q', 0x0e);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 5), hero.y + 3, 'Q', 0x06);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 6), hero.y + 3, 'Q', 0x06);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 7), hero.y + 3, 'Q', 0x06);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 8), hero.y + 3, 'Q', 0x06);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 9), hero.y + 3, 'Q', 0x06);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 10), hero.y + 3, 'Q', 0x06);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 11), hero.y + 3, 'Q', 0x0e);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 12), hero.y + 3, 'Q', 0x04);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 13), hero.y + 3, 'Q', 0x04);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 14), hero.y + 3, 'Q', 0x04);

			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 0), hero.y + 4, 'Q', 0x04);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 1), hero.y + 4, 'Q', 0x04);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 2), hero.y + 4, 'Q', 0x04);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 3), hero.y + 4, 'Q', 0x04);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 4), hero.y + 4, 'Q', 0x0e);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 5), hero.y + 4, 'Q', 0x06);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 6), hero.y + 4, 'Q', 0x0e);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 7), hero.y + 4, 'Q', 0x04);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 8), hero.y + 4, 'Q', 0x04);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 9), hero.y + 4, 'Q', 0x0e);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 10), hero.y + 4, 'Q', 0x06);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 11), hero.y + 4, 'Q', 0x0e);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 12), hero.y + 4, 'Q', 0x04);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 13), hero.y + 4, 'Q', 0x04);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 14), hero.y + 4, 'Q', 0x04);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 15), hero.y + 4, 'Q', 0x04);

			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 0), hero.y + 5, 'Q', 0x04);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 1), hero.y + 5, 'Q', 0x04);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 2), hero.y + 5, 'Q', 0x04);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 3), hero.y + 5, 'Q', 0x04);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 4), hero.y + 5, 'Q', 0x0e);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 5), hero.y + 5, 'Q', 0x0e);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 6), hero.y + 5, 'Q', 0x0e);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 7), hero.y + 5, 'Q', 0x04);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 8), hero.y + 5, 'Q', 0x04);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 9), hero.y + 5, 'Q', 0x0e);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 10), hero.y + 5, 'Q', 0x0e);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 11), hero.y + 5, 'Q', 0x0e);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 12), hero.y + 5, 'Q', 0x04);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 13), hero.y + 5, 'Q', 0x04);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 14), hero.y + 5, 'Q', 0x04);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 15), hero.y + 5, 'Q', 0x04);

			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 0), hero.y + 6, 'Q', 0x04);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 1), hero.y + 6, 'Q', 0x04);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 2), hero.y + 6, 'Q', 0x04);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 3), hero.y + 6, 'Q', 0x04);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 4), hero.y + 6, 'Q', 0x04);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 5), hero.y + 6, 'Q', 0x04);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 6), hero.y + 6, 'Q', 0x04);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 7), hero.y + 6, 'Q', 0x04);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 8), hero.y + 6, 'Q', 0x04);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 9), hero.y + 6, 'Q', 0x04);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 10), hero.y + 6, 'Q', 0x04);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 11), hero.y + 6, 'Q', 0x04);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 12), hero.y + 6, 'Q', 0x04);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 13), hero.y + 6, 'Q', 0x04);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 14), hero.y + 6, 'Q', 0x04);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 15), hero.y + 6, 'Q', 0x04);

			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 1), hero.y + 7, 'Q', 0x04);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 2), hero.y + 7, 'Q', 0x04);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 3), hero.y + 7, 'Q', 0x04);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 4), hero.y + 7, 'Q', 0x04);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 5), hero.y + 7, 'Q', 0x0e);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 6), hero.y + 7, 'Q', 0x0e);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 7), hero.y + 7, 'Q', 0x0e);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 8), hero.y + 7, 'Q', 0x0e);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 9), hero.y + 7, 'Q', 0x0e);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 10), hero.y + 7, 'Q', 0x0e);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 11), hero.y + 7, 'Q', 0x04);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 12), hero.y + 7, 'Q', 0x04);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 13), hero.y + 7, 'Q', 0x04);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 14), hero.y + 7, 'Q', 0x04);

			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 2), hero.y + 8, 'Q', 0x06);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 3), hero.y + 8, 'Q', 0x06);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 4), hero.y + 8, 'Q', 0x0e);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 5), hero.y + 8, 'Q', 0x0e);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 6), hero.y + 8, 'Q', 0x0e);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 7), hero.y + 8, 'Q', 0x0e);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 8), hero.y + 8, 'Q', 0x0e);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 9), hero.y + 8, 'Q', 0x0e);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 10), hero.y + 8, 'Q', 0x0e);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 11), hero.y + 8, 'Q', 0x0e);

			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 1), hero.y + 9, 'Q', 0x06);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 2), hero.y + 9, 'Q', 0x06);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 3), hero.y + 9, 'Q', 0x06);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 4), hero.y + 9, 'Q', 0x06);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 5), hero.y + 9, 'Q', 0x06);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 6), hero.y + 9, 'Q', 0x06);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 10), hero.y + 9, 'Q', 0x06);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 11), hero.y + 9, 'Q', 0x06);
			gotoxyAndPutchar(MX / 2 - (centerLine - hero.x - 12), hero.y + 9, 'Q', 0x06);
		}


	}
}

//显示敌人
void SuperMushRoom::showEnemy() //显示敌人
{
	if (GameState == 0)
	{
		for (int i = 0; i < N_EMERY; i++)
		{
			if (enemy[i].id == 0)
				continue;
			else if (enemy[i].id < 0)
			{
				enemy[i].id++;

				if (enemy[i].turn == 1)
				{
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 2), enemy[i].y + 8, 'Q', 0x06);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 3), enemy[i].y + 8, 'Q', 0x06);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 4), enemy[i].y + 8, 'Q', 0x0e);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 5), enemy[i].y + 8, 'Q', 0x0e);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 6), enemy[i].y + 8, 'Q', 0x0e);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 7), enemy[i].y + 8, 'Q', 0x0e);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 8), enemy[i].y + 8, 'Q', 0x0e);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 9), enemy[i].y + 8, 'Q', 0x0e);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 10), enemy[i].y + 8, 'Q', 0x0e);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 11), enemy[i].y + 8, 'Q', 0x0e);

					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 1), enemy[i].y + 9, 'Q', 0x06);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 2), enemy[i].y + 9, 'Q', 0x06);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 3), enemy[i].y + 9, 'Q', 0x06);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 4), enemy[i].y + 9, 'Q', 0x06);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 5), enemy[i].y + 9, 'Q', 0x06);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 6), enemy[i].y + 9, 'Q', 0x06);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 10), enemy[i].y + 9, 'Q', 0x06);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 11), enemy[i].y + 9, 'Q', 0x06);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 12), enemy[i].y + 9, 'Q', 0x06);
				}
				else
				{
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 4), enemy[i].y + 8, 'Q', 0x0e);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 5), enemy[i].y + 8, 'Q', 0x0e);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 6), enemy[i].y + 8, 'Q', 0x0e);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 7), enemy[i].y + 8, 'Q', 0x0e);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 8), enemy[i].y + 8, 'Q', 0x0e);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 9), enemy[i].y + 8, 'Q', 0x0e);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 10), enemy[i].y + 8, 'Q', 0x0e);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 11), enemy[i].y + 8, 'Q', 0x0e);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 12), enemy[i].y + 8, 'Q', 0x06);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 13), enemy[i].y + 8, 'Q', 0x06);

					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 3), enemy[i].y + 9, 'Q', 0x06);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 4), enemy[i].y + 9, 'Q', 0x06);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 5), enemy[i].y + 9, 'Q', 0x06);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 9), enemy[i].y + 9, 'Q', 0x06);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 10), enemy[i].y + 9, 'Q', 0x06);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 11), enemy[i].y + 9, 'Q', 0x06);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 12), enemy[i].y + 9, 'Q', 0x06);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 13), enemy[i].y + 9, 'Q', 0x06);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 14), enemy[i].y + 9, 'Q', 0x06);
				}
			}
			else if (enemy[i].x >= centerLine - MX / 2 && enemy[i].x <= centerLine + MX / 2)
			{
				if (enemy[i].turn == 1)
				{
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 5), enemy[i].y + 0, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 6), enemy[i].y + 0, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 7), enemy[i].y + 0, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 8), enemy[i].y + 0, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 9), enemy[i].y + 0, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 10), enemy[i].y + 0, 'Q', 0x04);

					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 4), enemy[i].y + 1, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 5), enemy[i].y + 1, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 6), enemy[i].y + 1, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 7), enemy[i].y + 1, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 8), enemy[i].y + 1, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 9), enemy[i].y + 1, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 10), enemy[i].y + 1, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 11), enemy[i].y + 1, 'Q', 0x04);

					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 2), enemy[i].y + 2, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 3), enemy[i].y + 2, 'Q', 0x06);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 4), enemy[i].y + 2, 'Q', 0x06);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 5), enemy[i].y + 2, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 6), enemy[i].y + 2, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 7), enemy[i].y + 2, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 8), enemy[i].y + 2, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 9), enemy[i].y + 2, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 10), enemy[i].y + 2, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 11), enemy[i].y + 2, 'Q', 0x06);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 12), enemy[i].y + 2, 'Q', 0x06);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 13), enemy[i].y + 2, 'Q', 0x04);

					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 1), enemy[i].y + 3, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 2), enemy[i].y + 3, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 3), enemy[i].y + 3, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 4), enemy[i].y + 3, 'Q', 0x0e);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 5), enemy[i].y + 3, 'Q', 0x06);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 6), enemy[i].y + 3, 'Q', 0x06);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 7), enemy[i].y + 3, 'Q', 0x06);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 8), enemy[i].y + 3, 'Q', 0x06);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 9), enemy[i].y + 3, 'Q', 0x06);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 10), enemy[i].y + 3, 'Q', 0x06);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 11), enemy[i].y + 3, 'Q', 0x0e);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 12), enemy[i].y + 3, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 13), enemy[i].y + 3, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 14), enemy[i].y + 3, 'Q', 0x04);

					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 0), enemy[i].y + 4, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 1), enemy[i].y + 4, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 2), enemy[i].y + 4, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 3), enemy[i].y + 4, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 4), enemy[i].y + 4, 'Q', 0x0e);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 5), enemy[i].y + 4, 'Q', 0x06);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 6), enemy[i].y + 4, 'Q', 0x0e);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 7), enemy[i].y + 4, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 8), enemy[i].y + 4, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 9), enemy[i].y + 4, 'Q', 0x0e);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 10), enemy[i].y + 4, 'Q', 0x06);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 11), enemy[i].y + 4, 'Q', 0x0e);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 12), enemy[i].y + 4, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 13), enemy[i].y + 4, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 14), enemy[i].y + 4, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 15), enemy[i].y + 4, 'Q', 0x04);

					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 0), enemy[i].y + 5, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 1), enemy[i].y + 5, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 2), enemy[i].y + 5, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 3), enemy[i].y + 5, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 4), enemy[i].y + 5, 'Q', 0x0e);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 5), enemy[i].y + 5, 'Q', 0x0e);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 6), enemy[i].y + 5, 'Q', 0x0e);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 7), enemy[i].y + 5, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 8), enemy[i].y + 5, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 9), enemy[i].y + 5, 'Q', 0x0e);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 10), enemy[i].y + 5, 'Q', 0x0e);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 11), enemy[i].y + 5, 'Q', 0x0e);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 12), enemy[i].y + 5, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 13), enemy[i].y + 5, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 14), enemy[i].y + 5, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 15), enemy[i].y + 5, 'Q', 0x04);

					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 0), enemy[i].y + 6, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 1), enemy[i].y + 6, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 2), enemy[i].y + 6, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 3), enemy[i].y + 6, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 4), enemy[i].y + 6, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 5), enemy[i].y + 6, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 6), enemy[i].y + 6, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 7), enemy[i].y + 6, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 8), enemy[i].y + 6, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 9), enemy[i].y + 6, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 10), enemy[i].y + 6, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 11), enemy[i].y + 6, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 12), enemy[i].y + 6, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 13), enemy[i].y + 6, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 14), enemy[i].y + 6, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 15), enemy[i].y + 6, 'Q', 0x04);

					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 1), enemy[i].y + 7, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 2), enemy[i].y + 7, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 3), enemy[i].y + 7, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 4), enemy[i].y + 7, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 5), enemy[i].y + 7, 'Q', 0x0e);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 6), enemy[i].y + 7, 'Q', 0x0e);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 7), enemy[i].y + 7, 'Q', 0x0e);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 8), enemy[i].y + 7, 'Q', 0x0e);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 9), enemy[i].y + 7, 'Q', 0x0e);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 10), enemy[i].y + 7, 'Q', 0x0e);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 11), enemy[i].y + 7, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 12), enemy[i].y + 7, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 13), enemy[i].y + 7, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 14), enemy[i].y + 7, 'Q', 0x04);

					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 2), enemy[i].y + 8, 'Q', 0x06);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 3), enemy[i].y + 8, 'Q', 0x06);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 4), enemy[i].y + 8, 'Q', 0x0e);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 5), enemy[i].y + 8, 'Q', 0x0e);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 6), enemy[i].y + 8, 'Q', 0x0e);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 7), enemy[i].y + 8, 'Q', 0x0e);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 8), enemy[i].y + 8, 'Q', 0x0e);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 9), enemy[i].y + 8, 'Q', 0x0e);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 10), enemy[i].y + 8, 'Q', 0x0e);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 11), enemy[i].y + 8, 'Q', 0x0e);

					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 1), enemy[i].y + 9, 'Q', 0x06);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 2), enemy[i].y + 9, 'Q', 0x06);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 3), enemy[i].y + 9, 'Q', 0x06);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 4), enemy[i].y + 9, 'Q', 0x06);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 5), enemy[i].y + 9, 'Q', 0x06);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 6), enemy[i].y + 9, 'Q', 0x06);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 10), enemy[i].y + 9, 'Q', 0x06);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 11), enemy[i].y + 9, 'Q', 0x06);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 12), enemy[i].y + 9, 'Q', 0x06);
				}
				else
				{

					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 5), enemy[i].y + 0, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 6), enemy[i].y + 0, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 7), enemy[i].y + 0, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 8), enemy[i].y + 0, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 9), enemy[i].y + 0, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 10), enemy[i].y + 0, 'Q', 0x04);

					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 4), enemy[i].y + 1, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 5), enemy[i].y + 1, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 6), enemy[i].y + 1, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 7), enemy[i].y + 1, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 8), enemy[i].y + 1, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 9), enemy[i].y + 1, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 10), enemy[i].y + 1, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 11), enemy[i].y + 1, 'Q', 0x04);

					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 2), enemy[i].y + 2, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 3), enemy[i].y + 2, 'Q', 0x06);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 4), enemy[i].y + 2, 'Q', 0x06);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 5), enemy[i].y + 2, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 6), enemy[i].y + 2, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 7), enemy[i].y + 2, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 8), enemy[i].y + 2, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 9), enemy[i].y + 2, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 10), enemy[i].y + 2, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 11), enemy[i].y + 2, 'Q', 0x06);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 12), enemy[i].y + 2, 'Q', 0x06);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 13), enemy[i].y + 2, 'Q', 0x04);

					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 1), enemy[i].y + 3, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 2), enemy[i].y + 3, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 3), enemy[i].y + 3, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 4), enemy[i].y + 3, 'Q', 0x0e);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 5), enemy[i].y + 3, 'Q', 0x06);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 6), enemy[i].y + 3, 'Q', 0x06);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 7), enemy[i].y + 3, 'Q', 0x06);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 8), enemy[i].y + 3, 'Q', 0x06);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 9), enemy[i].y + 3, 'Q', 0x06);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 10), enemy[i].y + 3, 'Q', 0x06);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 11), enemy[i].y + 3, 'Q', 0x0e);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 12), enemy[i].y + 3, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 13), enemy[i].y + 3, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 14), enemy[i].y + 3, 'Q', 0x04);

					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 0), enemy[i].y + 4, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 1), enemy[i].y + 4, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 2), enemy[i].y + 4, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 3), enemy[i].y + 4, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 4), enemy[i].y + 4, 'Q', 0x0e);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 5), enemy[i].y + 4, 'Q', 0x06);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 6), enemy[i].y + 4, 'Q', 0x0e);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 7), enemy[i].y + 4, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 8), enemy[i].y + 4, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 9), enemy[i].y + 4, 'Q', 0x0e);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 10), enemy[i].y + 4, 'Q', 0x06);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 11), enemy[i].y + 4, 'Q', 0x0e);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 12), enemy[i].y + 4, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 13), enemy[i].y + 4, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 14), enemy[i].y + 4, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 15), enemy[i].y + 4, 'Q', 0x04);

					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 0), enemy[i].y + 5, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 1), enemy[i].y + 5, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 2), enemy[i].y + 5, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 3), enemy[i].y + 5, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 4), enemy[i].y + 5, 'Q', 0x0e);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 5), enemy[i].y + 5, 'Q', 0x0e);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 6), enemy[i].y + 5, 'Q', 0x0e);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 7), enemy[i].y + 5, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 8), enemy[i].y + 5, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 9), enemy[i].y + 5, 'Q', 0x0e);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 10), enemy[i].y + 5, 'Q', 0x0e);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 11), enemy[i].y + 5, 'Q', 0x0e);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 12), enemy[i].y + 5, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 13), enemy[i].y + 5, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 14), enemy[i].y + 5, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 15), enemy[i].y + 5, 'Q', 0x04);

					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 0), enemy[i].y + 6, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 1), enemy[i].y + 6, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 2), enemy[i].y + 6, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 3), enemy[i].y + 6, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 4), enemy[i].y + 6, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 5), enemy[i].y + 6, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 6), enemy[i].y + 6, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 7), enemy[i].y + 6, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 8), enemy[i].y + 6, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 9), enemy[i].y + 6, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 10), enemy[i].y + 6, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 11), enemy[i].y + 6, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 12), enemy[i].y + 6, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 13), enemy[i].y + 6, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 14), enemy[i].y + 6, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 15), enemy[i].y + 6, 'Q', 0x04);

					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 1), enemy[i].y + 7, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 2), enemy[i].y + 7, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 3), enemy[i].y + 7, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 4), enemy[i].y + 7, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 5), enemy[i].y + 7, 'Q', 0x0e);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 6), enemy[i].y + 7, 'Q', 0x0e);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 7), enemy[i].y + 7, 'Q', 0x0e);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 8), enemy[i].y + 7, 'Q', 0x0e);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 9), enemy[i].y + 7, 'Q', 0x0e);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 10), enemy[i].y + 7, 'Q', 0x0e);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 11), enemy[i].y + 7, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 12), enemy[i].y + 7, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 13), enemy[i].y + 7, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 14), enemy[i].y + 7, 'Q', 0x04);

					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 4), enemy[i].y + 8, 'Q', 0x0e);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 5), enemy[i].y + 8, 'Q', 0x0e);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 6), enemy[i].y + 8, 'Q', 0x0e);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 7), enemy[i].y + 8, 'Q', 0x0e);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 8), enemy[i].y + 8, 'Q', 0x0e);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 9), enemy[i].y + 8, 'Q', 0x0e);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 10), enemy[i].y + 8, 'Q', 0x0e);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 11), enemy[i].y + 8, 'Q', 0x0e);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 12), enemy[i].y + 8, 'Q', 0x06);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 13), enemy[i].y + 8, 'Q', 0x06);

					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 3), enemy[i].y + 9, 'Q', 0x06);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 4), enemy[i].y + 9, 'Q', 0x06);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 5), enemy[i].y + 9, 'Q', 0x06);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 9), enemy[i].y + 9, 'Q', 0x06);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 10), enemy[i].y + 9, 'Q', 0x06);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 11), enemy[i].y + 9, 'Q', 0x06);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 12), enemy[i].y + 9, 'Q', 0x06);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 13), enemy[i].y + 9, 'Q', 0x06);
					gotoxyAndPutchar(MX / 2 - (centerLine - enemy[i].x - 14), enemy[i].y + 9, 'Q', 0x06);
				}
			}
		}
	}
	else if (GameState == 1)
	{
		for (int i = 0; i < N_EMERY; i++)
		{
			if (enemy[i].id == 0)
				continue;
			else if (enemy[i].id < 0)
			{
				enemy[i].id++;

				if (enemy[i].turn == 1)
				{
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 2, enemy[i].y + 7, 'Q', 0x0e);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 3, enemy[i].y + 7, 'Q', 0x0e);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 4, enemy[i].y + 7, 'Q', 0x06);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 5, enemy[i].y + 7, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 6, enemy[i].y + 7, 'Q', 0x0e);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 7, enemy[i].y + 7, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 8, enemy[i].y + 7, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 9, enemy[i].y + 7, 'Q', 0x0e);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 10, enemy[i].y + 7, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 11, enemy[i].y + 7, 'Q', 0x06);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 12, enemy[i].y + 7, 'Q', 0x0e);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 13, enemy[i].y + 7, 'Q', 0x0e);

					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 4, enemy[i].y + 8, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 5, enemy[i].y + 8, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 6, enemy[i].y + 8, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 9, enemy[i].y + 8, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 10, enemy[i].y + 8, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 11, enemy[i].y + 8, 'Q', 0x04);

					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 2, enemy[i].y + 9, 'Q', 0x06);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 3, enemy[i].y + 9, 'Q', 0x06);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 4, enemy[i].y + 9, 'Q', 0x06);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 5, enemy[i].y + 9, 'Q', 0x06);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 10, enemy[i].y + 9, 'Q', 0x06);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 11, enemy[i].y + 9, 'Q', 0x06);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 12, enemy[i].y + 9, 'Q', 0x06);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 13, enemy[i].y + 9, 'Q', 0x06);
				}
				else
				{

					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 2, enemy[i].y + 7, 'Q', 0x0e);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 3, enemy[i].y + 7, 'Q', 0x0e);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 4, enemy[i].y + 7, 'Q', 0x06);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 5, enemy[i].y + 7, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 6, enemy[i].y + 7, 'Q', 0x0e);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 7, enemy[i].y + 7, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 8, enemy[i].y + 7, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 9, enemy[i].y + 7, 'Q', 0x0e);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 10, enemy[i].y + 7, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 11, enemy[i].y + 7, 'Q', 0x06);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 12, enemy[i].y + 7, 'Q', 0x0e);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 13, enemy[i].y + 7, 'Q', 0x0e);

					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 4, enemy[i].y + 8, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 5, enemy[i].y + 8, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 6, enemy[i].y + 8, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 9, enemy[i].y + 8, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 10, enemy[i].y + 8, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 11, enemy[i].y + 8, 'Q', 0x04);

					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 2, enemy[i].y + 9, 'Q', 0x06);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 3, enemy[i].y + 9, 'Q', 0x06);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 4, enemy[i].y + 9, 'Q', 0x06);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 5, enemy[i].y + 9, 'Q', 0x06);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 10, enemy[i].y + 9, 'Q', 0x06);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 11, enemy[i].y + 9, 'Q', 0x06);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 12, enemy[i].y + 9, 'Q', 0x06);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 13, enemy[i].y + 9, 'Q', 0x06);
				}
			}
			else if (enemy[i].x >= centerLine - MX / 2 && enemy[i].x <= centerLine + MX / 2)
			{
				if (enemy[i].turn == 1)
				{
					/*
						  000
						 000000000
						21222122
					   12112221221
						  22221
						110110111
					   11100001111
					  221020020122
						000  000
					  1111    1111
					*/
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 6, enemy[i].y + 0, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 7, enemy[i].y + 0, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 8, enemy[i].y + 0, 'Q', 0x04);

					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 5, enemy[i].y + 1, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 6, enemy[i].y + 1, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 7, enemy[i].y + 1, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 8, enemy[i].y + 1, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 9, enemy[i].y + 1, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 10, enemy[i].y + 1, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 11, enemy[i].y + 1, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 12, enemy[i].y + 1, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 13, enemy[i].y + 1, 'Q', 0x04);

					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 4, enemy[i].y + 2, 'Q', 0x0e);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 5, enemy[i].y + 2, 'Q', 0x06);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 6, enemy[i].y + 2, 'Q', 0x0e);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 7, enemy[i].y + 2, 'Q', 0x0e);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 8, enemy[i].y + 2, 'Q', 0x0e);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 9, enemy[i].y + 2, 'Q', 0x06);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 10, enemy[i].y + 2, 'Q', 0x0e);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 11, enemy[i].y + 2, 'Q', 0x0e);

					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 3, enemy[i].y + 3, 'Q', 0x06);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 4, enemy[i].y + 3, 'Q', 0x0e);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 5, enemy[i].y + 3, 'Q', 0x06);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 6, enemy[i].y + 3, 'Q', 0x06);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 7, enemy[i].y + 3, 'Q', 0x0e);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 8, enemy[i].y + 3, 'Q', 0x0e);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 9, enemy[i].y + 3, 'Q', 0x0e);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 10, enemy[i].y + 3, 'Q', 0x06);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 11, enemy[i].y + 3, 'Q', 0x0e);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 12, enemy[i].y + 3, 'Q', 0x0e);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 13, enemy[i].y + 3, 'Q', 0x06);

					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 6, enemy[i].y + 4, 'Q', 0x0e);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 7, enemy[i].y + 4, 'Q', 0x0e);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 8, enemy[i].y + 4, 'Q', 0x0e);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 9, enemy[i].y + 4, 'Q', 0x0e);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 10, enemy[i].y + 4, 'Q', 0x06);

					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 4, enemy[i].y + 5, 'Q', 0x06);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 5, enemy[i].y + 5, 'Q', 0x06);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 6, enemy[i].y + 5, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 7, enemy[i].y + 5, 'Q', 0x06);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 8, enemy[i].y + 5, 'Q', 0x06);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 9, enemy[i].y + 5, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 10, enemy[i].y + 5, 'Q', 0x06);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 11, enemy[i].y + 5, 'Q', 0x06);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 12, enemy[i].y + 5, 'Q', 0x06);

					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 3, enemy[i].y + 6, 'Q', 0x06);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 4, enemy[i].y + 6, 'Q', 0x06);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 5, enemy[i].y + 6, 'Q', 0x06);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 6, enemy[i].y + 6, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 7, enemy[i].y + 6, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 8, enemy[i].y + 6, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 9, enemy[i].y + 6, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 10, enemy[i].y + 6, 'Q', 0x06);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 11, enemy[i].y + 6, 'Q', 0x06);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 12, enemy[i].y + 6, 'Q', 0x06);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 13, enemy[i].y + 6, 'Q', 0x06);

					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 2, enemy[i].y + 7, 'Q', 0x0e);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 3, enemy[i].y + 7, 'Q', 0x0e);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 4, enemy[i].y + 7, 'Q', 0x06);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 5, enemy[i].y + 7, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 6, enemy[i].y + 7, 'Q', 0x0e);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 7, enemy[i].y + 7, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 8, enemy[i].y + 7, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 9, enemy[i].y + 7, 'Q', 0x0e);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 10, enemy[i].y + 7, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 11, enemy[i].y + 7, 'Q', 0x06);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 12, enemy[i].y + 7, 'Q', 0x0e);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 13, enemy[i].y + 7, 'Q', 0x0e);

					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 4, enemy[i].y + 8, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 5, enemy[i].y + 8, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 6, enemy[i].y + 8, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 9, enemy[i].y + 8, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 10, enemy[i].y + 8, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 11, enemy[i].y + 8, 'Q', 0x04);

					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 2, enemy[i].y + 9, 'Q', 0x06);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 3, enemy[i].y + 9, 'Q', 0x06);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 4, enemy[i].y + 9, 'Q', 0x06);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 5, enemy[i].y + 9, 'Q', 0x06);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 10, enemy[i].y + 9, 'Q', 0x06);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 11, enemy[i].y + 9, 'Q', 0x06);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 12, enemy[i].y + 9, 'Q', 0x06);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 13, enemy[i].y + 9, 'Q', 0x06);
				}
				else
				{

					/*
							   000
						  000000000
							22122212
						  12212221121
							 12222
						   111011011
						  11110000111
						  221020020122
							000  000
						  1111    1111
					*/

					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 7, enemy[i].y + 0, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 8, enemy[i].y + 0, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 9, enemy[i].y + 0, 'Q', 0x04);

					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 2, enemy[i].y + 1, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 3, enemy[i].y + 1, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 4, enemy[i].y + 1, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 5, enemy[i].y + 1, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 6, enemy[i].y + 1, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 7, enemy[i].y + 1, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 8, enemy[i].y + 1, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 9, enemy[i].y + 1, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 10, enemy[i].y + 1, 'Q', 0x04);

					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 4, enemy[i].y + 2, 'Q', 0x0e);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 5, enemy[i].y + 2, 'Q', 0x0e);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 6, enemy[i].y + 2, 'Q', 0x06);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 7, enemy[i].y + 2, 'Q', 0x0e);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 8, enemy[i].y + 2, 'Q', 0x0e);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 9, enemy[i].y + 2, 'Q', 0x0e);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 10, enemy[i].y + 2, 'Q', 0x06);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 11, enemy[i].y + 2, 'Q', 0x0e);

					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 2, enemy[i].y + 3, 'Q', 0x06);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 3, enemy[i].y + 3, 'Q', 0x0e);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 4, enemy[i].y + 3, 'Q', 0x0e);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 5, enemy[i].y + 3, 'Q', 0x06);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 6, enemy[i].y + 3, 'Q', 0x0e);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 7, enemy[i].y + 3, 'Q', 0x0e);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 8, enemy[i].y + 3, 'Q', 0x0e);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 9, enemy[i].y + 3, 'Q', 0x06);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 10, enemy[i].y + 3, 'Q', 0x06);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 11, enemy[i].y + 3, 'Q', 0x0e);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 12, enemy[i].y + 3, 'Q', 0x06);

					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 5, enemy[i].y + 4, 'Q', 0x06);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 6, enemy[i].y + 4, 'Q', 0x0e);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 7, enemy[i].y + 4, 'Q', 0x0e);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 8, enemy[i].y + 4, 'Q', 0x0e);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 9, enemy[i].y + 4, 'Q', 0x0e);

					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 3, enemy[i].y + 5, 'Q', 0x06);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 4, enemy[i].y + 5, 'Q', 0x06);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 5, enemy[i].y + 5, 'Q', 0x06);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 6, enemy[i].y + 5, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 7, enemy[i].y + 5, 'Q', 0x06);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 8, enemy[i].y + 5, 'Q', 0x06);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 9, enemy[i].y + 5, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 10, enemy[i].y + 5, 'Q', 0x06);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 11, enemy[i].y + 5, 'Q', 0x06);

					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 2, enemy[i].y + 6, 'Q', 0x06);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 3, enemy[i].y + 6, 'Q', 0x06);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 4, enemy[i].y + 6, 'Q', 0x06);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 5, enemy[i].y + 6, 'Q', 0x06);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 6, enemy[i].y + 6, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 7, enemy[i].y + 6, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 8, enemy[i].y + 6, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 9, enemy[i].y + 6, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 10, enemy[i].y + 6, 'Q', 0x06);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 11, enemy[i].y + 6, 'Q', 0x06);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 12, enemy[i].y + 6, 'Q', 0x06);

					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 2, enemy[i].y + 7, 'Q', 0x0e);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 3, enemy[i].y + 7, 'Q', 0x0e);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 4, enemy[i].y + 7, 'Q', 0x06);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 5, enemy[i].y + 7, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 6, enemy[i].y + 7, 'Q', 0x0e);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 7, enemy[i].y + 7, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 8, enemy[i].y + 7, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 9, enemy[i].y + 7, 'Q', 0x0e);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 10, enemy[i].y + 7, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 11, enemy[i].y + 7, 'Q', 0x06);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 12, enemy[i].y + 7, 'Q', 0x0e);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 13, enemy[i].y + 7, 'Q', 0x0e);

					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 4, enemy[i].y + 8, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 5, enemy[i].y + 8, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 6, enemy[i].y + 8, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 9, enemy[i].y + 8, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 10, enemy[i].y + 8, 'Q', 0x04);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 11, enemy[i].y + 8, 'Q', 0x04);

					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 2, enemy[i].y + 9, 'Q', 0x06);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 3, enemy[i].y + 9, 'Q', 0x06);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 4, enemy[i].y + 9, 'Q', 0x06);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 5, enemy[i].y + 9, 'Q', 0x06);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 10, enemy[i].y + 9, 'Q', 0x06);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 11, enemy[i].y + 9, 'Q', 0x06);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 12, enemy[i].y + 9, 'Q', 0x06);
					gotoxyAndPutchar(MX / 2 - centerLine + enemy[i].x + 13, enemy[i].y + 9, 'Q', 0x06);
				}
			}
		}



		{

		}
	}
}

//显示分数
void SuperMushRoom::showScore()
{
	gotoxy(200, 1);
	printf_s("Score : %d", score);
}

//显示关卡
void SuperMushRoom::showChapter()
{
	gotoxy(220, 1);
	printf_s("Chapter : %d", chapter);
}


//显示生命
void SuperMushRoom::showLife()
{
	gotoxy(240, 1);
	printf_s("Life : %d", life);
}

//传输标准缓冲区中的画面到显示的缓冲区
void SuperMushRoom::copyToBuffer(HANDLE SHandle, int STop, int SLeft, int SBottom, int SRight, HANDLE DHandle, int DTop, int DLeft, int DBottom, int DRight)
{

	// 设置源方框 
	SMALL_RECT srctReadRect;    //从源缓冲区截取的方框
	srctReadRect.Top = STop;
	srctReadRect.Left = SLeft;
	srctReadRect.Bottom = SBottom - 1;
	srctReadRect.Right = SRight - 1;

	//设置临时储存的数组的大小，多少行，多少列
	COORD coordBufSize;
	coordBufSize.Y = SBottom - STop;
	coordBufSize.X = SRight - SLeft;


	//设置临时缓冲开始装入的开始位置
	COORD coordBufCoord;
	coordBufCoord.X = 0;
	coordBufCoord.Y = 0;

	//从屏幕拷贝“块”到临时储存的数组
	ReadConsoleOutput(
		SHandle,        // screen buffer to read from 
		chiBuffer,      // buffer to copy into 
		coordBufSize,   // col-row size of chiBuffer 
		coordBufCoord,  // top left dest. cell in chiBuffer 
		&srctReadRect); // screen buffer source rectangle 



	// 设置目标方框  
	SMALL_RECT srctWriteRect;    //写到新缓冲区的方框
	srctWriteRect.Top = DTop;
	srctWriteRect.Left = DLeft;
	srctWriteRect.Bottom = DBottom - 1;
	srctWriteRect.Right = DRight - 1;

	//从临时储存的数组拷贝“块”到屏幕目标缓冲区
	WriteConsoleOutput(
		DHandle,           // screen buffer to write to 
		chiBuffer,        // buffer to copy from 
		coordBufSize,     // col-row size of chiBuffer 
		coordBufCoord,    // top left src cell in chiBuffer 
		&srctWriteRect);  // dest. screen buffer rectangle 

}

//控制主角移动
void SuperMushRoom::control()
{

	int ch;             //储存获取的命令值
	int k = 0;          //k  n 组合 控制子弹发射频率
	int n = 0;


	clock_t  t1, t2;
	t1 = clock();
	while (1)
	{



		//Sleep(20);//控制游戏的进行速度
		t = sqrt(2 * JH / G) / 10; //模拟重力
		k++;
		if (k == 1000)
			k = 0;
		if (_kbhit())
		{
			ch = GetCommand();
			if (ch&CMD_LEFT)
				left();
			if (ch&CMD_RIGHT)
				right();
			if ((ch&CMD_UP))
				up();
			if (ch&CMD_DOWN)
				hero.turn = 0;
			if (ch&CMD_ESC)
			{
				SuperMushRoomPause();

			}
			if (ch & CMD_SHOOT && arm == 1)
			{
				if (n == 0)//第一次按就直接发射
				{
					shoot();
					n = 1;
				}
				n++;
				if (k % 10 == 0 && n > 10)//如果是长按，则让它一定频率发射
				{
					shoot();
				}
			}
			else//不是长按
				n = 0;
		}

		judge();
		if (GameState == 10 || GameState == 20)//正常返回
			return;
		if (GameState == -1 )//退出
			return;

		show();


		do
		{
			t2 = clock();
		} while (t2 - t1 < 50);

		t1 = clock();


	}

	return;
}

//获取按键信息
int SuperMushRoom::GetCommand()
{
	int temp = 0;

	if (GetAsyncKeyState('A') & 0x8000)
		temp |= CMD_LEFT;
	if (GetAsyncKeyState('D') & 0x8000)
		temp |= CMD_RIGHT;
	if ((GetAsyncKeyState('W') & 0x8000) || (GetAsyncKeyState('K') & 0x8000))
		temp |= CMD_UP;
	if (GetAsyncKeyState('S') & 0x8000)
		temp |= CMD_DOWN;
	if (GetAsyncKeyState('J') & 0x8000)
		temp |= CMD_SHOOT;
	if (GetAsyncKeyState(VK_ESCAPE) & 0x8000)
		temp |= CMD_ESC;
	return temp;
}

//主角跳跃
void SuperMushRoom::up()
{
	if (hero.jump != 0)
		return;
	else
	{
		mciSendString("play jump from 0", NULL, 0, NULL);
		v0 = -sqrt(2 * G * JH);
		hero.jump = 1;
	}
}

//主角向左运动
void SuperMushRoom::left()
{
	hero.x -= STEP;
	hero.turn = -1;
	if (HeroisHitWall())
	{
		hero.x += STEP;
	}
}

//主角向右运动
void SuperMushRoom::right()
{
	hero.x += STEP;
	hero.turn = 1;
	if (HeroisHitWall())
	{
		hero.x -= STEP;
	}
}

//主角开枪
void SuperMushRoom::shoot()
{
	if(GameState == 0)
		mciSendString("play bullet from 0 ", NULL, 0, NULL);
	else if(GameState == 1)
		mciSendString("play Sbullet from 0 ", NULL, 0, NULL);
	if (hero.y + HY / 2 >= 0 && hero.y + HY / 2 < MY)
	{
		bullet[n_bullet].id = 1;
		if (hero.turn == -1)
		{
			bullet[n_bullet].x = hero.x - 1;
			bullet[n_bullet].xLeftMost = bullet[n_bullet].x - 120;//射程
			bullet[n_bullet].turn = -1;
		}
		else
		{
			bullet[n_bullet].x = hero.x + HX;
			bullet[n_bullet].xRightMost = bullet[n_bullet].x + 120;//射程
			bullet[n_bullet].turn = 1;
		}
		bullet[n_bullet].y = hero.y + HY / 2;
		if (n_bullet >= 19)
		{
			n_bullet = 0;
		}
		else
		{
			n_bullet++;
		}
	}
}

//子弹撞到东西
bool SuperMushRoom::BulletHit()
{
	bool hit = false;

	for (int i = 0; i < 20; i++)
	{
		//超出射程
		if (bullet[i].id != 0)
		{
			if (bullet[i].turn == -1)
			{
				if (bullet[i].x < bullet[i].xLeftMost)
					bullet[i].id = 0;
			}
			else
			{
				if (bullet[i].x > bullet[i].xRightMost)
					bullet[i].id = 0;
			}


		}


		//墙
		if (bullet[i].id != 0)
		{
			int k, m;
			if (bullet[i].turn == 1)
			{
				k = bullet[i].x - STEP - STEP / 2;
				m = bullet[i].x;
			}
			else
			{
				k = bullet[i].x;
				m = bullet[i].x + STEP + STEP / 2;

			}
			for (; k <= m; k++)
			{

				if (bullet[i].y < MY && BMap[bullet[i].y][k] == '#')
				{
					hit = true;
					bullet[i].id = 0;
					mciSendString("play hit_wall from 0 ", NULL, 0, NULL);
				}
			}
		}

		// 敌人
		if (bullet[i].id != 0)
		{
			for (int j = 0; j < N_EMERY; j++)
			{
				if (enemy[j].id != 0 && (abs(bullet[i].y - (enemy[j].y + HY / 2)) <= HY / 2 && abs(bullet[i].x - (enemy[j].x + HX / 2)) <= HX / 2))
				{
					bullet[i].id = 0;
					enemy[j].id = 0;
					if (GameState == 0)
						mciSendString("play hit_enemy from 0 ", NULL, 0, NULL);
					else if (GameState == 1)
						mciSendString("play Shit_enemy from 0 ", NULL, 0, NULL);
					score += 100;
				}
			}
		}

	}
	return hit;
}

//上升时撞到头
bool SuperMushRoom::HeroUpHitWall()
{
	bool result = false;
	for (int i = hero.y; i < hero.y + HY; i++)
	{
		for (int j = hero.x; j < hero.x + HX; j++)
		{
			if (i <= MY && i >= 0 && BMap[i][j] == '#')
			{

				//
				BMap[i][j] = ' ';
				if (!result)
				{
					result = true;
					mciSendString("play hit_brick from 0", NULL, 0, NULL);
					score += 50;
				}
			}
		}
	}
	return result;

}

//游戏暂停
void SuperMushRoom::SuperMushRoomPause()
{
	if (GameState == 0)
	{
		mciSendString("stop   bgm ", NULL, 0, NULL);
	}
	else if (GameState == 1)
	{
		mciSendString("stop   Sbgm ", NULL, 0, NULL);
	}
	while (_kbhit())
		char c = _getch();

	ifstream fp(".\\res\\pause.txt");                                                        //输出暂停界面
	if (!fp)
	{
		cerr << "open pause.txt error" << endl;
		exit(1);
	}
	char tempStr[300];
	int i = 0;
	while (!fp.eof())
	{
		gotoxy(0, i++);
		fp.getline(tempStr, 300);
		puts(tempStr);
	}
	fp.close();
	int q;     //用于移动光标时定位
	int t = 1;  //选项1 CONTINUE   2 EXIT
	while (1)
	{

		if (_kbhit())
		{
			char ch = getch();
			if (ch == 13)//ENTER
			{
				if (t == 1)
				{
					if(GameState == 0)
						mciSendString("play bgm", NULL, 0, NULL);
					else if(GameState == 1)
						mciSendString("play Sbgm", NULL, 0, NULL);
					break;
				}
				else if (t == -1)
				{
					GameState = -1;
					break;
				}
			}
			else if (ch == 'w' || ch == 's')
			{
				if (t == 1)//start
					q = 44;
				else if (t == -1)//exit
					q = 55;
				for (int i = 0; i < 8; i++)
				{
					gotoxy(80, q++);
					printf("               ");
				}
				t = -t;
				if (t == 1)//start
				{
					q = 44;
				}
				else if (t == -1)//exit
				{
					q = 55;
				}
				gotoxy(80, q++);
				printf("         H");
				gotoxy(80, q++);
				printf("         HH");
				gotoxy(80, q++);
				printf("         HHH");
				gotoxy(80, q++);
				printf("HHHHHHHHHHHHH");
				gotoxy(80, q++);
				printf("HHHHHHHHHHHHH");
				gotoxy(80, q++);
				printf("         HHH");
				gotoxy(80, q++);
				printf("         HH");
				gotoxy(80, q++);
				printf("         H");
			}
		}

		copyToBuffer(hStdout, 0, 0, MY, 260, hNewScreenBuffer, 0, 0, MY, 260);

		Sleep(25);
	}



}

//站在地面
bool SuperMushRoom::HeroisLanded()
{

	int lines = -1;
	for (int i = hero.y; i <= hero.y + HY; i++)
	{
		for (int j = hero.x; j < hero.x + HX; j++)
		{
			if (i <= MY&& i >= 0 && BMap[i][j] == '#')
			{
				lines = i - hero.y;
				break;
			}
		}
		if (lines != -1)
			break;
	}
	if (lines != -1)
	{
		hero.y -= (HY - lines);
		return true;
	}
	return false;

}

//左右撞墙
bool SuperMushRoom::HeroisHitWall()
{
	for (int i = hero.y; i < hero.y + HY; i++)
	{
		for (int j = hero.x; j <= hero.x + HX; j++)
		{
			if ((i < MY) && i >= 0 && (BMap[i][j] == '#'))
			{
				mciSendString("play hit_wall from 0", NULL, 0, NULL);
				return true;
			}
		}
	}
	return false;
}

//是否碰到金币
bool SuperMushRoom::iscoin()
{
	int tempHerox = hero.x + HX / 2;
	int tempHeroY = hero.y + HY / 2;

	for (int i = 0; i < N_COIN; i++)
	{
		if (coins[i].id != 0)
		{
			if (abs(coins[i].x - tempHerox) <= HX / 2 && abs(coins[i].y - tempHeroY) <= HY / 2)
			{
				coins[i].id = 0;
				mciSendString("play coin from 0", NULL, 0, NULL);
				return true;
			}
		}
	}
	return false;

}

//是否碰到敌人
bool SuperMushRoom::isEnemy()
{
	for (int i = 0; i < N_EMERY; i++)
	{
		if (enemy[i].id == 1)
		{
			if (abs(enemy[i].x - hero.x) < HX  && abs(enemy[i].y - hero.y) == 0)
			{
				return true;
			}
		}
	}
	return false;
}

//下降踩到敌人
bool SuperMushRoom::HeroDownStep()
{
	for (int i = 0; i < N_EMERY; i++)
	{
		if (enemy[i].id == 1)
		{
			if (abs(hero.x - enemy[i].x) < HX  && hero.y > enemy[i].y  && hero.y - enemy[i].y <= HY && v0 > 0)
			{
				enemy[i].id = -2;
				score += 100;
				return true;
			}
		}
	}
	return false;
}

//处理 判断
void  SuperMushRoom::judge()
{

	//第二关移动梯子
	static int  times = 0;
	static int  ly = 48; //记录移动的梯子的y坐标
	if (chapter == 2 && (times++) % 3 == 0)//3下动一下
	{
		for (int i = 0; i < 3; i++)
		{
			for (int j = 2080 + 11; j <= 2080 + 57; j++)
			{
				BMap[(ly + i) % 80][j] = ' ';
			}
		}
		for (int i = 0; i < 3; i++)
		{
			for (int j = 2080 + 11; j <= 2080 + 57; j++)
			{
				BMap[(ly + i + 3) % 80][j] = '#';
			}
		}

		ly += 3;
	}


	//移动敌人
	for (int i = 0; i < N_EMERY; i++)
	{
		if (enemy[i].id == 1)
		{
			if (enemy[i].turn == -1)
				enemy[i].x -= STEP;
			else
				enemy[i].x += STEP;
			if (enemy[i].x <= enemy[i].xLeftMost)
			{
				enemy[i].turn = 1;
			}
			if (enemy[i].x >= enemy[i].xRightMost)
			{
				enemy[i].turn = -1;
			}

		}
	}

	//移动子弹
	for (int i = 0; i < 20; i++)
	{
		if (bullet[i].id != 0)
		{
			bullet[i].x += bullet[i].turn * STEP * 3 / 2;
		}
	}

	BulletHit();


	if (centerLine - hero.x >= MX / 2)
		hero.x = centerLine - MX / 2;

	if (hero.jump != 0)//腾空
	{
		h = v0*t + G*pow(t, 2) / 2;

		hero.y += (int)(h + 0.5);//四舍五入
		if (v0 >= 0)   //自由落体
		{

			if (HeroDownStep())//踩到怪了，速度变为0
			{
				v0 = 0;
				hero.y = hero.y / HY*HY;
				hero.jump = 0;
				if(GameState == 0)
					mciSendString("play step from 0 ", NULL, 0, NULL);
				else if(GameState == 1)
					mciSendString("play Sstep from 0 ", NULL, 0, NULL);
			}
		}
		else       //向上跳跃
		{


			if (HeroUpHitWall())
			{
				v0 = 0;
				h = 0;
				hero.y = hero.y / HY*HY + HY;
			}
		}
		v0 = v0 + G*t;
		if (HeroisLanded())
		{
			hero.jump = 0;
		}
		if (HeroUpHitWall())
		{
			hero.jump = -hero.jump;
			hero.y++;
		}

	}
	if ((!HeroisLanded()) && hero.jump == 0)
	{
		hero.jump = -1;
	}
	//获取武器
	if (weapon.id == 1 && abs(hero.x - weapon.x) < HX / 2 && abs(hero.y - weapon.y) < HY / 2)
	{
		weapon.id = 0;
		if (arm == 0)
		{
			arm = 1;
		}
		else
		{
			life++;
		}
		score += 500;
		if (GameState == 0)
			mciSendString("play arm from 0", NULL, 0, NULL);
		else if (GameState == 1)
			mciSendString("play Sarm from 0", NULL, 0, NULL);
		
	}


	//金币
	if (iscoin())
	{
		score += 50;
	}

	if (HeroDownStep())
	{
		if (GameState == 0)
			mciSendString("play step from 0 ", NULL, 0, NULL);
		else if (GameState == 1)
			mciSendString("play Sstep from 0 ", NULL, 0, NULL);

	}
	else if (isEnemy() || hero.y >= MY + 100)
	{
		dead();
		return;

	}
	if (hero.x >= MX * MK - 60)
	{
		win();
		return;
	}

}

//处理死
void SuperMushRoom::dead()
{

	if(GameState == 0)
		mciSendString("stop   bgm ", NULL, 0, NULL);
	else if(GameState == 1)
		mciSendString("stop   Sbgm ", NULL, 0, NULL);
	if(GameState == 0)
		mciSendString("play  dead1 from 0", NULL, 0, NULL);
	else if(GameState == 1)
		mciSendString("play  Sdead1 from 0", NULL, 0, NULL);
	Sleep(5000);

	hero.x = XX;
	hero.y = YY;
	hero.turn = 0;
	hero.jump = 0;

	v0 = 0;
	h = 0;
	t = 0;

	arm = 0;
	n_bullet = 0;                                                                       //第几颗炮弹
	score = 0;
	centerLine = MX / 2;


	life--;


	ifstream fp(".\\res\\life.txt");                                                        //输出显示生命界面
	if (!fp)
	{
		cerr << "open life.txt error" << endl;
		exit(1);
	}
	char tempStr[300];
	int i = 0;
	while (!fp.eof())
	{
		gotoxy(0, i++);
		fp.getline(tempStr, 300);
		puts(tempStr);
	}
	fp.close();


	switch (life)
	{
	case 0:
		i = 29;
		gotoxy(203, i++);
		puts("                       *******                               "); gotoxy(203, i++);
		puts("                    *************                            "); gotoxy(203, i++);
		puts("                   ***************                           "); gotoxy(203, i++);
		puts("                  *****************                          "); gotoxy(203, i++);
		puts("                 ******      *******                         "); gotoxy(203, i++);
		puts("                ******         ******                        "); gotoxy(203, i++);
		puts("               ******           *****                        "); gotoxy(203, i++);
		puts("               *****            ******                       "); gotoxy(203, i++);
		puts("              *****              *****                       "); gotoxy(203, i++);
		puts("              *****              *****                       "); gotoxy(203, i++);
		puts("              *****               *****                      "); gotoxy(203, i++);
		puts("             *****                *****                      "); gotoxy(203, i++);
		puts("             *****                *****                      "); gotoxy(203, i++);
		puts("             *****                *****                      "); gotoxy(203, i++);
		puts("             *****                ******                     "); gotoxy(203, i++);
		puts("            *****                  *****                     "); gotoxy(203, i++);
		puts("            *****                  *****                     "); gotoxy(203, i++);
		puts("            *****                  *****                     "); gotoxy(203, i++);
		puts("            *****                  *****                     "); gotoxy(203, i++);
		puts("            *****                  *****                     "); gotoxy(203, i++);
		puts("            *****                  *****                     "); gotoxy(203, i++);
		puts("            *****                  *****                     "); gotoxy(203, i++);
		puts("            *****                  *****                     "); gotoxy(203, i++);
		puts("            *****                  *****                     "); gotoxy(203, i++);
		puts("            *****                  *****                     "); gotoxy(203, i++);
		puts("            *****                  *****                     "); gotoxy(203, i++);
		puts("            *****                  *****                     "); gotoxy(203, i++);
		puts("            *****                  *****                     "); gotoxy(203, i++);
		puts("            *****                  *****                     "); gotoxy(203, i++);
		puts("            *****                  ****                      "); gotoxy(203, i++);
		puts("             *****                *****                      "); gotoxy(203, i++);
		puts("             *****                *****                      "); gotoxy(203, i++);
		puts("             *****                *****                      "); gotoxy(203, i++);
		puts("             *****                ****                       "); gotoxy(203, i++);
		puts("              *****              *****                       "); gotoxy(203, i++);
		puts("              *****              *****                       "); gotoxy(203, i++);
		puts("              ******            *****                        "); gotoxy(203, i++);
		puts("               ******           *****                        "); gotoxy(203, i++);
		puts("               *******         *****                         "); gotoxy(203, i++);
		puts("                *******      ******                          "); gotoxy(203, i++);
		puts("                 *****************                           "); gotoxy(203, i++);
		puts("                  ***************                            "); gotoxy(203, i++);
		puts("                    ************                             "); gotoxy(203, i++);
		puts("                      *******                                "); gotoxy(203, i++);
		break;
	case 1:
		i = 29;
		gotoxy(203, i++);
		puts("                          ***                                "); gotoxy(203, i++);
		puts("                         ****                                "); gotoxy(203, i++);
		puts("                        *****                                "); gotoxy(203, i++);
		puts("                      *******                                "); gotoxy(203, i++);
		puts("                    *********                                "); gotoxy(203, i++);
		puts("                  ***********                                "); gotoxy(203, i++);
		puts("                ******* *****                                "); gotoxy(203, i++);
		puts("              ********  *****                                "); gotoxy(203, i++);
		puts("              ******    *****                                "); gotoxy(203, i++);
		puts("              ****      *****                                "); gotoxy(203, i++);
		puts("              **        *****                                "); gotoxy(203, i++);
		puts("                        *****                                "); gotoxy(203, i++);
		puts("                        *****                                "); gotoxy(203, i++);
		puts("                        *****                                "); gotoxy(203, i++);
		puts("                        *****                                "); gotoxy(203, i++);
		puts("                        *****                                "); gotoxy(203, i++);
		puts("                        *****                                "); gotoxy(203, i++);
		puts("                        *****                                "); gotoxy(203, i++);
		puts("                        *****                                "); gotoxy(203, i++);
		puts("                        *****                                "); gotoxy(203, i++);
		puts("                        *****                                "); gotoxy(203, i++);
		puts("                        *****                                "); gotoxy(203, i++);
		puts("                        *****                                "); gotoxy(203, i++);
		puts("                        *****                                "); gotoxy(203, i++);
		puts("                        *****                                "); gotoxy(203, i++);
		puts("                        *****                                "); gotoxy(203, i++);
		puts("                        *****                                "); gotoxy(203, i++);
		puts("                        *****                                "); gotoxy(203, i++);
		puts("                        *****                                "); gotoxy(203, i++);
		puts("                        *****                                "); gotoxy(203, i++);
		puts("                        *****                                "); gotoxy(203, i++);
		puts("                        *****                                "); gotoxy(203, i++);
		puts("                        *****                                "); gotoxy(203, i++);
		puts("                        *****                                "); gotoxy(203, i++);
		puts("                        *****                                "); gotoxy(203, i++);
		puts("                        *****                                "); gotoxy(203, i++);
		puts("                        *****                                "); gotoxy(203, i++);
		puts("                        *****                                "); gotoxy(203, i++);
		puts("                        *****                                "); gotoxy(203, i++);
		puts("                        *****                                "); gotoxy(203, i++);
		puts("                        *****                                "); gotoxy(203, i++);
		puts("                        *****                                "); gotoxy(203, i++);
		puts("                        *****                                "); gotoxy(203, i++);
		puts("                                                             "); gotoxy(203, i++);
		break;
	case 2:
		i = 29;
		gotoxy(203, i++);
		puts("                       *********                             "); gotoxy(203, i++);
		puts("                     *************                           "); gotoxy(203, i++);
		puts("                   *****************                         "); gotoxy(203, i++);
		puts("                  *******************                        "); gotoxy(203, i++);
		puts("                 *******      ********                       "); gotoxy(203, i++);
		puts("                 *****          *******                      "); gotoxy(203, i++);
		puts("                 ***             ******                      "); gotoxy(203, i++);
		puts("                 **               *****                      "); gotoxy(203, i++);
		puts("                 *                ******                     "); gotoxy(203, i++);
		puts("                                   *****                     "); gotoxy(203, i++);
		puts("                                   *****                     "); gotoxy(203, i++);
		puts("                                   *****                     "); gotoxy(203, i++);
		puts("                                   *****                     "); gotoxy(203, i++);
		puts("                                   *****                     "); gotoxy(203, i++);
		puts("                                   *****                     "); gotoxy(203, i++);
		puts("                                  ******                     "); gotoxy(203, i++);
		puts("                                  *****                      "); gotoxy(203, i++);
		puts("                                 ******                      "); gotoxy(203, i++);
		puts("                                 ******                      "); gotoxy(203, i++);
		puts("                                ******                       "); gotoxy(203, i++);
		puts("                               ******                        "); gotoxy(203, i++);
		puts("                              *******                        "); gotoxy(203, i++);
		puts("                            ********                         "); gotoxy(203, i++);
		puts("                           ********                          "); gotoxy(203, i++);
		puts("                          ********                           "); gotoxy(203, i++);
		puts("                        ********                             "); gotoxy(203, i++);
		puts("                       ********                              "); gotoxy(203, i++);
		puts("                      ********                               "); gotoxy(203, i++);
		puts("                     *******                                 "); gotoxy(203, i++);
		puts("                   ********                                  "); gotoxy(203, i++);
		puts("                   *******                                   "); gotoxy(203, i++);
		puts("                  *******                                    "); gotoxy(203, i++);
		puts("                 ******                                      "); gotoxy(203, i++);
		puts("                 ******                                      "); gotoxy(203, i++);
		puts("                ******                                       "); gotoxy(203, i++);
		puts("                *****                                        "); gotoxy(203, i++);
		puts("               ******                                        "); gotoxy(203, i++);
		puts("               ******                                        "); gotoxy(203, i++);
		puts("               ******                                        "); gotoxy(203, i++);
		puts("               **************************                    "); gotoxy(203, i++);
		puts("               **************************                    "); gotoxy(203, i++);
		puts("               **************************                    "); gotoxy(203, i++);
		puts("               **************************                    "); gotoxy(203, i++);
		puts("                                                             "); gotoxy(203, i++);
		break;
	case 3:
		i = 29;
		gotoxy(203, i++);
		puts("                         ********                            "); gotoxy(203, i++);
		puts("                       *************                         "); gotoxy(203, i++);
		puts("                     ****************                        "); gotoxy(203, i++);
		puts("                    *******************                      "); gotoxy(203, i++);
		puts("                    *****       *******                      "); gotoxy(203, i++);
		puts("                    ***           ******                     "); gotoxy(203, i++);
		puts("                    *              *****                     "); gotoxy(203, i++);
		puts("                                   ******                    "); gotoxy(203, i++);
		puts("                                    *****                    "); gotoxy(203, i++);
		puts("                                    *****                    "); gotoxy(203, i++);
		puts("                                    *****                    "); gotoxy(203, i++);
		puts("                                    *****                    "); gotoxy(203, i++);
		puts("                                    *****                    "); gotoxy(203, i++);
		puts("                                    *****                    "); gotoxy(203, i++);
		puts("                                   *****                     "); gotoxy(203, i++);
		puts("                                   *****                     "); gotoxy(203, i++);
		puts("                                  *****                      "); gotoxy(203, i++);
		puts("                                 *****                       "); gotoxy(203, i++);
		puts("                              *******                        "); gotoxy(203, i++);
		puts("                      **************                         "); gotoxy(203, i++);
		puts("                      ************                           "); gotoxy(203, i++);
		puts("                      **************                         "); gotoxy(203, i++);
		puts("                      ****************                       "); gotoxy(203, i++);
		puts("                              *********                      "); gotoxy(203, i++);
		puts("                                 *******                     "); gotoxy(203, i++);
		puts("                                   ******                    "); gotoxy(203, i++);
		puts("                                   ******                    "); gotoxy(203, i++);
		puts("                                    *****                    "); gotoxy(203, i++);
		puts("                                     *****                   "); gotoxy(203, i++);
		puts("                                     *****                   "); gotoxy(203, i++);
		puts("                                     *****                   "); gotoxy(203, i++);
		puts("                                     *****                   "); gotoxy(203, i++);
		puts("                                     *****                   "); gotoxy(203, i++);
		puts("                                     *****                   "); gotoxy(203, i++);
		puts("                                     *****                   "); gotoxy(203, i++);
		puts("                                    *****                    "); gotoxy(203, i++);
		puts("                                    *****                    "); gotoxy(203, i++);
		puts("                  *                *****                     "); gotoxy(203, i++);
		puts("                  ***            *******                     "); gotoxy(203, i++);
		puts("                  ******       ********                      "); gotoxy(203, i++);
		puts("                  ********************                       "); gotoxy(203, i++);
		puts("                   ******************                        "); gotoxy(203, i++);
		puts("                     **************                          "); gotoxy(203, i++);
		puts("                       *********                             "); gotoxy(203, i++);
		break;

	default:
		i = 29;
		gotoxy(203, i++);
		puts("                   ********                    **            "); gotoxy(203, i++);
		puts("                 *************                 **            "); gotoxy(203, i++);
		puts("               ****************                **            "); gotoxy(203, i++);
		puts("              *******************              **            "); gotoxy(203, i++);
		puts("              *****       *******          **********        "); gotoxy(203, i++);
		puts("              ***           ******         **********        "); gotoxy(203, i++);
		puts("              *              *****             **            "); gotoxy(203, i++);
		puts("                             ******            **            "); gotoxy(203, i++);
		puts("                              *****            **            "); gotoxy(203, i++);
		puts("                              *****            **            "); gotoxy(203, i++);
		puts("                              *****                          "); gotoxy(203, i++);
		puts("                              *****                          "); gotoxy(203, i++);
		puts("                              *****                          "); gotoxy(203, i++);
		puts("                              *****                          "); gotoxy(203, i++);
		puts("                             *****                           "); gotoxy(203, i++);
		puts("                             *****                           "); gotoxy(203, i++);
		puts("                            *****                            "); gotoxy(203, i++);
		puts("                           *****                             "); gotoxy(203, i++);
		puts("                        *******                              "); gotoxy(203, i++);
		puts("                **************                               "); gotoxy(203, i++);
		puts("                ************                                 "); gotoxy(203, i++);
		puts("                **************                               "); gotoxy(203, i++);
		puts("                ****************                             "); gotoxy(203, i++);
		puts("                        *********                            "); gotoxy(203, i++);
		puts("                           *******                           "); gotoxy(203, i++);
		puts("                             ******                          "); gotoxy(203, i++);
		puts("                             ******                          "); gotoxy(203, i++);
		puts("                              *****                          "); gotoxy(203, i++);
		puts("                               *****                         "); gotoxy(203, i++);
		puts("                               *****                         "); gotoxy(203, i++);
		puts("                               *****                         "); gotoxy(203, i++);
		puts("                               *****                         "); gotoxy(203, i++);
		puts("                               *****                         "); gotoxy(203, i++);
		puts("                               *****                         "); gotoxy(203, i++);
		puts("                               *****                         "); gotoxy(203, i++);
		puts("                              *****                          "); gotoxy(203, i++);
		puts("                              *****                          "); gotoxy(203, i++);
		puts("            *                *****                           "); gotoxy(203, i++);
		puts("            ***            *******                           "); gotoxy(203, i++);
		puts("            ******       ********                            "); gotoxy(203, i++);
		puts("            ********************                             "); gotoxy(203, i++);
		puts("             ******************                              "); gotoxy(203, i++);
		puts("               **************                                "); gotoxy(203, i++);
		puts("                 *********                                   "); gotoxy(203, i++);
		break;

	}

	copyToBuffer(hStdout, 0, 0, MY, 260, hNewScreenBuffer, 0, 0, MY, 260);

	Sleep(5000);


	if (life <= 0)
	{
		mciSendString("play  gameover from 0", NULL, 0, NULL);

		ifstream fp1(".\\res\\gameover.txt");                                                        //输出GameOver界面
		if (!fp1)
		{
			cerr << "open gameover.txt error" << endl;
			exit(1);
		}
		char tempStr[300];
		int i = 0;
		while (!fp1.eof())
		{
			gotoxy(0, i++);
			fp1.getline(tempStr, 300);
			puts(tempStr);
		}
		fp1.close();

		copyToBuffer(hStdout, 0, 0, MY, 260, hNewScreenBuffer, 0, 0, MY, 260);
		Sleep(5000);
		GameState = -1;
		return;
	}
	
	GameState = (GameState + 1) * 10;  //用于返回
	return;

}

//处理赢
void SuperMushRoom::win()
{
	if (GameState == 0)
		mciSendString("stop   bgm ", NULL, 0, NULL);
	else if (GameState == 1)
		mciSendString("stop   Sbgm ", NULL, 0, NULL);

	mciSendString("play  win  from 0", NULL, 0, NULL);
	for (int i = 0; i < 37; i++)
	{
		for (int j = 0; j < 9; j++)
		{
			BMap[26 + i][260 * 8 + 209 + j] = ' ';
			BMap[26 + i + 4][260 * 8 + 209 + j] = 'l';
		}
		BMap[26 + i + 4][260 * 8 + 209 + 8] = '>';
		show();
		Sleep(300);
	}
	Sleep(1000);


	hero.x = XX;
	hero.y = YY;
	hero.turn = 0;
	hero.jump = 0;

	v0 = 0;
	h = 0;
	t = 0;

	n_bullet = 0;                                                                       //第几颗炮弹

	centerLine = MX / 2;


	chapter++;
	//if (chapter <= 8)
	if (chapter <= 4)
	{
		GameState = (GameState + 1) * 10;  //用于返回
		return;
	}
	else if(GameState == 0)
	{

		while (_kbhit())
			char ch = _getch();
		//使显示的是标准缓冲区	 
		SetConsoleActiveScreenBuffer(hStdout);
		//播放字符动画

		CharacterAnimation A(".\\res\\win0.txt", ".\\res\\BadApple.mp3", 160, 60 + 1);//46 + 1个空行   演示画面
		//CharacterAnimation A("win0.txt", "BadApple.mp3", 240, 92, 71);//46 + 1个空行
		A.display();


		GameState = 1;                                                                     //逆袭版

		chapter = 1;																	   //第一关
		life = 3;                                                                          //生命数
		hero.id = 1;
		hero.x = XX;																	   //人物出现位置
		hero.y = YY;
		hero.turn = 0;                                                                     //人物朝向
		hero.state = 0;
		hero.jump = 0;                                                                     //不跳跃

		v0 = 0;																				//角色跳跃的初速度
		h = 0;																				//角色跳跃的高度
		t = 0;																				//角色跳跃的时间

																							//
		arm = 0;                                                                            //没有装备枪
		n_bullet = 0;                                                                       //第几颗炮弹
		score = 0;                                                                          //得分
		centerLine = MX / 2;                                                                //初始中线



		GameState = (GameState + 1) * 10;  //用于返回
		return;
	}
	else if(GameState == 1)
	{
		GameState = 2;
		Duel B;
		B.start();
		return;
	}
}

