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
#pragma comment(lib,"Winmm.lib")   //����Ϸ�������Ҫ�õ���
int life;                          //ȫ�ֱ���������
int score;                         //ȫ�ֱ���������
int chapter = 1;                   //ȫ�ֱ������ؿ�
int GameState = 0;                    //��Ϸ���е�ʲô״̬

int N_COIN = 13;                   //Ӳ����
int N_EMERY = 14;                  //������

void login();					  //��¼�������
void help();					  //��ʾ����

int  main()
{
	N_COIN = 24;
	N_EMERY = 14;
	while(1)
		login();
}

//��¼�������
void login()
{
	void hideCursor();
	void gotoxyAndPutchar(int x, int y, char c, int color = 0x07);
	void gotoxy(int x, int y);                                           //�ƶ���굽x,y
	void setFontSizeMode(int n);
	void remove_scrollbar();                                             //���ع�����

	SetConsoleDisplayMode(GetStdHandle(STD_OUTPUT_HANDLE), CONSOLE_FULLSCREEN_MODE, 0);//ȫ��
																					   //hideCursor();                                                                      //���ع��
	setFontSizeMode(7);                                                                //��������ģʽ7
	system("mode con cols=500 lines=500");                                             //����̨����
	remove_scrollbar();                                                               //���ع�����
	hideCursor();

	ifstream fp(".\\res\\login.txt");                                                        //�����¼����

	char tempStr[300];
	int i = 0;
	while (!fp.eof())
	{
		gotoxy(0, i++);
		fp.getline(tempStr, 300);
		puts(tempStr);
	}
	fp.close();
	int q;     //�����ƶ����ʱ��λ
	int t = 1;  //ѡ��1 ENTER   2 EXIT
	int count = 0;  //����ȷ���Ƿ�Ӧ�ý�����ʾ����
	while (1)
	{
		++count;
		if (count >= 300000)
		{
			CharacterAnimation A(".\\res\\yanshi.txt", ".\\res\\login.mp3", 260, 79 + 1);//46 + 1������   ��ʾ����
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

//��ʾ����
void help()
{
	void gotoxy(int x, int y);                                                       //�ƶ���굽x,y

	ifstream fhelp(".\\res\\help.txt");                                                        //�����¼����

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