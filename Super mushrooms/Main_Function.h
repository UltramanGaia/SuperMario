
#ifndef MAIN_TUNCTION_H
#define MAIN_TUNCTION_H
#include <windows.h>
#include <iostream>
using namespace std;

void init();//��ʼ��
void fullscreen();//ȫ��
void remove_scrollbar();//���ع�����


//��ʼ��
void init()
{
	fullscreen();
	mciSendString("open ��������.mp3 alias bgm", NULL, 0, NULL);
	mciSendString("open �ӵ�.mp3 alias bullet", NULL, 0, NULL);
	mciSendString("open ���.mp3 alias coin", NULL, 0, NULL);
	mciSendString("open ��.mp3 alias jump", NULL, 0, NULL);
	mciSendString("open �ӵ��򵽵���.mp3 alias hit_enemy", NULL, 0, NULL);
	mciSendString("open �ӵ�ײǽ.mp3 alias hit_wall", NULL, 0, NULL);
	mciSendString("open �ȵ���.mp3 alias step", NULL, 0, NULL);
	mciSendString("open �Ե�����.mp3 alias arm", NULL, 0, NULL);
	mciSendString("open ʤ��.mp3 alias win", NULL, 0, NULL);
	mciSendString("open ����1.mp3 alias dead1", NULL, 0, NULL);
	mciSendString("open ����2.mp3 alias dead2", NULL, 0, NULL);

}

//���ع�����
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


//ȫ��
void fullscreen()
{
	SetConsoleDisplayMode(GetStdHandle(STD_OUTPUT_HANDLE), CONSOLE_FULLSCREEN_MODE, 0);

	remove_scrollbar();
}


#endif // 