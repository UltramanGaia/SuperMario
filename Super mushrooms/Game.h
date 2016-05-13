#pragma once
#include <iostream>
#include <conio.h>
#include <math.h>
#include <Windows.h>
using namespace std;

#define MX  180//һ��ͼ���x
#define MY  50//һ��ͼ���y
#define MK  3 //����ͼ������һ��
#define XX  1//45                       //������ʼλ��
#define YY  39 
#define JH  7 //��Ծ�߶�
#define N_COIN   5 //Ӳ����
#define N_EMERY  5 //������
extern int life;                          //ȫ�ֱ���������
extern int score;                         //ȫ�ֱ���������
struct Hero
{
	int id;
	int x;//������
	int y;//������
	int turn;//�˶�����
	
	int jumpStep;//
	//           5
	//       4      -4 
	//     3            -3
	//   2                 -2
	// 1                      -1
	//0                         0
};

struct Map
{
	int id;//0 �� ��������
	int x;
	int y;
	int turn;//-1 �� 0���� 1 ��
	//int XStep;//
	int xLeftMost;
	int xRightMost;

};


class game        //��Ϸ��
{
public:
	game();
	~game();
	void start();        //��Ϸ��ʼ�Ľ��棬�Ͱ���ͣ����Ľ���
	void init();         //��ʼ��,��ͼ��
	void show();         //��ʾ����
	void control(char ); //���������ƶ�
	void judge();
	void gamePause();   //��Ϸ��ͣ
	void end();         //������Ϸ����
	void left();        //���������˶�
	void right();       //���������˶�
	void up();			//������Ծ
	bool HeroisHitWall();
	bool HeroisLanded();
	bool HeroUpHitWall();
	bool iscoin();
	bool isEmery();
private:
	int centerLine;
	Hero hero;
	char BMap[MY][MX * MK +1];
	Map coins[N_COIN];
	Map emery[N_EMERY];
	FILE *fp;
	int win;               //����Ƿ����
	int pause;             //����Ƿ�Esc����ͣ����
};