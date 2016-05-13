#include <iostream>
#include <conio.h>
#include <math.h>
#include <Windows.h>
#include <fstream>
#include <cstdio>
#include "SuperMushRoom.h"
#include "CharacterAnimation.h"
#include "duel.h"
using namespace std;
#pragma comment(lib,"Winmm.lib")   //给游戏添加音乐要用到它
int life;                          //全局变量，生命
int score;                         //全局变量，分数
int chapter = 1;                   //全局变量，关卡
int GameState = 0;                    //游戏进行到什么状态

int N_COIN = 13;                   //硬币数
int N_EMERY = 14;                  //敌人数

void login();					  //登录进入界面
void help();					  //显示帮助

int  main()
{
	N_COIN = 24;
	N_EMERY = 14;
	while(1)
		login();
}

//登录进入界面
void login()
{
	void hideCursor();
	void gotoxyAndPutchar(int x, int y, char c, int color = 0x07);
	void gotoxy(int x, int y);                                           //移动光标到x,y
	void setFontSizeMode(int n);
	void remove_scrollbar();                                             //隐藏滚动条

	SetConsoleDisplayMode(GetStdHandle(STD_OUTPUT_HANDLE), CONSOLE_FULLSCREEN_MODE, 0);//全屏
																					   //hideCursor();                                                                      //隐藏光标
	setFontSizeMode(7);                                                                //设置字体模式7
	system("mode con cols=500 lines=500");                                             //控制台行列
	remove_scrollbar();                                                               //隐藏滚动条
	hideCursor();

	ifstream fp(".\\res\\login.txt");                                                        //输出登录界面

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
	int t = 1;  //选项1 ENTER   2 EXIT
	int count = 0;  //用于确定是否应该进入演示画面
	while (1)
	{
		++count;
		if (count >= 300000)
		{
			CharacterAnimation A(".\\res\\yanshi.txt", ".\\res\\login.mp3", 260, 79 + 1);//46 + 1个空行   演示画面
			A.display();
			//system("cls");
			Sleep(300);
			while (_kbhit())
				char c = _getch();
			help();
			//login();
			return;
		}
//		Sleep(20);
		if (_kbhit())
		{
			count = 0;
			char ch = getch();
			if (ch == 13)//ENTER
			{
				if (t == 1)
				{
					//Duel B;
					//B.start();

					SuperMushRoom g;
					g.start();
				}
				else if (t == -1)
				{
					exit(0);
				}
				//t = 1;
				//login();
				return;
			}
			else if (ch == 'w'|| ch == 's')
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
	}
}

//显示帮助
void help()
{
	void gotoxy(int x, int y);                                                       //移动光标到x,y

	ifstream fhelp(".\\res\\help.txt");                                                        //输出登录界面

	char tempStr[300];
	int i = 0;
	while (!fhelp.eof())
	{
		gotoxy(0, i++);
		fhelp.getline(tempStr, 300);
		puts(tempStr);
		Sleep(150);
		if (_kbhit())
			break;
	}
	fhelp.close();
	while (_kbhit())
		char ch = _getch();

}