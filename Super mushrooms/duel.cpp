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
#include "duel.h"
#include "CharacterAnimation.h"
#pragma comment(lib,"Winmm.lib") 
using namespace std;



HANDLE hStdout1, hNewScreenBuffer1;                                    //������� , ��Ӧ����������
CHAR_INFO chiBuffer1[MY*MX];                                          //[MY][MX];���ڴ���ӱ�׼���������»��������ַ�
void remove_scrollbar1();                                             //���ع�����
void hideCursor1();                                                   //���ع��
void setFontSizeMode1(int n);                                         //���������Сģʽ
void gotoxy1(int x, int y);                                           //�ƶ���굽x,y
void gotoxyAndPutchar1(int x, int y, char c, int color = 0x07);       //�ƶ���굽x,y��ӡc,��ɫcolor



																	 //���ع��
void hideCursor1()
{
	CONSOLE_CURSOR_INFO cursor_info = { 1,0 };
	SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursor_info);
	SetConsoleCursorInfo(hNewScreenBuffer1, &cursor_info);
}

//�ƶ���굽x,y
void gotoxy1(int x, int y)
{
	COORD coord;			  // ����
	coord.X = x; coord.Y = y; // ���������x , y 
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord); //�ƶ���굽����(x,y) 
}

//�ƶ���굽x,y��ӡc,��ɫcolor
void gotoxyAndPutchar1(int x, int y, char c, int color)//�ƶ���굽x,y��ӡc
{
	if (x >= 0 && x <= MX && y >= 0 && y <= MY)
	{
		gotoxy1(x, y);
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
		putchar(c);
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 0x07);
	}

}

//���ع�����
void remove_scrollbar1()
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
	SetConsoleScreenBufferSize(hNewScreenBuffer1, new_size);
}

//���������Сģʽ
void setFontSizeMode1(int n)
{
	typedef BOOL(WINAPI *PROCSETCONSOLEFONT)(HANDLE, DWORD);//
	PROCSETCONSOLEFONT SetConsoleFont;						//���������СҪ��

	HANDLE hConsole;
	hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	HMODULE hKernel32 = GetModuleHandle("kernel32");
	SetConsoleFont = (PROCSETCONSOLEFONT)GetProcAddress(hKernel32, "SetConsoleFont");
	SetConsoleFont(hConsole, n);

	/*�����Сģʽ����
	x?  y? ģʽ
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


Duel::Duel()
{
	centerLine = MX / 2;                                                               //��Ļ����

	Hero.id = 1;
	Hero.x = XX;																	   //�������λ��
	Hero.y = YY;
	Hero.turn = 1;                                                                     //���ﳯ��
	Hero.state = 0;
	Hero.jump = 0;                                                                     //����Ծ
	Hero.n_bullet = 0;
	Hero.v0 = 0;																				//Ӣ����Ծ�ĳ��ٶ�
	Hero.h = 0;																				//Ӣ����Ծ�ĸ߶�
	Hero.t = 0;																				//Ӣ����Ծ��ʱ��
	Hero.hp = 100;


	Enemy.id = 1;
	Enemy.x = XX + 200;																	   //�������λ��
	Enemy.y = YY;
	Enemy.turn = -1;                                                                     //���ﳯ��
	Enemy.state = 0;
	Enemy.jump = 0;                                                                     //����Ծ
	Enemy.n_bullet = 0;
	Enemy.v0 = 0;																			//������Ծ�ĳ��ٶ�
	Enemy.h = 0;																				//������Ծ�ĸ߶�
	Enemy.t = 0;																				//������Ծ��ʱ��
	Enemy.hp = 100;

}

Duel::~Duel()
{
	//ʹ��ʾ�Ļָ�Ϊ��׼������	 
	SetConsoleActiveScreenBuffer(hStdout1);
}

//��Ϸ��ʼ
void Duel::start()
{
	while (GameState != -1)
	{
		init();//��ʼ��,��ͼ��
		mciSendString("play Bossbgm  repeat", NULL, 0, NULL);//�ظ����ű�������

		while (1)
		{

			show();
			control();
			if (GameState == 30)//��������
			{
				GameState = GameState / 10 - 1;
				break;
			}
			if (GameState == -1)
				break;
			judge();
			Sleep(20);
		}
	}
}

//��ʼ��,��ͼ��
void Duel::init()
{
	//���ô���
	setWindows();

	//���ر���
	loadBackGround();

	//��������
	loadMusic();

	//���ز�Ѫҩ
	loadBlood();

	//��ʼ���ӵ�
	for (int i = 0; i < 20; i++)
	{
		Hero.bullet[i].id = 0;
		Hero.bullet[i].x = -1;
		Hero.bullet[i].y = -1;

		Enemy.bullet[i].id = 0;
		Enemy.bullet[i].x = -1;
		Enemy.bullet[i].y = -1;
	}


}

//���ô���
void Duel::setWindows()
{
	SetConsoleDisplayMode(GetStdHandle(STD_OUTPUT_HANDLE), CONSOLE_FULLSCREEN_MODE, 0);//ȫ��
	setFontSizeMode1(7);                                                                //��������ģʽ7
	system("mode con cols=300 lines=300");                                             //����̨����
	remove_scrollbar1();                                                               //���ع�����

																					  // ����һ����� ָ��STDOUT ��������copy���ݵ��½��Ļ�����
	hStdout1 = GetStdHandle(STD_OUTPUT_HANDLE);
	// ����һ���»�����  , ����copy������ 
	hNewScreenBuffer1 = CreateConsoleScreenBuffer(
		GENERIC_READ |           // ��/д Ȩ�� 
		GENERIC_WRITE,
		FILE_SHARE_READ |
		FILE_SHARE_WRITE,        // ����
		NULL,                    // Ĭ�ϰ�ȫ����
		CONSOLE_TEXTMODE_BUFFER, // must be TEXTMODE 
		NULL);                   // reserved; must be NULL 

								 //ʹ��ʾ�����½��Ļ�����	 
	SetConsoleActiveScreenBuffer(hNewScreenBuffer1);
	//���ع��
	hideCursor1();
}

//���ر�����������
void Duel::loadBackGround()
{
	//���ر���
	ifstream fp(".\\res\\duelBackground.txt");
	if (!fp)
	{
		cerr << "Open Background error" << endl;
		exit(1);
	}
	char ch;
	int i = 0, j = 0, k = 0;
	while (!fp.eof())
	{

		ch = fp.get();
		if (ch != '\n')
		{
			BMap1[i][k * MX + j] = ch;
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

//��������
void Duel::loadMusic()
{

	mciSendString("open .\\res\\��������.mp3 alias bgm", NULL, 0, NULL);
	mciSendString("open .\\res\\�ӵ�.mp3 alias bullet", NULL, 0, NULL);
	mciSendString("open .\\res\\���.mp3 alias coin", NULL, 0, NULL);
	mciSendString("open .\\res\\��.mp3 alias jump", NULL, 0, NULL);
	mciSendString("open .\\res\\�ӵ��򵽵���.mp3 alias hit_enemy", NULL, 0, NULL);
	mciSendString("open .\\res\\�ӵ�ײǽ.mp3 alias hit_wall", NULL, 0, NULL);
	mciSendString("open .\\res\\ײ��ש��.mp3 alias hit_brick", NULL, 0, NULL);
	mciSendString("open .\\res\\�ȵ���.mp3 alias step", NULL, 0, NULL);
	mciSendString("open .\\res\\�Ե�����.mp3 alias arm", NULL, 0, NULL);
	mciSendString("open .\\res\\ʤ��.mp3 alias win", NULL, 0, NULL);
	mciSendString("open .\\res\\����1.mp3 alias dead1", NULL, 0, NULL);
	mciSendString("open .\\res\\��Ϸ����.mp3 alias gameover", NULL, 0, NULL);

	mciSendString("open .\\res\\�궷�ޱ���.mp3 alias Sbgm", NULL, 0, NULL);
	mciSendString("open .\\res\\�궷��shoot.mp3 alias Sbullet", NULL, 0, NULL);
	mciSendString("open .\\res\\�궷��boom.mp3 alias Shit_enemy", NULL, 0, NULL);
	mciSendString("open .\\res\\�궷��boom.mp3 alias Sstep", NULL, 0, NULL);
	mciSendString("open .\\res\\�궷�޳Ե�����.mp3 alias Sarm", NULL, 0, NULL);
	mciSendString("open .\\res\\�궷������1.mp3 alias Sdead1", NULL, 0, NULL);

	mciSendString("open .\\res\\boss.mp3 alias Bossbgm", NULL, 0, NULL);


}

//���ز�Ѫҩ
void Duel::loadBlood()
{
	ifstream fcoin;

	fcoin.open(".\\res\\chapter1_coins.txt");

	if (!fcoin)
	{
		cerr << "open files of coins error" << endl;
		exit(1);
	}
	for (int i = 0; i < N_COIN; i++)
	{
		fcoin >> blood[i].id >> blood[i].x >> blood[i].y >> blood[i].turn;
	}
	fcoin.close();

}

//��ʾ����
void Duel::show()
{
	if (Hero.x >= centerLine + MX / 4)          //������Ļ
		centerLine += STEP;
	if (centerLine > MX * MK - MX / 2)//�������һ��ͼ��벿�֣����ƶ�
		centerLine = MX * MK - MX / 2;

	if (Hero.x <= centerLine - MX / 4)          //������Ļ
		centerLine -= STEP;
	if (centerLine < MX / 2)//����һ��ͼǰ�벿�֣����ƶ�
		centerLine = MX / 2;

	if (Hero.x < 0)
		Hero.x = XX;
	if (Enemy.x < 0)
		Enemy.x = XX;

	//��ʾ����
	showBackground();

	//��ʾ��Ѫҩ
	showCoins();

	//��ʾ�ӵ�
	showBullet();

	//��ʾ����
	showEnemy();

	//��ʾӢ��
	showHero();

	//��ʾ����
	showLife();

	//�����׼�������еĻ��浽��ʾ�Ļ�����
	copyToBuffer(hStdout1, 0, 0, MY, 260, hNewScreenBuffer1, 0, 0, MY, 260);
}

//��ʾ����
void Duel::showBackground()
{
	char abc;
	gotoxy1(0, 0);
	for (int i = 0; i < MY; i++)
	{
		abc = BMap1[i][centerLine - MX / 2 + MX];
		BMap1[i][centerLine - MX / 2 + MX] = '\0';
		puts(&BMap1[i][centerLine - MX / 2]);
		BMap1[i][centerLine - MX / 2 + MX] = abc;
	}
}

//��ʾ���
void Duel::showCoins()
{
	for (int i = 0; i < N_COIN; i++)
	{
		if (blood[i].id == 0)
			continue;
		else if (blood[i].x > centerLine - MX / 2 && blood[i].x < centerLine + MX / 2)
		{
			gotoxyAndPutchar1(MX / 2 - (centerLine - blood[i].x) + 1, blood[i].y - 2, '$', 0x04);
			gotoxyAndPutchar1(MX / 2 - (centerLine - blood[i].x) + 2, blood[i].y - 2, '$', 0x04);

			gotoxyAndPutchar1(MX / 2 - (centerLine - blood[i].x) + 0, blood[i].y - 1, '$', 0x04);
			gotoxyAndPutchar1(MX / 2 - (centerLine - blood[i].x) + 3, blood[i].y - 1, '$', 0x04);

			gotoxyAndPutchar1(MX / 2 - (centerLine - blood[i].x) + 0, blood[i].y - 0, '$', 0x04);
			gotoxyAndPutchar1(MX / 2 - (centerLine - blood[i].x) + 3, blood[i].y - 0, '$', 0x04);

			gotoxyAndPutchar1(MX / 2 - (centerLine - blood[i].x) + 1, blood[i].y + 1, '$', 0x04);
			gotoxyAndPutchar1(MX / 2 - (centerLine - blood[i].x) + 2, blood[i].y + 1, '$', 0x04);
		}
	}
}

//��ʾ�ӵ�
void Duel::showBullet()
{
	//���ǵ��ӵ�
	for (int i = 0; i < 20; i++)
	{
		if (Hero.bullet[i].id == 0)
			continue;
		else if (Hero.bullet[i].x >= centerLine - MX / 2 && Hero.bullet[i].x <= centerLine + MX / 2)
		{
			if (Hero.bullet[i].turn == 1)
			{
				gotoxyAndPutchar1(MX / 2 - (centerLine - Hero.bullet[i].x) - 1, Hero.bullet[i].y - 1, 'O', 0x0f);
				gotoxyAndPutchar1(MX / 2 - (centerLine - Hero.bullet[i].x), Hero.bullet[i].y, 'O', 0x0f);
				gotoxyAndPutchar1(MX / 2 - (centerLine - Hero.bullet[i].x) - 1, Hero.bullet[i].y + 1 , 'O', 0x0f);
			}
			else
			{
				gotoxyAndPutchar1(MX / 2 - (centerLine - Hero.bullet[i].x) + 1, Hero.bullet[i].y - 1, 'O', 0x0f);
				gotoxyAndPutchar1(MX / 2 - (centerLine - Hero.bullet[i].x), Hero.bullet[i].y, 'O', 0x0f);
				gotoxyAndPutchar1(MX / 2 - (centerLine - Hero.bullet[i].x) + 1, Hero.bullet[i].y + 1, 'O', 0x0f);
			}
		}
	}
	//���˵��ӵ�
	for (int i = 0; i < 20; i++)
	{
		if (Enemy.bullet[i].id == 0)
			continue;
		else if (Enemy.bullet[i].x >= centerLine - MX / 2 && Enemy.bullet[i].x <= centerLine + MX / 2)
		{
			if (Enemy.bullet[i].turn == 1)
			{
				gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.bullet[i].x) - 1 , Enemy.bullet[i].y - 1, '@', 0x0c);
				gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.bullet[i].x), Enemy.bullet[i].y, '@', 0x0c);
				gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.bullet[i].x) - 1, Enemy.bullet[i].y + 1, '@', 0x0c);
			}
			else
			{
				gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.bullet[i].x) + 1, Enemy.bullet[i].y - 1, '@', 0x0c);
				gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.bullet[i].x), Enemy.bullet[i].y, '@', 0x0c);
				gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.bullet[i].x) + 1, Enemy.bullet[i].y + 1, '@', 0x0c);

			}
		}
	}
}

//��ʾ����
void Duel::showHero()
{
	if (Hero.jump != 0)//��Ծ
	{
		if (Hero.turn != -1)//����
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
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 5, Hero.y + 0, 'Q', 0x04);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 6, Hero.y + 0, 'Q', 0x04);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 7, Hero.y + 0, 'Q', 0x04);

			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 4, Hero.y + 1, 'Q', 0x04);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 5, Hero.y + 1, 'Q', 0x04);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 6, Hero.y + 1, 'Q', 0x04);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 7, Hero.y + 1, 'Q', 0x04);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 8, Hero.y + 1, 'Q', 0x04);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 9, Hero.y + 1, 'Q', 0x04);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 10, Hero.y + 1, 'Q', 0x04);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 11, Hero.y + 1, 'Q', 0x04);

			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 3, Hero.y + 2, 'Q', 0x06);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 4, Hero.y + 2, 'Q', 0x0e);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 5, Hero.y + 2, 'Q', 0x06);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 6, Hero.y + 2, 'Q', 0x0e);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 7, Hero.y + 2, 'Q', 0x0e);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 8, Hero.y + 2, 'Q', 0x0e);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 9, Hero.y + 2, 'Q', 0x06);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 10, Hero.y + 2, 'Q', 0x0e);

			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 3, Hero.y + 3, 'Q', 0x06);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 4, Hero.y + 3, 'Q', 0x0e);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 5, Hero.y + 3, 'Q', 0x06);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 6, Hero.y + 3, 'Q', 0x06);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 7, Hero.y + 3, 'Q', 0x0e);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 8, Hero.y + 3, 'Q', 0x0e);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 9, Hero.y + 3, 'Q', 0x0e);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 10, Hero.y + 3, 'Q', 0x06);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 11, Hero.y + 3, 'Q', 0x0e);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 12, Hero.y + 3, 'Q', 0x0e);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 13, Hero.y + 3, 'Q', 0x0e);

			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 5, Hero.y + 4, 'Q', 0x06);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 6, Hero.y + 4, 'Q', 0x06);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 7, Hero.y + 4, 'Q', 0x06);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 8, Hero.y + 4, 'Q', 0x06);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 9, Hero.y + 4, 'Q', 0x0e);

			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 2, Hero.y + 5, 'Q', 0x06);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 3, Hero.y + 5, 'Q', 0x06);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 4, Hero.y + 5, 'Q', 0x06);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 5, Hero.y + 5, 'Q', 0x06);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 6, Hero.y + 5, 'Q', 0x04);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 7, Hero.y + 5, 'Q', 0x04);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 8, Hero.y + 5, 'Q', 0x04);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 9, Hero.y + 5, 'Q', 0x06);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 10, Hero.y + 5, 'Q', 0x06);

			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 0, Hero.y + 6, 'Q', 0x0e);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 1, Hero.y + 6, 'Q', 0x0e);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 2, Hero.y + 6, 'Q', 0x0e);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 4, Hero.y + 6, 'Q', 0x06);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 5, Hero.y + 6, 'Q', 0x06);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 6, Hero.y + 6, 'Q', 0x04);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 7, Hero.y + 6, 'Q', 0x0e);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 8, Hero.y + 6, 'Q', 0x04);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 9, Hero.y + 6, 'Q', 0x04);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 10, Hero.y + 6, 'Q', 0x04);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 11, Hero.y + 6, 'Q', 0x04);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 12, Hero.y + 6, 'Q', 0x06);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 13, Hero.y + 6, 'Q', 0x06);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 14, Hero.y + 6, 'Q', 0x0e);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 15, Hero.y + 6, 'Q', 0x0e);

			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 0, Hero.y + 7, 'Q', 0x0e);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 1, Hero.y + 7, 'Q', 0x0e);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 4, Hero.y + 7, 'Q', 0x04);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 5, Hero.y + 7, 'Q', 0x04);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 6, Hero.y + 7, 'Q', 0x04);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 7, Hero.y + 7, 'Q', 0x04);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 8, Hero.y + 7, 'Q', 0x04);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 9, Hero.y + 7, 'Q', 0x04);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 10, Hero.y + 7, 'Q', 0x04);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 11, Hero.y + 7, 'Q', 0x04);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 14, Hero.y + 7, 'Q', 0x06);

			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 1, Hero.y + 8, 'Q', 0x06);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 2, Hero.y + 8, 'Q', 0x06);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 3, Hero.y + 8, 'Q', 0x04);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 4, Hero.y + 8, 'Q', 0x04);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 5, Hero.y + 8, 'Q', 0x04);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 9, Hero.y + 8, 'Q', 0x04);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 10, Hero.y + 8, 'Q', 0x04);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 11, Hero.y + 8, 'Q', 0x04);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 12, Hero.y + 8, 'Q', 0x04);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 13, Hero.y + 8, 'Q', 0x06);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 14, Hero.y + 8, 'Q', 0x06);

			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 1, Hero.y + 9, 'Q', 0x06);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 2, Hero.y + 9, 'Q', 0x06);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 3, Hero.y + 9, 'Q', 0x06);
		}
		else//����
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

			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 8, Hero.y + 0, 'Q', 0x04);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 9, Hero.y + 0, 'Q', 0x04);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 10, Hero.y + 0, 'Q', 0x04);

			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 4, Hero.y + 1, 'Q', 0x04);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 5, Hero.y + 1, 'Q', 0x04);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 6, Hero.y + 1, 'Q', 0x04);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 7, Hero.y + 1, 'Q', 0x04);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 8, Hero.y + 1, 'Q', 0x04);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 9, Hero.y + 1, 'Q', 0x04);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 10, Hero.y + 1, 'Q', 0x04);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 11, Hero.y + 1, 'Q', 0x04);

			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 5, Hero.y + 2, 'Q', 0x0e);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 6, Hero.y + 2, 'Q', 0x06);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 7, Hero.y + 2, 'Q', 0x0e);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 8, Hero.y + 2, 'Q', 0x0e);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 9, Hero.y + 2, 'Q', 0x0e);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 10, Hero.y + 2, 'Q', 0x06);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 11, Hero.y + 2, 'Q', 0x0e);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 12, Hero.y + 2, 'Q', 0x06);

			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 2, Hero.y + 3, 'Q', 0x0e);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 3, Hero.y + 3, 'Q', 0x0e);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 4, Hero.y + 3, 'Q', 0x0e);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 5, Hero.y + 3, 'Q', 0x06);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 6, Hero.y + 3, 'Q', 0x0e);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 7, Hero.y + 3, 'Q', 0x0e);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 8, Hero.y + 3, 'Q', 0x0e);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 9, Hero.y + 3, 'Q', 0x06);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 10, Hero.y + 3, 'Q', 0x06);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 11, Hero.y + 3, 'Q', 0x0e);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 12, Hero.y + 3, 'Q', 0x06);

			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 6, Hero.y + 4, 'Q', 0x0e);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 7, Hero.y + 4, 'Q', 0x06);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 8, Hero.y + 4, 'Q', 0x06);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 9, Hero.y + 4, 'Q', 0x06);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 10, Hero.y + 4, 'Q', 0x06);

			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 5, Hero.y + 5, 'Q', 0x06);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 6, Hero.y + 5, 'Q', 0x06);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 7, Hero.y + 5, 'Q', 0x04);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 8, Hero.y + 5, 'Q', 0x04);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 9, Hero.y + 5, 'Q', 0x04);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 10, Hero.y + 5, 'Q', 0x06);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 11, Hero.y + 5, 'Q', 0x06);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 12, Hero.y + 5, 'Q', 0x06);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 13, Hero.y + 5, 'Q', 0x06);

			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 0, Hero.y + 6, 'Q', 0x0e);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 1, Hero.y + 6, 'Q', 0x0e);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 2, Hero.y + 6, 'Q', 0x06);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 3, Hero.y + 6, 'Q', 0x06);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 4, Hero.y + 6, 'Q', 0x04);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 5, Hero.y + 6, 'Q', 0x04);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 6, Hero.y + 6, 'Q', 0x04);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 7, Hero.y + 6, 'Q', 0x04);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 8, Hero.y + 6, 'Q', 0x0e);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 9, Hero.y + 6, 'Q', 0x04);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 10, Hero.y + 6, 'Q', 0x06);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 11, Hero.y + 6, 'Q', 0x06);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 13, Hero.y + 6, 'Q', 0x0e);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 14, Hero.y + 6, 'Q', 0x0e);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 15, Hero.y + 6, 'Q', 0x0e);

			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 1, Hero.y + 7, 'Q', 0x06);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 4, Hero.y + 7, 'Q', 0x04);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 5, Hero.y + 7, 'Q', 0x04);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 6, Hero.y + 7, 'Q', 0x04);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 7, Hero.y + 7, 'Q', 0x04);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 8, Hero.y + 7, 'Q', 0x04);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 9, Hero.y + 7, 'Q', 0x04);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 10, Hero.y + 7, 'Q', 0x04);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 11, Hero.y + 7, 'Q', 0x04);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 14, Hero.y + 7, 'Q', 0x0e);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 15, Hero.y + 7, 'Q', 0x0e);

			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 1, Hero.y + 8, 'Q', 0x06);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 2, Hero.y + 8, 'Q', 0x06);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 3, Hero.y + 8, 'Q', 0x04);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 4, Hero.y + 8, 'Q', 0x04);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 5, Hero.y + 8, 'Q', 0x04);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 6, Hero.y + 8, 'Q', 0x04);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 10, Hero.y + 8, 'Q', 0x04);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 11, Hero.y + 8, 'Q', 0x04);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 12, Hero.y + 8, 'Q', 0x04);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 13, Hero.y + 8, 'Q', 0x06);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 14, Hero.y + 8, 'Q', 0x06);

			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 12, Hero.y + 9, 'Q', 0x06);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 13, Hero.y + 9, 'Q', 0x06);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 14, Hero.y + 9, 'Q', 0x06);
		}
	}
	else if (Hero.turn == -1)//վ������
	{
		Hero.state++;
		int ch;
		if (Hero.state % 5 != 0 || (ch = GetCommand()) == 0)//5֡����һ�α任�����Զ���
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

			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 7, Hero.y + 0, 'Q', 0x04);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 8, Hero.y + 0, 'Q', 0x04);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 9, Hero.y + 0, 'Q', 0x04);

			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 2, Hero.y + 1, 'Q', 0x04);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 3, Hero.y + 1, 'Q', 0x04);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 4, Hero.y + 1, 'Q', 0x04);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 5, Hero.y + 1, 'Q', 0x04);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 6, Hero.y + 1, 'Q', 0x04);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 7, Hero.y + 1, 'Q', 0x04);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 8, Hero.y + 1, 'Q', 0x04);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 9, Hero.y + 1, 'Q', 0x04);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 10, Hero.y + 1, 'Q', 0x04);

			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 4, Hero.y + 2, 'Q', 0x0e);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 5, Hero.y + 2, 'Q', 0x0e);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 6, Hero.y + 2, 'Q', 0x06);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 7, Hero.y + 2, 'Q', 0x0e);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 8, Hero.y + 2, 'Q', 0x0e);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 9, Hero.y + 2, 'Q', 0x0e);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 10, Hero.y + 2, 'Q', 0x06);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 11, Hero.y + 2, 'Q', 0x0e);

			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 2, Hero.y + 3, 'Q', 0x06);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 3, Hero.y + 3, 'Q', 0x0e);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 4, Hero.y + 3, 'Q', 0x0e);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 5, Hero.y + 3, 'Q', 0x06);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 6, Hero.y + 3, 'Q', 0x0e);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 7, Hero.y + 3, 'Q', 0x0e);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 8, Hero.y + 3, 'Q', 0x0e);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 9, Hero.y + 3, 'Q', 0x06);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 10, Hero.y + 3, 'Q', 0x06);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 11, Hero.y + 3, 'Q', 0x0e);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 12, Hero.y + 3, 'Q', 0x06);

			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 5, Hero.y + 4, 'Q', 0x06);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 6, Hero.y + 4, 'Q', 0x0e);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 7, Hero.y + 4, 'Q', 0x0e);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 8, Hero.y + 4, 'Q', 0x0e);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 9, Hero.y + 4, 'Q', 0x0e);

			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 3, Hero.y + 5, 'Q', 0x06);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 4, Hero.y + 5, 'Q', 0x06);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 5, Hero.y + 5, 'Q', 0x06);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 6, Hero.y + 5, 'Q', 0x04);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 7, Hero.y + 5, 'Q', 0x06);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 8, Hero.y + 5, 'Q', 0x06);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 9, Hero.y + 5, 'Q', 0x04);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 10, Hero.y + 5, 'Q', 0x06);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 11, Hero.y + 5, 'Q', 0x06);

			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 2, Hero.y + 6, 'Q', 0x06);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 3, Hero.y + 6, 'Q', 0x06);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 4, Hero.y + 6, 'Q', 0x06);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 5, Hero.y + 6, 'Q', 0x06);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 6, Hero.y + 6, 'Q', 0x04);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 7, Hero.y + 6, 'Q', 0x04);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 8, Hero.y + 6, 'Q', 0x04);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 9, Hero.y + 6, 'Q', 0x04);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 10, Hero.y + 6, 'Q', 0x06);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 11, Hero.y + 6, 'Q', 0x06);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 12, Hero.y + 6, 'Q', 0x06);

			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 2, Hero.y + 7, 'Q', 0x0e);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 3, Hero.y + 7, 'Q', 0x0e);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 4, Hero.y + 7, 'Q', 0x06);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 5, Hero.y + 7, 'Q', 0x04);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 6, Hero.y + 7, 'Q', 0x0e);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 7, Hero.y + 7, 'Q', 0x04);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 8, Hero.y + 7, 'Q', 0x04);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 9, Hero.y + 7, 'Q', 0x0e);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 10, Hero.y + 7, 'Q', 0x04);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 11, Hero.y + 7, 'Q', 0x06);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 12, Hero.y + 7, 'Q', 0x0e);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 13, Hero.y + 7, 'Q', 0x0e);

			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 4, Hero.y + 8, 'Q', 0x04);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 5, Hero.y + 8, 'Q', 0x04);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 6, Hero.y + 8, 'Q', 0x04);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 9, Hero.y + 8, 'Q', 0x04);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 10, Hero.y + 8, 'Q', 0x04);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 11, Hero.y + 8, 'Q', 0x04);

			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 2, Hero.y + 9, 'Q', 0x06);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 3, Hero.y + 9, 'Q', 0x06);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 4, Hero.y + 9, 'Q', 0x06);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 5, Hero.y + 9, 'Q', 0x06);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 10, Hero.y + 9, 'Q', 0x06);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 11, Hero.y + 9, 'Q', 0x06);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 12, Hero.y + 9, 'Q', 0x06);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 13, Hero.y + 9, 'Q', 0x06);
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

			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 8, Hero.y + 0, 'Q', 0x04);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 9, Hero.y + 0, 'Q', 0x04);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 10, Hero.y + 0, 'Q', 0x04);

			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 3, Hero.y + 1, 'Q', 0x04);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 4, Hero.y + 1, 'Q', 0x04);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 5, Hero.y + 1, 'Q', 0x04);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 6, Hero.y + 1, 'Q', 0x04);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 7, Hero.y + 1, 'Q', 0x04);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 8, Hero.y + 1, 'Q', 0x04);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 9, Hero.y + 1, 'Q', 0x04);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 10, Hero.y + 1, 'Q', 0x04);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 11, Hero.y + 1, 'Q', 0x04);

			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 5, Hero.y + 2, 'Q', 0x0e);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 6, Hero.y + 2, 'Q', 0x06);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 7, Hero.y + 2, 'Q', 0x0e);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 8, Hero.y + 2, 'Q', 0x0e);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 9, Hero.y + 2, 'Q', 0x06);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 10, Hero.y + 2, 'Q', 0x06);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 11, Hero.y + 2, 'Q', 0x06);

			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 3, Hero.y + 3, 'Q', 0x06);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 4, Hero.y + 3, 'Q', 0x0e);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 5, Hero.y + 3, 'Q', 0x0e);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 6, Hero.y + 3, 'Q', 0x06);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 7, Hero.y + 3, 'Q', 0x0e);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 8, Hero.y + 3, 'Q', 0x0e);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 9, Hero.y + 3, 'Q', 0x0e);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 10, Hero.y + 3, 'Q', 0x06);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 11, Hero.y + 3, 'Q', 0x0e);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 12, Hero.y + 3, 'Q', 0x06);

			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 6, Hero.y + 4, 'Q', 0x06);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 7, Hero.y + 4, 'Q', 0x06);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 8, Hero.y + 4, 'Q', 0x04);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 9, Hero.y + 4, 'Q', 0x06);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 10, Hero.y + 4, 'Q', 0x06);

			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 4, Hero.y + 5, 'Q', 0x06);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 5, Hero.y + 5, 'Q', 0x06);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 6, Hero.y + 5, 'Q', 0x04);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 7, Hero.y + 5, 'Q', 0x04);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 8, Hero.y + 5, 'Q', 0x06);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 9, Hero.y + 5, 'Q', 0x06);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 10, Hero.y + 5, 'Q', 0x06);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 11, Hero.y + 5, 'Q', 0x06);

			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 3, Hero.y + 6, 'Q', 0x04);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 4, Hero.y + 6, 'Q', 0x04);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 5, Hero.y + 6, 'Q', 0x04);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 6, Hero.y + 6, 'Q', 0x0e);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 7, Hero.y + 6, 'Q', 0x0e);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 8, Hero.y + 6, 'Q', 0x0e);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 9, Hero.y + 6, 'Q', 0x06);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 10, Hero.y + 6, 'Q', 0x06);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 11, Hero.y + 6, 'Q', 0x04);

			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 4, Hero.y + 7, 'Q', 0x04);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 5, Hero.y + 7, 'Q', 0x04);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 6, Hero.y + 7, 'Q', 0x04);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 7, Hero.y + 7, 'Q', 0x0e);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 8, Hero.y + 7, 'Q', 0x0e);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 9, Hero.y + 7, 'Q', 0x06);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 10, Hero.y + 7, 'Q', 0x04);

			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 4, Hero.y + 8, 'Q', 0x06);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 5, Hero.y + 8, 'Q', 0x06);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 6, Hero.y + 8, 'Q', 0x06);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 7, Hero.y + 8, 'Q', 0x04);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 8, Hero.y + 8, 'Q', 0x04);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 9, Hero.y + 8, 'Q', 0x04);

			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 3, Hero.y + 9, 'Q', 0x06);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 4, Hero.y + 9, 'Q', 0x06);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 5, Hero.y + 9, 'Q', 0x06);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 6, Hero.y + 9, 'Q', 0x06);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 7, Hero.y + 9, 'Q', 0x06);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 8, Hero.y + 9, 'Q', 0x06);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 9, Hero.y + 9, 'Q', 0x06);
		}

	}
	else //վ������
	{
		int ch;
		Hero.state++;
		if (Hero.turn == 0 || Hero.state % 5 != 0 || (ch = GetCommand()) == 0)//5֡����һ�α任�����Զ���
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
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 6, Hero.y + 0, 'Q', 0x04);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 7, Hero.y + 0, 'Q', 0x04);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 8, Hero.y + 0, 'Q', 0x04);

			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 5, Hero.y + 1, 'Q', 0x04);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 6, Hero.y + 1, 'Q', 0x04);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 7, Hero.y + 1, 'Q', 0x04);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 8, Hero.y + 1, 'Q', 0x04);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 9, Hero.y + 1, 'Q', 0x04);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 10, Hero.y + 1, 'Q', 0x04);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 11, Hero.y + 1, 'Q', 0x04);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 12, Hero.y + 1, 'Q', 0x04);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 13, Hero.y + 1, 'Q', 0x04);

			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 4, Hero.y + 2, 'Q', 0x0e);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 5, Hero.y + 2, 'Q', 0x06);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 6, Hero.y + 2, 'Q', 0x0e);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 7, Hero.y + 2, 'Q', 0x0e);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 8, Hero.y + 2, 'Q', 0x0e);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 9, Hero.y + 2, 'Q', 0x06);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 10, Hero.y + 2, 'Q', 0x0e);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 11, Hero.y + 2, 'Q', 0x0e);

			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 3, Hero.y + 3, 'Q', 0x06);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 4, Hero.y + 3, 'Q', 0x0e);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 5, Hero.y + 3, 'Q', 0x06);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 6, Hero.y + 3, 'Q', 0x06);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 7, Hero.y + 3, 'Q', 0x0e);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 8, Hero.y + 3, 'Q', 0x0e);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 9, Hero.y + 3, 'Q', 0x0e);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 10, Hero.y + 3, 'Q', 0x06);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 11, Hero.y + 3, 'Q', 0x0e);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 12, Hero.y + 3, 'Q', 0x0e);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 13, Hero.y + 3, 'Q', 0x06);

			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 6, Hero.y + 4, 'Q', 0x0e);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 7, Hero.y + 4, 'Q', 0x0e);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 8, Hero.y + 4, 'Q', 0x0e);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 9, Hero.y + 4, 'Q', 0x0e);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 10, Hero.y + 4, 'Q', 0x06);

			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 4, Hero.y + 5, 'Q', 0x06);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 5, Hero.y + 5, 'Q', 0x06);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 6, Hero.y + 5, 'Q', 0x04);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 7, Hero.y + 5, 'Q', 0x06);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 8, Hero.y + 5, 'Q', 0x06);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 9, Hero.y + 5, 'Q', 0x04);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 10, Hero.y + 5, 'Q', 0x06);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 11, Hero.y + 5, 'Q', 0x06);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 12, Hero.y + 5, 'Q', 0x06);

			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 3, Hero.y + 6, 'Q', 0x06);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 4, Hero.y + 6, 'Q', 0x06);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 5, Hero.y + 6, 'Q', 0x06);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 6, Hero.y + 6, 'Q', 0x04);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 7, Hero.y + 6, 'Q', 0x04);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 8, Hero.y + 6, 'Q', 0x04);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 9, Hero.y + 6, 'Q', 0x04);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 10, Hero.y + 6, 'Q', 0x06);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 11, Hero.y + 6, 'Q', 0x06);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 12, Hero.y + 6, 'Q', 0x06);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 13, Hero.y + 6, 'Q', 0x06);

			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 2, Hero.y + 7, 'Q', 0x0e);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 3, Hero.y + 7, 'Q', 0x0e);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 4, Hero.y + 7, 'Q', 0x06);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 5, Hero.y + 7, 'Q', 0x04);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 6, Hero.y + 7, 'Q', 0x0e);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 7, Hero.y + 7, 'Q', 0x04);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 8, Hero.y + 7, 'Q', 0x04);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 9, Hero.y + 7, 'Q', 0x0e);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 10, Hero.y + 7, 'Q', 0x04);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 11, Hero.y + 7, 'Q', 0x06);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 12, Hero.y + 7, 'Q', 0x0e);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 13, Hero.y + 7, 'Q', 0x0e);

			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 4, Hero.y + 8, 'Q', 0x04);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 5, Hero.y + 8, 'Q', 0x04);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 6, Hero.y + 8, 'Q', 0x04);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 9, Hero.y + 8, 'Q', 0x04);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 10, Hero.y + 8, 'Q', 0x04);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 11, Hero.y + 8, 'Q', 0x04);

			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 2, Hero.y + 9, 'Q', 0x06);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 3, Hero.y + 9, 'Q', 0x06);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 4, Hero.y + 9, 'Q', 0x06);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 5, Hero.y + 9, 'Q', 0x06);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 10, Hero.y + 9, 'Q', 0x06);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 11, Hero.y + 9, 'Q', 0x06);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 12, Hero.y + 9, 'Q', 0x06);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 13, Hero.y + 9, 'Q', 0x06);
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

			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 5, Hero.y + 0, 'Q', 0x04);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 6, Hero.y + 0, 'Q', 0x04);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 7, Hero.y + 0, 'Q', 0x04);

			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 4, Hero.y + 1, 'Q', 0x04);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 5, Hero.y + 1, 'Q', 0x04);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 6, Hero.y + 1, 'Q', 0x04);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 7, Hero.y + 1, 'Q', 0x04);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 8, Hero.y + 1, 'Q', 0x04);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 9, Hero.y + 1, 'Q', 0x04);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 10, Hero.y + 1, 'Q', 0x04);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 11, Hero.y + 1, 'Q', 0x04);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 12, Hero.y + 1, 'Q', 0x04);

			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 4, Hero.y + 2, 'Q', 0x06);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 5, Hero.y + 2, 'Q', 0x06);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 6, Hero.y + 2, 'Q', 0x06);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 7, Hero.y + 2, 'Q', 0x0e);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 8, Hero.y + 2, 'Q', 0x0e);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 9, Hero.y + 2, 'Q', 0x06);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 10, Hero.y + 2, 'Q', 0x0e);

			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 3, Hero.y + 3, 'Q', 0x06);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 4, Hero.y + 3, 'Q', 0x0e);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 5, Hero.y + 3, 'Q', 0x06);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 6, Hero.y + 3, 'Q', 0x0e);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 7, Hero.y + 3, 'Q', 0x0e);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 8, Hero.y + 3, 'Q', 0x0e);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 9, Hero.y + 3, 'Q', 0x06);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 10, Hero.y + 3, 'Q', 0x0e);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 11, Hero.y + 3, 'Q', 0x0e);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 12, Hero.y + 3, 'Q', 0x06);

			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 5, Hero.y + 4, 'Q', 0x06);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 6, Hero.y + 4, 'Q', 0x06);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 7, Hero.y + 4, 'Q', 0x04);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 8, Hero.y + 4, 'Q', 0x06);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 9, Hero.y + 4, 'Q', 0x06);

			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 4, Hero.y + 5, 'Q', 0x06);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 5, Hero.y + 5, 'Q', 0x06);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 6, Hero.y + 5, 'Q', 0x06);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 7, Hero.y + 5, 'Q', 0x06);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 8, Hero.y + 5, 'Q', 0x04);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 9, Hero.y + 5, 'Q', 0x04);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 10, Hero.y + 5, 'Q', 0x06);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 11, Hero.y + 5, 'Q', 0x06);

			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 4, Hero.y + 6, 'Q', 0x04);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 5, Hero.y + 6, 'Q', 0x06);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 6, Hero.y + 6, 'Q', 0x06);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 7, Hero.y + 6, 'Q', 0x0e);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 8, Hero.y + 6, 'Q', 0x0e);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 9, Hero.y + 6, 'Q', 0x0e);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 10, Hero.y + 6, 'Q', 0x04);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 11, Hero.y + 6, 'Q', 0x04);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 12, Hero.y + 6, 'Q', 0x04);

			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 5, Hero.y + 7, 'Q', 0x04);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 6, Hero.y + 7, 'Q', 0x06);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 7, Hero.y + 7, 'Q', 0x0e);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 8, Hero.y + 7, 'Q', 0x0e);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 9, Hero.y + 7, 'Q', 0x04);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 10, Hero.y + 7, 'Q', 0x04);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 11, Hero.y + 7, 'Q', 0x04);

			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 6, Hero.y + 8, 'Q', 0x04);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 7, Hero.y + 8, 'Q', 0x04);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 8, Hero.y + 8, 'Q', 0x04);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 9, Hero.y + 8, 'Q', 0x06);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 10, Hero.y + 8, 'Q', 0x06);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 11, Hero.y + 8, 'Q', 0x06);

			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 6, Hero.y + 9, 'Q', 0x06);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 7, Hero.y + 9, 'Q', 0x06);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 8, Hero.y + 9, 'Q', 0x06);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 9, Hero.y + 9, 'Q', 0x06);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 10, Hero.y + 9, 'Q', 0x06);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 11, Hero.y + 9, 'Q', 0x06);
			gotoxyAndPutchar1(MX / 2 - centerLine + Hero.x + 12, Hero.y + 9, 'Q', 0x06);

		}
	}

}

//��ʾ����
void Duel::showEnemy() //��ʾ����
{
	if (Enemy.turn == 1)
	{
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 5), Enemy.y + 0, 'Q', 0x04);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 6), Enemy.y + 0, 'Q', 0x04);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 7), Enemy.y + 0, 'Q', 0x04);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 8), Enemy.y + 0, 'Q', 0x04);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 9), Enemy.y + 0, 'Q', 0x04);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 10), Enemy.y + 0, 'Q', 0x04);

		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 4), Enemy.y + 1, 'Q', 0x04);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 5), Enemy.y + 1, 'Q', 0x04);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 6), Enemy.y + 1, 'Q', 0x04);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 7), Enemy.y + 1, 'Q', 0x04);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 8), Enemy.y + 1, 'Q', 0x04);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 9), Enemy.y + 1, 'Q', 0x04);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 10), Enemy.y + 1, 'Q', 0x04);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 11), Enemy.y + 1, 'Q', 0x04);

		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 2), Enemy.y + 2, 'Q', 0x04);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 3), Enemy.y + 2, 'Q', 0x06);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 4), Enemy.y + 2, 'Q', 0x06);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 5), Enemy.y + 2, 'Q', 0x04);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 6), Enemy.y + 2, 'Q', 0x04);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 7), Enemy.y + 2, 'Q', 0x04);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 8), Enemy.y + 2, 'Q', 0x04);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 9), Enemy.y + 2, 'Q', 0x04);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 10), Enemy.y + 2, 'Q', 0x04);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 11), Enemy.y + 2, 'Q', 0x06);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 12), Enemy.y + 2, 'Q', 0x06);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 13), Enemy.y + 2, 'Q', 0x04);

		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 1), Enemy.y + 3, 'Q', 0x04);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 2), Enemy.y + 3, 'Q', 0x04);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 3), Enemy.y + 3, 'Q', 0x04);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 4), Enemy.y + 3, 'Q', 0x0e);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 5), Enemy.y + 3, 'Q', 0x06);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 6), Enemy.y + 3, 'Q', 0x06);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 7), Enemy.y + 3, 'Q', 0x06);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 8), Enemy.y + 3, 'Q', 0x06);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 9), Enemy.y + 3, 'Q', 0x06);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 10), Enemy.y + 3, 'Q', 0x06);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 11), Enemy.y + 3, 'Q', 0x0e);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 12), Enemy.y + 3, 'Q', 0x04);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 13), Enemy.y + 3, 'Q', 0x04);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 14), Enemy.y + 3, 'Q', 0x04);

		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 0), Enemy.y + 4, 'Q', 0x04);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 1), Enemy.y + 4, 'Q', 0x04);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 2), Enemy.y + 4, 'Q', 0x04);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 3), Enemy.y + 4, 'Q', 0x04);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 4), Enemy.y + 4, 'Q', 0x0e);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 5), Enemy.y + 4, 'Q', 0x06);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 6), Enemy.y + 4, 'Q', 0x0e);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 7), Enemy.y + 4, 'Q', 0x04);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 8), Enemy.y + 4, 'Q', 0x04);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 9), Enemy.y + 4, 'Q', 0x0e);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 10), Enemy.y + 4, 'Q', 0x06);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 11), Enemy.y + 4, 'Q', 0x0e);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 12), Enemy.y + 4, 'Q', 0x04);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 13), Enemy.y + 4, 'Q', 0x04);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 14), Enemy.y + 4, 'Q', 0x04);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 15), Enemy.y + 4, 'Q', 0x04);

		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 0), Enemy.y + 5, 'Q', 0x04);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 1), Enemy.y + 5, 'Q', 0x04);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 2), Enemy.y + 5, 'Q', 0x04);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 3), Enemy.y + 5, 'Q', 0x04);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 4), Enemy.y + 5, 'Q', 0x0e);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 5), Enemy.y + 5, 'Q', 0x0e);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 6), Enemy.y + 5, 'Q', 0x0e);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 7), Enemy.y + 5, 'Q', 0x04);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 8), Enemy.y + 5, 'Q', 0x04);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 9), Enemy.y + 5, 'Q', 0x0e);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 10), Enemy.y + 5, 'Q', 0x0e);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 11), Enemy.y + 5, 'Q', 0x0e);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 12), Enemy.y + 5, 'Q', 0x04);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 13), Enemy.y + 5, 'Q', 0x04);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 14), Enemy.y + 5, 'Q', 0x04);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 15), Enemy.y + 5, 'Q', 0x04);

		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 0), Enemy.y + 6, 'Q', 0x04);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 1), Enemy.y + 6, 'Q', 0x04);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 2), Enemy.y + 6, 'Q', 0x04);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 3), Enemy.y + 6, 'Q', 0x04);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 4), Enemy.y + 6, 'Q', 0x04);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 5), Enemy.y + 6, 'Q', 0x04);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 6), Enemy.y + 6, 'Q', 0x04);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 7), Enemy.y + 6, 'Q', 0x04);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 8), Enemy.y + 6, 'Q', 0x04);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 9), Enemy.y + 6, 'Q', 0x04);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 10), Enemy.y + 6, 'Q', 0x04);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 11), Enemy.y + 6, 'Q', 0x04);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 12), Enemy.y + 6, 'Q', 0x04);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 13), Enemy.y + 6, 'Q', 0x04);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 14), Enemy.y + 6, 'Q', 0x04);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 15), Enemy.y + 6, 'Q', 0x04);

		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 1), Enemy.y + 7, 'Q', 0x04);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 2), Enemy.y + 7, 'Q', 0x04);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 3), Enemy.y + 7, 'Q', 0x04);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 4), Enemy.y + 7, 'Q', 0x04);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 5), Enemy.y + 7, 'Q', 0x0e);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 6), Enemy.y + 7, 'Q', 0x0e);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 7), Enemy.y + 7, 'Q', 0x0e);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 8), Enemy.y + 7, 'Q', 0x0e);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 9), Enemy.y + 7, 'Q', 0x0e);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 10), Enemy.y + 7, 'Q', 0x0e);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 11), Enemy.y + 7, 'Q', 0x04);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 12), Enemy.y + 7, 'Q', 0x04);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 13), Enemy.y + 7, 'Q', 0x04);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 14), Enemy.y + 7, 'Q', 0x04);

		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 2), Enemy.y + 8, 'Q', 0x06);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 3), Enemy.y + 8, 'Q', 0x06);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 4), Enemy.y + 8, 'Q', 0x0e);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 5), Enemy.y + 8, 'Q', 0x0e);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 6), Enemy.y + 8, 'Q', 0x0e);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 7), Enemy.y + 8, 'Q', 0x0e);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 8), Enemy.y + 8, 'Q', 0x0e);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 9), Enemy.y + 8, 'Q', 0x0e);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 10), Enemy.y + 8, 'Q', 0x0e);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 11), Enemy.y + 8, 'Q', 0x0e);

		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 1), Enemy.y + 9, 'Q', 0x06);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 2), Enemy.y + 9, 'Q', 0x06);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 3), Enemy.y + 9, 'Q', 0x06);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 4), Enemy.y + 9, 'Q', 0x06);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 5), Enemy.y + 9, 'Q', 0x06);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 6), Enemy.y + 9, 'Q', 0x06);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 10), Enemy.y + 9, 'Q', 0x06);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 11), Enemy.y + 9, 'Q', 0x06);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 12), Enemy.y + 9, 'Q', 0x06);
	}
	else
	{

		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 5), Enemy.y + 0, 'Q', 0x04);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 6), Enemy.y + 0, 'Q', 0x04);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 7), Enemy.y + 0, 'Q', 0x04);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 8), Enemy.y + 0, 'Q', 0x04);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 9), Enemy.y + 0, 'Q', 0x04);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 10), Enemy.y + 0, 'Q', 0x04);

		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 4), Enemy.y + 1, 'Q', 0x04);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 5), Enemy.y + 1, 'Q', 0x04);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 6), Enemy.y + 1, 'Q', 0x04);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 7), Enemy.y + 1, 'Q', 0x04);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 8), Enemy.y + 1, 'Q', 0x04);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 9), Enemy.y + 1, 'Q', 0x04);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 10), Enemy.y + 1, 'Q', 0x04);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 11), Enemy.y + 1, 'Q', 0x04);

		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 2), Enemy.y + 2, 'Q', 0x04);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 3), Enemy.y + 2, 'Q', 0x06);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 4), Enemy.y + 2, 'Q', 0x06);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 5), Enemy.y + 2, 'Q', 0x04);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 6), Enemy.y + 2, 'Q', 0x04);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 7), Enemy.y + 2, 'Q', 0x04);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 8), Enemy.y + 2, 'Q', 0x04);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 9), Enemy.y + 2, 'Q', 0x04);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 10), Enemy.y + 2, 'Q', 0x04);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 11), Enemy.y + 2, 'Q', 0x06);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 12), Enemy.y + 2, 'Q', 0x06);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 13), Enemy.y + 2, 'Q', 0x04);

		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 1), Enemy.y + 3, 'Q', 0x04);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 2), Enemy.y + 3, 'Q', 0x04);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 3), Enemy.y + 3, 'Q', 0x04);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 4), Enemy.y + 3, 'Q', 0x0e);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 5), Enemy.y + 3, 'Q', 0x06);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 6), Enemy.y + 3, 'Q', 0x06);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 7), Enemy.y + 3, 'Q', 0x06);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 8), Enemy.y + 3, 'Q', 0x06);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 9), Enemy.y + 3, 'Q', 0x06);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 10), Enemy.y + 3, 'Q', 0x06);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 11), Enemy.y + 3, 'Q', 0x0e);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 12), Enemy.y + 3, 'Q', 0x04);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 13), Enemy.y + 3, 'Q', 0x04);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 14), Enemy.y + 3, 'Q', 0x04);

		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 0), Enemy.y + 4, 'Q', 0x04);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 1), Enemy.y + 4, 'Q', 0x04);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 2), Enemy.y + 4, 'Q', 0x04);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 3), Enemy.y + 4, 'Q', 0x04);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 4), Enemy.y + 4, 'Q', 0x0e);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 5), Enemy.y + 4, 'Q', 0x06);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 6), Enemy.y + 4, 'Q', 0x0e);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 7), Enemy.y + 4, 'Q', 0x04);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 8), Enemy.y + 4, 'Q', 0x04);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 9), Enemy.y + 4, 'Q', 0x0e);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 10), Enemy.y + 4, 'Q', 0x06);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 11), Enemy.y + 4, 'Q', 0x0e);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 12), Enemy.y + 4, 'Q', 0x04);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 13), Enemy.y + 4, 'Q', 0x04);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 14), Enemy.y + 4, 'Q', 0x04);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 15), Enemy.y + 4, 'Q', 0x04);

		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 0), Enemy.y + 5, 'Q', 0x04);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 1), Enemy.y + 5, 'Q', 0x04);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 2), Enemy.y + 5, 'Q', 0x04);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 3), Enemy.y + 5, 'Q', 0x04);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 4), Enemy.y + 5, 'Q', 0x0e);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 5), Enemy.y + 5, 'Q', 0x0e);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 6), Enemy.y + 5, 'Q', 0x0e);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 7), Enemy.y + 5, 'Q', 0x04);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 8), Enemy.y + 5, 'Q', 0x04);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 9), Enemy.y + 5, 'Q', 0x0e);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 10), Enemy.y + 5, 'Q', 0x0e);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 11), Enemy.y + 5, 'Q', 0x0e);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 12), Enemy.y + 5, 'Q', 0x04);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 13), Enemy.y + 5, 'Q', 0x04);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 14), Enemy.y + 5, 'Q', 0x04);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 15), Enemy.y + 5, 'Q', 0x04);

		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 0), Enemy.y + 6, 'Q', 0x04);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 1), Enemy.y + 6, 'Q', 0x04);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 2), Enemy.y + 6, 'Q', 0x04);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 3), Enemy.y + 6, 'Q', 0x04);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 4), Enemy.y + 6, 'Q', 0x04);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 5), Enemy.y + 6, 'Q', 0x04);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 6), Enemy.y + 6, 'Q', 0x04);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 7), Enemy.y + 6, 'Q', 0x04);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 8), Enemy.y + 6, 'Q', 0x04);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 9), Enemy.y + 6, 'Q', 0x04);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 10), Enemy.y + 6, 'Q', 0x04);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 11), Enemy.y + 6, 'Q', 0x04);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 12), Enemy.y + 6, 'Q', 0x04);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 13), Enemy.y + 6, 'Q', 0x04);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 14), Enemy.y + 6, 'Q', 0x04);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 15), Enemy.y + 6, 'Q', 0x04);

		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 1), Enemy.y + 7, 'Q', 0x04);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 2), Enemy.y + 7, 'Q', 0x04);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 3), Enemy.y + 7, 'Q', 0x04);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 4), Enemy.y + 7, 'Q', 0x04);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 5), Enemy.y + 7, 'Q', 0x0e);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 6), Enemy.y + 7, 'Q', 0x0e);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 7), Enemy.y + 7, 'Q', 0x0e);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 8), Enemy.y + 7, 'Q', 0x0e);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 9), Enemy.y + 7, 'Q', 0x0e);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 10), Enemy.y + 7, 'Q', 0x0e);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 11), Enemy.y + 7, 'Q', 0x04);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 12), Enemy.y + 7, 'Q', 0x04);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 13), Enemy.y + 7, 'Q', 0x04);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 14), Enemy.y + 7, 'Q', 0x04);

		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 4), Enemy.y + 8, 'Q', 0x0e);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 5), Enemy.y + 8, 'Q', 0x0e);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 6), Enemy.y + 8, 'Q', 0x0e);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 7), Enemy.y + 8, 'Q', 0x0e);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 8), Enemy.y + 8, 'Q', 0x0e);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 9), Enemy.y + 8, 'Q', 0x0e);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 10), Enemy.y + 8, 'Q', 0x0e);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 11), Enemy.y + 8, 'Q', 0x0e);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 12), Enemy.y + 8, 'Q', 0x06);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 13), Enemy.y + 8, 'Q', 0x06);

		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 3), Enemy.y + 9, 'Q', 0x06);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 4), Enemy.y + 9, 'Q', 0x06);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 5), Enemy.y + 9, 'Q', 0x06);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 9), Enemy.y + 9, 'Q', 0x06);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 10), Enemy.y + 9, 'Q', 0x06);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 11), Enemy.y + 9, 'Q', 0x06);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 12), Enemy.y + 9, 'Q', 0x06);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 13), Enemy.y + 9, 'Q', 0x06);
		gotoxyAndPutchar1(MX / 2 - (centerLine - Enemy.x - 14), Enemy.y + 9, 'Q', 0x06);
	}
}

//��ʾ����
void Duel::showLife()
{

	gotoxy1(0, 1);
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 0x0c);
	for (int i = 0; i < Hero.hp; i++)
		printf_s("*");

	gotoxy1(260 - 1 - Enemy.hp, 1);
	for (int i = 0; i < Enemy.hp; i++)
		printf_s("*");
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 0x07);
}

//�����׼�������еĻ��浽��ʾ�Ļ�����
void Duel::copyToBuffer(HANDLE SHandle, int STop, int SLeft, int SBottom, int SRight, HANDLE DHandle, int DTop, int DLeft, int DBottom, int DRight)
{

	// ����Դ���� 
	SMALL_RECT srctReadRect;    //��Դ��������ȡ�ķ���
	srctReadRect.Top = STop;
	srctReadRect.Left = SLeft;
	srctReadRect.Bottom = SBottom - 1;
	srctReadRect.Right = SRight - 1;

	//������ʱ���������Ĵ�С�������У�������
	COORD coordBufSize;
	coordBufSize.Y = SBottom - STop;
	coordBufSize.X = SRight - SLeft;


	//������ʱ���忪ʼװ��Ŀ�ʼλ��
	COORD coordBufCoord;
	coordBufCoord.X = 0;
	coordBufCoord.Y = 0;

	//����Ļ�������顱����ʱ���������
	ReadConsoleOutput(
		SHandle,        // screen buffer to read from 
		chiBuffer1,      // buffer to copy into 
		coordBufSize,   // col-row size of chiBuffer1 
		coordBufCoord,  // top left dest. cell in chiBuffer1 
		&srctReadRect); // screen buffer source rectangle 



						// ����Ŀ�귽��  
	SMALL_RECT srctWriteRect;    //д���»������ķ���
	srctWriteRect.Top = DTop;
	srctWriteRect.Left = DLeft;
	srctWriteRect.Bottom = DBottom - 1;
	srctWriteRect.Right = DRight - 1;

	//����ʱ��������鿽�����顱����ĻĿ�껺����
	WriteConsoleOutput(
		DHandle,           // screen buffer to write to 
		chiBuffer1,        // buffer to copy from 
		coordBufSize,     // col-row size of chiBuffer1 
		coordBufCoord,    // top left src cell in chiBuffer1 
		&srctWriteRect);  // dest. screen buffer rectangle 

}

//�������Ǻ͵����ƶ�
void Duel::control()
{

	int ch;                          //�����ȡ������ֵ
	int k = 0 , enemyk = 0;          //k  n ��� �����ӵ�����Ƶ��
	int n = 0 , enemyn = 0;


	clock_t  t1, t2;
	t1 = clock();
	while (1)
	{
		//Sleep(20);//������Ϸ�Ľ����ٶ�
		if (GameState == 30)//��������
			return;
		if (GameState == -1)
			return;

		Hero.t = sqrt(2 * JH / G) / 10; //ģ������
		Enemy.t = sqrt(2 * JH / G) / 10;
		
		k++;
		if (k == 1000)
			k = 0;
		if (_kbhit())
		{
			ch = GetCommand();
			if (ch&CMD_LEFT)
				left(Hero);
			if (ch&CMD_RIGHT)
				right(Hero);
			if ((ch&CMD_UP))
				up(Hero);
			if (ch&CMD_DOWN)
				Hero.turn = 0;
			if (ch&CMD_ESC)
			{
				DuelPause();
			}
			if (ch & CMD_SHOOT)
			{
				if (n == 0)//��һ�ΰ���ֱ�ӷ���
				{
					shoot();
					n = 1;
				}
				n++;
				if (k % 10 == 0 && n > 10)//����ǳ�����������һ��Ƶ�ʷ���
				{
					shoot();
				}
			}
			else//���ǳ���
				n = 0;
		}


		enemyk++;
		if (enemyk == 1000)
			enemyk = 0;
		if (_kbhit())
		{
			ch = GetAICommand();
			if (ch&CMD_LEFT)
				left(Enemy);
			if (ch&CMD_RIGHT)
				right(Enemy);
			if ((ch&CMD_UP))
				up(Enemy);
			if (ch&CMD_DOWN)
				Enemy.turn = 0;
			if (ch&CMD_ESC)
			{
				DuelPause();
			}
			if (ch & CMD_SHOOT)
			{
				if (enemyn == 0)//��һ�ΰ���ֱ�ӷ���
				{
					EnemyShoot();
					enemyn = 1;
				}
				enemyn++;
				if (enemyk % 10 == 0 && enemyn > 10)//����ǳ�����������һ��Ƶ�ʷ���
				{
					EnemyShoot();
				}
			}
			else//���ǳ���
				enemyn = 0;
		}

		judge();


		
		show();

		do                     //������Ϸ�Ľ����ٶ�
		{
			t2 = clock();
		} while (t2 - t1 < 50);

		t1 = clock();

	}

	return;
}

//��ȡ�û�������Ϣ
int Duel::GetCommand()
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

//��ȡAI������Ϣ
int Duel::GetAICommand()
{
	int temp = 0;
	
	if (Hero.x < Enemy.x)
	{
		if ( Enemy.x - Hero.x > 75)
		{
			temp |= CMD_LEFT;
		}
		if ((Enemy.x - Hero.x < 100 && rand() % 100 <= 10) || rand() % 100 <= 25)
			temp |= CMD_LEFT;
	}
	 if (Hero.x > Enemy.x)
	{
		if ( Hero.x - Enemy.x  > 75)
		{
			temp |= CMD_RIGHT;
		}
		if ((Hero.x - Enemy.x < 100 && rand() % 100 <= 10) || rand() % 100 <= 25)
			temp |= CMD_RIGHT;
	}
	if((Hero.y < Enemy.y && HeroisLanded(Hero) && rand() % 15 == 0) || rand() % 100 <= 10)
		temp |= CMD_UP;

	if (rand() % 10 == 0)
		temp |= CMD_SHOOT;
	
	return temp;
	
}

//��Ծ
void Duel::up(Person & op)
{
	if (op.jump != 0)
		return;
	else
	{
		mciSendString("play jump from 0", NULL, 0, NULL);
		op.v0 = -sqrt(2 * G * JH);
		op.jump = 1;
	}
}

//�����˶�
void Duel::left(Person & op)
{
	op.x -= STEP;
	op.turn = -1;
	if (HeroisHitWall())
	{
		op.x += STEP;
	}
}

//�����˶�
void Duel::right(Person & op)
{
	op.x += STEP;
	op.turn = 1;
	if (HeroisHitWall())
	{
		op.x -= STEP;
	}
}

//���ǿ�ǹ
void Duel::shoot()
{
	mciSendString("play bullet from 0 ", NULL, 0, NULL);
	if (Hero.y + HY / 2 >= 0 && Hero.y + HY / 2 < MY)
	{
		Hero.bullet[Hero.n_bullet].id = 1;
		if (Hero.turn == -1)
		{
			Hero.bullet[Hero.n_bullet].x = Hero.x - 1;
			Hero.bullet[Hero.n_bullet].xLeftMost = Hero.bullet[Hero.n_bullet].x - 120;//���
			Hero.bullet[Hero.n_bullet].turn = -1;
		}
		else
		{
			Hero.bullet[Hero.n_bullet].x = Hero.x + HX;
			Hero.bullet[Hero.n_bullet].xRightMost = Hero.bullet[Hero.n_bullet].x + 120;//���
			Hero.bullet[Hero.n_bullet].turn = 1;
		}
		Hero.bullet[Hero.n_bullet].y = Hero.y + HY / 2;
		if (Hero.n_bullet >= 19)
		{
			Hero.n_bullet = 0;
		}
		else
		{
			Hero.n_bullet++;
		}
	}
}

//���˿�ǹ
void Duel::EnemyShoot()
{
	mciSendString("play Sbullet from 0 ", NULL, 0, NULL);
	if (Enemy.y + HY / 2 >= 0 && Enemy.y + HY / 2 < MY)
	{
		Enemy.bullet[Enemy.n_bullet].id = 1;
		if (Enemy.turn == -1)
		{
			Enemy.bullet[Enemy.n_bullet].x = Enemy.x - 1;
			Enemy.bullet[Enemy.n_bullet].xLeftMost = Enemy.bullet[Enemy.n_bullet].x - 100;//���
			Enemy.bullet[Enemy.n_bullet].turn = -1;
		}
		else
		{
			Enemy.bullet[Enemy.n_bullet].x = Enemy.x + HX;
			Enemy.bullet[Enemy.n_bullet].xRightMost = Enemy.bullet[Enemy.n_bullet].x + 100;//���
			Enemy.bullet[Enemy.n_bullet].turn = 1;
		}
		Enemy.bullet[Enemy.n_bullet].y = Enemy.y + HY / 2;
		if (Enemy.n_bullet >= 19)
		{
			Enemy.n_bullet = 0;
		}
		else
		{
			Enemy.n_bullet++;
		}
	}
}

//�ӵ�ײ������
bool Duel::BulletHit()
{
	bool hit = false;

//�����ӵ�
	for (int i = 0; i < 20; i++)
	{
		//�������
		if (Hero.bullet[i].id != 0)
		{
			if (Hero.bullet[i].turn == -1)
			{
				if (Hero.bullet[i].x < Hero.bullet[i].xLeftMost)
					Hero.bullet[i].id = 0;
			}
			else
			{
				if (Hero.bullet[i].x > Hero.bullet[i].xRightMost)
					Hero.bullet[i].id = 0;
			}
		}


		//ǽ
		if (Hero.bullet[i].id != 0)
		{
			int k, m;
			if (Hero.bullet[i].turn == 1)
			{
				k = Hero.bullet[i].x - STEP - STEP / 2;
				m = Hero.bullet[i].x;
			}
			else
			{
				k = Hero.bullet[i].x;
				m = Hero.bullet[i].x + STEP + STEP / 2;

			}
			for (; k <= m; k++)
			{

				if (Hero.bullet[i].y < MY && BMap1[Hero.bullet[i].y][k] == '#')
				{
					hit = true;
					Hero.bullet[i].id = 0;
					mciSendString("play hit_wall from 0 ", NULL, 0, NULL);
				}
			}
		}

       // ����
		if (Hero.bullet[i].id != 0)
		{

			if ( (abs(Hero.bullet[i].y - (Enemy.y + HY / 2)) <= HY / 2 && abs(Hero.bullet[i].x - (Enemy.x + HX / 2)) <= HX / 2))
			{
				Hero.bullet[i].id = 0;

				mciSendString("play hit_enemy from 0 ", NULL, 0, NULL);

				Enemy.hp -= 1;
			}

		}
	}


//�����ӵ�
	for (int i = 0; i < 20; i++)
	{
		//�������
		if (Enemy.bullet[i].id != 0)
		{
			if (Enemy.bullet[i].turn == -1)
			{
				if (Enemy.bullet[i].x < Enemy.bullet[i].xLeftMost)
					Enemy.bullet[i].id = 0;
			}
			else
			{
				if (Enemy.bullet[i].x > Enemy.bullet[i].xRightMost)
					Enemy.bullet[i].id = 0;
			}
		}


		//ǽ
		if (Enemy.bullet[i].id != 0)
		{
			int k, m;
			if (Enemy.bullet[i].turn == 1)
			{
				k = Enemy.bullet[i].x - STEP - STEP / 2;
				m = Enemy.bullet[i].x;
			}
			else
			{
				k = Enemy.bullet[i].x;
				m = Enemy.bullet[i].x + STEP + STEP / 2;

			}
			for (; k <= m; k++)
			{

				if (Enemy.bullet[i].y < MY && BMap1[Enemy.bullet[i].y][k] == '#')
				{
					hit = true;
					Enemy.bullet[i].id = 0;
					mciSendString("play hit_wall from 0 ", NULL, 0, NULL);
				}
			}
		}

		// ���˵ĵ��� -- ����
		if (Enemy.bullet[i].id != 0)
		{

			if ((abs(Enemy.bullet[i].y - (Hero.y + HY / 2)) <= HY / 2 && abs(Enemy.bullet[i].x - (Hero.x + HX / 2)) <= HX / 2))
			{
				Enemy.bullet[i].id = 0;

				mciSendString("play Shit_enemy from 0 ", NULL, 0, NULL);

				Hero.hp -= 2;
			}

		}
	}


	return hit;
}

//��������ʱײ��ͷ
bool Duel::HeroUpHitWall()
{
	bool result = false;
	for (int i = Hero.y; i < Hero.y + HY; i++)
	{
		for (int j = Hero.x; j < Hero.x + HX; j++)
		{
			if (i <= MY && i >= 0 && BMap1[i][j] == '#')
			{

				//
				BMap1[i][j] = ' ';
				if (!result)
				{
					result = true;
					mciSendString("play hit_brick from 0", NULL, 0, NULL);
				}
			}
		}
	}
	return result;

}

//��������ʱײ��ͷ
bool Duel::EnemyUpHitWall()
{
	bool result = false;
	for (int i = Enemy.y; i < Enemy.y + HY; i++)
	{
		for (int j = Enemy.x; j < Enemy.x + HX; j++)
		{
			if (i <= MY && i >= 0 && BMap1[i][j] == '#')
			{

				//
				BMap1[i][j] = ' ';
				if (!result)
				{
					result = true;
					mciSendString("play hit_brick from 0", NULL, 0, NULL);
				}
			}
		}
	}
	return result;

}

//��Ϸ��ͣ
void Duel::DuelPause()
{

	mciSendString("stop   bgm ", NULL, 0, NULL);
	while (_kbhit())
		char c = _getch();

	ifstream fp(".\\res\\pause.txt");                                                        //�����ͣ����
	if (!fp)
	{
		cerr << "open pause.txt error" << endl;
		exit(1);
	}
	char tempStr[300];
	int i = 0;
	while (!fp.eof())
	{
		gotoxy1(0, i++);
		fp.getline(tempStr, 300);
		puts(tempStr);
	}
	fp.close();
	int q;     //�����ƶ����ʱ��λ
	int t = 1;  //ѡ��1 CONTINUE   2 EXIT
	while (1)
	{

		if (_kbhit())
		{
			char ch = getch();
			if (ch == 13)//ENTER
			{
				if (t == 1)
				{

					mciSendString("play bgm", NULL, 0, NULL);
					break;
				}
				else if (t == -1)
				{
					exit(0);
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
					gotoxy1(80, q++);
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
				gotoxy1(80, q++);
				printf("         H");
				gotoxy1(80, q++);
				printf("         HH");
				gotoxy1(80, q++);
				printf("         HHH");
				gotoxy1(80, q++);
				printf("HHHHHHHHHHHHH");
				gotoxy1(80, q++);
				printf("HHHHHHHHHHHHH");
				gotoxy1(80, q++);
				printf("         HHH");
				gotoxy1(80, q++);
				printf("         HH");
				gotoxy1(80, q++);
				printf("         H");
			}
		}

		copyToBuffer(hStdout1, 0, 0, MY, 260, hNewScreenBuffer1, 0, 0, MY, 260);

		Sleep(25);
	}



}

//վ�ڵ���
bool Duel::HeroisLanded(Person & Hero)
{

	int lines = -1;
	for (int i = Hero.y; i <= Hero.y + HY; i++)
	{
		for (int j = Hero.x; j < Hero.x + HX; j++)
		{
			if (i <= MY && i >= 0 && BMap1[i][j] == '#')
			{
				lines = i - Hero.y;
				break;
			}
		}
		if (lines != -1)
			break;
	}
	if (lines != -1)
	{
		Hero.y -= (HY - lines);
		return true;
	}
	return false;

}

//����ײǽ
bool Duel::HeroisHitWall()
{
	for (int i = Hero.y; i < Hero.y + HY; i++)
	{
		for (int j = Hero.x; j <= Hero.x + HX; j++)
		{
			if ((i < MY) && i >= 0 && (BMap1[i][j] == '#'))
			{
				mciSendString("play hit_wall from 0", NULL, 0, NULL);
				return true;
			}
		}
	}
	return false;
}

//���� �ж�
void  Duel::judge()
{


	//�ƶ��ӵ�
	for (int i = 0; i < 20; i++)
	{
		if (Hero.bullet[i].id != 0)
		{
			Hero.bullet[i].x += Hero.bullet[i].turn * STEP * 3 / 2;
		}

		if (Enemy.bullet[i].id != 0)
		{
			Enemy.bullet[i].x += Enemy.bullet[i].turn * STEP * 3 / 2;
		}
	}

	BulletHit();


	if (Hero.jump != 0)//�ڿ�
	{
		Hero.h = Hero.v0*Hero.t + G*pow(Hero.t, 2) / 2;

		Hero.y += (int)(Hero.h + 0.5);//��������
		if (Hero.v0 >= 0)   //��������
		{

		}
		else       //������Ծ
		{
			if (HeroUpHitWall())
			{
				Hero.v0 = 0;
				Hero.h = 0;
				Hero.y = Hero.y / HY*HY + HY;
			}
		}
		Hero.v0 = Hero.v0 + G * Hero.t;
		if (HeroisLanded(Hero))
		{
			Hero.jump = 0;
		}
		if (HeroUpHitWall())
		{
			Hero.jump = -Hero.jump;
			Hero.y++;
		}

	}
	if ((!HeroisLanded(Hero)) && Hero.jump == 0)
	{
		Hero.jump = -1;
	}


	if (Enemy.jump != 0)//�ڿ�
	{
		Enemy.h = Enemy.v0*Enemy.t + G*pow(Enemy.t, 2) / 2;

		Enemy.y += (int)(Enemy.h + 0.5);//��������
		if (Enemy.v0 >= 0)   //��������
		{

		}
		else       //������Ծ
		{

			if (EnemyUpHitWall())
			{
				Enemy.v0 = 0;
				Enemy.h = 0;
				Enemy.y = Enemy.y / HY*HY + HY;
			}
		}
		Enemy.v0 = Enemy.v0 + G * Enemy.t;
		if (HeroisLanded(Enemy))
		{
			Enemy.jump = 0;
		}
		if (EnemyUpHitWall())
		{
			Enemy.jump = -Enemy.jump;
			Enemy.y++;
		}

	}
	if ((!HeroisLanded(Enemy)) && Enemy.jump == 0)
	{
		Enemy.jump = -1;
	}


	if (isblood())
		Hero.hp += 10;


	if (Hero.hp <= 0 || Hero.y >= MY)
		dead();
	else if (Enemy.hp <= 0  || Enemy.y >= MY)
		win();
	
	if (GameState == 30)//��������
		return;

	if (GameState == -1)
		return;




}
//�Ƿ��������
bool Duel::isblood()
{
	int tempHerox = Hero.x + HX / 2;
	int tempHeroY = Hero.y + HY / 2;

	for (int i = 0; i < N_COIN; i++)
	{
		if (blood[i].id != 0)
		{
			if (abs(blood[i].x - tempHerox) <= HX / 2 && abs(blood[i].y - tempHeroY) <= HY / 2)
			{
				blood[i].id = 0;
				mciSendString("play coin from 0", NULL, 0, NULL);
				return true;
			}
		}
	}
	return false;

}

//������
void Duel::dead()
{


	mciSendString("stop   Bossbgm ", NULL, 0, NULL);
	mciSendString("play  dead1 from 0", NULL, 0, NULL);

	Sleep(5000);

	centerLine = MX / 2;

	Hero.id = 1;
	Hero.x = XX;																	   //�������λ��
	Hero.y = YY;
	Hero.turn = 1;                                                                     //���ﳯ��
	Hero.state = 0;
	Hero.jump = 0;                                                                     //����Ծ
	Hero.n_bullet = 0;
	Hero.v0 = 0;																				//Ӣ����Ծ�ĳ��ٶ�
	Hero.h = 0;																				//Ӣ����Ծ�ĸ߶�
	Hero.t = 0;																				//Ӣ����Ծ��ʱ��
	Hero.hp = 100;


	Enemy.id = 1;
	Enemy.x = XX + 200;																	   //�������λ��
	Enemy.y = YY;
	Enemy.turn = -1;                                                                     //���ﳯ��
	Enemy.state = 0;
	Enemy.jump = 0;                                                                     //����Ծ
	Enemy.n_bullet = 0;
	Enemy.v0 = 0;																			//������Ծ�ĳ��ٶ�
	Enemy.h = 0;																				//������Ծ�ĸ߶�
	Enemy.t = 0;																				//������Ծ��ʱ��
	Enemy.hp = 100;


	Sleep(5000);

	life--;

	ifstream fp(".\\res\\life.txt");                                                        //�����ʾ��������
	if (!fp)
	{
		cerr << "open life.txt error" << endl;
		exit(1);
	}
	char tempStr[300];
	int i = 0;
	while (!fp.eof())
	{
		gotoxy1(0, i++);
		fp.getline(tempStr, 300);
		puts(tempStr);
	}
	fp.close();


	switch (life)
	{
	case 0:
		i = 29;
		gotoxy1(203, i++);
		puts("                       *******                               "); gotoxy1(203, i++);
		puts("                    *************                            "); gotoxy1(203, i++);
		puts("                   ***************                           "); gotoxy1(203, i++);
		puts("                  *****************                          "); gotoxy1(203, i++);
		puts("                 ******      *******                         "); gotoxy1(203, i++);
		puts("                ******         ******                        "); gotoxy1(203, i++);
		puts("               ******           *****                        "); gotoxy1(203, i++);
		puts("               *****            ******                       "); gotoxy1(203, i++);
		puts("              *****              *****                       "); gotoxy1(203, i++);
		puts("              *****              *****                       "); gotoxy1(203, i++);
		puts("              *****               *****                      "); gotoxy1(203, i++);
		puts("             *****                *****                      "); gotoxy1(203, i++);
		puts("             *****                *****                      "); gotoxy1(203, i++);
		puts("             *****                *****                      "); gotoxy1(203, i++);
		puts("             *****                ******                     "); gotoxy1(203, i++);
		puts("            *****                  *****                     "); gotoxy1(203, i++);
		puts("            *****                  *****                     "); gotoxy1(203, i++);
		puts("            *****                  *****                     "); gotoxy1(203, i++);
		puts("            *****                  *****                     "); gotoxy1(203, i++);
		puts("            *****                  *****                     "); gotoxy1(203, i++);
		puts("            *****                  *****                     "); gotoxy1(203, i++);
		puts("            *****                  *****                     "); gotoxy1(203, i++);
		puts("            *****                  *****                     "); gotoxy1(203, i++);
		puts("            *****                  *****                     "); gotoxy1(203, i++);
		puts("            *****                  *****                     "); gotoxy1(203, i++);
		puts("            *****                  *****                     "); gotoxy1(203, i++);
		puts("            *****                  *****                     "); gotoxy1(203, i++);
		puts("            *****                  *****                     "); gotoxy1(203, i++);
		puts("            *****                  *****                     "); gotoxy1(203, i++);
		puts("            *****                  ****                      "); gotoxy1(203, i++);
		puts("             *****                *****                      "); gotoxy1(203, i++);
		puts("             *****                *****                      "); gotoxy1(203, i++);
		puts("             *****                *****                      "); gotoxy1(203, i++);
		puts("             *****                ****                       "); gotoxy1(203, i++);
		puts("              *****              *****                       "); gotoxy1(203, i++);
		puts("              *****              *****                       "); gotoxy1(203, i++);
		puts("              ******            *****                        "); gotoxy1(203, i++);
		puts("               ******           *****                        "); gotoxy1(203, i++);
		puts("               *******         *****                         "); gotoxy1(203, i++);
		puts("                *******      ******                          "); gotoxy1(203, i++);
		puts("                 *****************                           "); gotoxy1(203, i++);
		puts("                  ***************                            "); gotoxy1(203, i++);
		puts("                    ************                             "); gotoxy1(203, i++);
		puts("                      *******                                "); gotoxy1(203, i++);
		break;
	case 1:
		i = 29;
		gotoxy1(203, i++);
		puts("                          ***                                "); gotoxy1(203, i++);
		puts("                         ****                                "); gotoxy1(203, i++);
		puts("                        *****                                "); gotoxy1(203, i++);
		puts("                      *******                                "); gotoxy1(203, i++);
		puts("                    *********                                "); gotoxy1(203, i++);
		puts("                  ***********                                "); gotoxy1(203, i++);
		puts("                ******* *****                                "); gotoxy1(203, i++);
		puts("              ********  *****                                "); gotoxy1(203, i++);
		puts("              ******    *****                                "); gotoxy1(203, i++);
		puts("              ****      *****                                "); gotoxy1(203, i++);
		puts("              **        *****                                "); gotoxy1(203, i++);
		puts("                        *****                                "); gotoxy1(203, i++);
		puts("                        *****                                "); gotoxy1(203, i++);
		puts("                        *****                                "); gotoxy1(203, i++);
		puts("                        *****                                "); gotoxy1(203, i++);
		puts("                        *****                                "); gotoxy1(203, i++);
		puts("                        *****                                "); gotoxy1(203, i++);
		puts("                        *****                                "); gotoxy1(203, i++);
		puts("                        *****                                "); gotoxy1(203, i++);
		puts("                        *****                                "); gotoxy1(203, i++);
		puts("                        *****                                "); gotoxy1(203, i++);
		puts("                        *****                                "); gotoxy1(203, i++);
		puts("                        *****                                "); gotoxy1(203, i++);
		puts("                        *****                                "); gotoxy1(203, i++);
		puts("                        *****                                "); gotoxy1(203, i++);
		puts("                        *****                                "); gotoxy1(203, i++);
		puts("                        *****                                "); gotoxy1(203, i++);
		puts("                        *****                                "); gotoxy1(203, i++);
		puts("                        *****                                "); gotoxy1(203, i++);
		puts("                        *****                                "); gotoxy1(203, i++);
		puts("                        *****                                "); gotoxy1(203, i++);
		puts("                        *****                                "); gotoxy1(203, i++);
		puts("                        *****                                "); gotoxy1(203, i++);
		puts("                        *****                                "); gotoxy1(203, i++);
		puts("                        *****                                "); gotoxy1(203, i++);
		puts("                        *****                                "); gotoxy1(203, i++);
		puts("                        *****                                "); gotoxy1(203, i++);
		puts("                        *****                                "); gotoxy1(203, i++);
		puts("                        *****                                "); gotoxy1(203, i++);
		puts("                        *****                                "); gotoxy1(203, i++);
		puts("                        *****                                "); gotoxy1(203, i++);
		puts("                        *****                                "); gotoxy1(203, i++);
		puts("                        *****                                "); gotoxy1(203, i++);
		puts("                                                             "); gotoxy1(203, i++);
		break;
	case 2:
		i = 29;
		gotoxy1(203, i++);
		puts("                       *********                             "); gotoxy1(203, i++);
		puts("                     *************                           "); gotoxy1(203, i++);
		puts("                   *****************                         "); gotoxy1(203, i++);
		puts("                  *******************                        "); gotoxy1(203, i++);
		puts("                 *******      ********                       "); gotoxy1(203, i++);
		puts("                 *****          *******                      "); gotoxy1(203, i++);
		puts("                 ***             ******                      "); gotoxy1(203, i++);
		puts("                 **               *****                      "); gotoxy1(203, i++);
		puts("                 *                ******                     "); gotoxy1(203, i++);
		puts("                                   *****                     "); gotoxy1(203, i++);
		puts("                                   *****                     "); gotoxy1(203, i++);
		puts("                                   *****                     "); gotoxy1(203, i++);
		puts("                                   *****                     "); gotoxy1(203, i++);
		puts("                                   *****                     "); gotoxy1(203, i++);
		puts("                                   *****                     "); gotoxy1(203, i++);
		puts("                                  ******                     "); gotoxy1(203, i++);
		puts("                                  *****                      "); gotoxy1(203, i++);
		puts("                                 ******                      "); gotoxy1(203, i++);
		puts("                                 ******                      "); gotoxy1(203, i++);
		puts("                                ******                       "); gotoxy1(203, i++);
		puts("                               ******                        "); gotoxy1(203, i++);
		puts("                              *******                        "); gotoxy1(203, i++);
		puts("                            ********                         "); gotoxy1(203, i++);
		puts("                           ********                          "); gotoxy1(203, i++);
		puts("                          ********                           "); gotoxy1(203, i++);
		puts("                        ********                             "); gotoxy1(203, i++);
		puts("                       ********                              "); gotoxy1(203, i++);
		puts("                      ********                               "); gotoxy1(203, i++);
		puts("                     *******                                 "); gotoxy1(203, i++);
		puts("                   ********                                  "); gotoxy1(203, i++);
		puts("                   *******                                   "); gotoxy1(203, i++);
		puts("                  *******                                    "); gotoxy1(203, i++);
		puts("                 ******                                      "); gotoxy1(203, i++);
		puts("                 ******                                      "); gotoxy1(203, i++);
		puts("                ******                                       "); gotoxy1(203, i++);
		puts("                *****                                        "); gotoxy1(203, i++);
		puts("               ******                                        "); gotoxy1(203, i++);
		puts("               ******                                        "); gotoxy1(203, i++);
		puts("               ******                                        "); gotoxy1(203, i++);
		puts("               **************************                    "); gotoxy1(203, i++);
		puts("               **************************                    "); gotoxy1(203, i++);
		puts("               **************************                    "); gotoxy1(203, i++);
		puts("               **************************                    "); gotoxy1(203, i++);
		puts("                                                             "); gotoxy1(203, i++);
		break;
	case 3:
		i = 29;
		gotoxy1(203, i++);
		puts("                         ********                            "); gotoxy1(203, i++);
		puts("                       *************                         "); gotoxy1(203, i++);
		puts("                     ****************                        "); gotoxy1(203, i++);
		puts("                    *******************                      "); gotoxy1(203, i++);
		puts("                    *****       *******                      "); gotoxy1(203, i++);
		puts("                    ***           ******                     "); gotoxy1(203, i++);
		puts("                    *              *****                     "); gotoxy1(203, i++);
		puts("                                   ******                    "); gotoxy1(203, i++);
		puts("                                    *****                    "); gotoxy1(203, i++);
		puts("                                    *****                    "); gotoxy1(203, i++);
		puts("                                    *****                    "); gotoxy1(203, i++);
		puts("                                    *****                    "); gotoxy1(203, i++);
		puts("                                    *****                    "); gotoxy1(203, i++);
		puts("                                    *****                    "); gotoxy1(203, i++);
		puts("                                   *****                     "); gotoxy1(203, i++);
		puts("                                   *****                     "); gotoxy1(203, i++);
		puts("                                  *****                      "); gotoxy1(203, i++);
		puts("                                 *****                       "); gotoxy1(203, i++);
		puts("                              *******                        "); gotoxy1(203, i++);
		puts("                      **************                         "); gotoxy1(203, i++);
		puts("                      ************                           "); gotoxy1(203, i++);
		puts("                      **************                         "); gotoxy1(203, i++);
		puts("                      ****************                       "); gotoxy1(203, i++);
		puts("                              *********                      "); gotoxy1(203, i++);
		puts("                                 *******                     "); gotoxy1(203, i++);
		puts("                                   ******                    "); gotoxy1(203, i++);
		puts("                                   ******                    "); gotoxy1(203, i++);
		puts("                                    *****                    "); gotoxy1(203, i++);
		puts("                                     *****                   "); gotoxy1(203, i++);
		puts("                                     *****                   "); gotoxy1(203, i++);
		puts("                                     *****                   "); gotoxy1(203, i++);
		puts("                                     *****                   "); gotoxy1(203, i++);
		puts("                                     *****                   "); gotoxy1(203, i++);
		puts("                                     *****                   "); gotoxy1(203, i++);
		puts("                                     *****                   "); gotoxy1(203, i++);
		puts("                                    *****                    "); gotoxy1(203, i++);
		puts("                                    *****                    "); gotoxy1(203, i++);
		puts("                  *                *****                     "); gotoxy1(203, i++);
		puts("                  ***            *******                     "); gotoxy1(203, i++);
		puts("                  ******       ********                      "); gotoxy1(203, i++);
		puts("                  ********************                       "); gotoxy1(203, i++);
		puts("                   ******************                        "); gotoxy1(203, i++);
		puts("                     **************                          "); gotoxy1(203, i++);
		puts("                       *********                             "); gotoxy1(203, i++);
		break;

	default:
		i = 29;
		gotoxy1(203, i++);
		puts("                   ********                    **            "); gotoxy1(203, i++);
		puts("                 *************                 **            "); gotoxy1(203, i++);
		puts("               ****************                **            "); gotoxy1(203, i++);
		puts("              *******************              **            "); gotoxy1(203, i++);
		puts("              *****       *******          **********        "); gotoxy1(203, i++);
		puts("              ***           ******         **********        "); gotoxy1(203, i++);
		puts("              *              *****             **            "); gotoxy1(203, i++);
		puts("                             ******            **            "); gotoxy1(203, i++);
		puts("                              *****            **            "); gotoxy1(203, i++);
		puts("                              *****            **            "); gotoxy1(203, i++);
		puts("                              *****                          "); gotoxy1(203, i++);
		puts("                              *****                          "); gotoxy1(203, i++);
		puts("                              *****                          "); gotoxy1(203, i++);
		puts("                              *****                          "); gotoxy1(203, i++);
		puts("                             *****                           "); gotoxy1(203, i++);
		puts("                             *****                           "); gotoxy1(203, i++);
		puts("                            *****                            "); gotoxy1(203, i++);
		puts("                           *****                             "); gotoxy1(203, i++);
		puts("                        *******                              "); gotoxy1(203, i++);
		puts("                **************                               "); gotoxy1(203, i++);
		puts("                ************                                 "); gotoxy1(203, i++);
		puts("                **************                               "); gotoxy1(203, i++);
		puts("                ****************                             "); gotoxy1(203, i++);
		puts("                        *********                            "); gotoxy1(203, i++);
		puts("                           *******                           "); gotoxy1(203, i++);
		puts("                             ******                          "); gotoxy1(203, i++);
		puts("                             ******                          "); gotoxy1(203, i++);
		puts("                              *****                          "); gotoxy1(203, i++);
		puts("                               *****                         "); gotoxy1(203, i++);
		puts("                               *****                         "); gotoxy1(203, i++);
		puts("                               *****                         "); gotoxy1(203, i++);
		puts("                               *****                         "); gotoxy1(203, i++);
		puts("                               *****                         "); gotoxy1(203, i++);
		puts("                               *****                         "); gotoxy1(203, i++);
		puts("                               *****                         "); gotoxy1(203, i++);
		puts("                              *****                          "); gotoxy1(203, i++);
		puts("                              *****                          "); gotoxy1(203, i++);
		puts("            *                *****                           "); gotoxy1(203, i++);
		puts("            ***            *******                           "); gotoxy1(203, i++);
		puts("            ******       ********                            "); gotoxy1(203, i++);
		puts("            ********************                             "); gotoxy1(203, i++);
		puts("             ******************                              "); gotoxy1(203, i++);
		puts("               **************                                "); gotoxy1(203, i++);
		puts("                 *********                                   "); gotoxy1(203, i++);
		break;

	}

	copyToBuffer(hStdout1, 0, 0, MY, 260, hNewScreenBuffer1, 0, 0, MY, 260);

	Sleep(5000);


	if (life <= 0)
	{
		mciSendString("play  gameover from 0", NULL, 0, NULL);

		ifstream fp1(".\\res\\gameover.txt");                                                        //���GameOver����
		if (!fp1)
		{
			cerr << "open gameover.txt error" << endl;
			exit(1);
		}
		char tempStr[300];
		int i = 0;
		while (!fp1.eof())
		{
			gotoxy1(0, i++);
			fp1.getline(tempStr, 300);
			puts(tempStr);
		}
		fp1.close();

		copyToBuffer(hStdout1, 0, 0, MY, 260, hNewScreenBuffer1, 0, 0, MY, 260);
		Sleep(5000);

		GameState = -1;
		return;
	}
	else
	{
		GameState = (GameState + 1) * 10;
		return;
	}

}

//����Ӯ
void Duel::win()
{
	show();

	mciSendString("stop   Bossbgm ", NULL, 0, NULL);

	mciSendString("play  win  from 0", NULL, 0, NULL);
	Sleep(5000);



	while (_kbhit())
		char ch = _getch();
	//ʹ��ʾ���Ǳ�׼������	 
	SetConsoleActiveScreenBuffer(hStdout1);
	//�����ַ�����

	CharacterAnimation A(".\\res\\win0.txt", ".\\res\\BadApple.mp3", 160, 60 + 1);//46 + 1������   ��ʾ����
	//CharacterAnimation A("win0.txt", "BadApple.mp3", 240, 92, 71);//46 + 1������
	A.display();

	GameState = -1;
	return;
}

