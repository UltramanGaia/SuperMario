#pragma once
#include <iostream>
#include <conio.h>
#include <math.h>
#include <Windows.h>
using namespace std;
#define G	9.8   //�������ٶ� 
#define MX  260//һ��ͼ���x
#define MY  80//һ��ͼ���y
#define MK  9 //����ͼ������һ��
#define XX  1//45                       //������ʼλ��
#define YY  60 
#define HX  16  //����  ���� ����� �����
#define HY  10  //����  ���� �߶�
#define JH   (2 * HX + 1) //��Ծ�߶�
#define STEP  2 //������һ���������ظ���


#define	CMD_LEFT   1                 //������ĺ궨��
#define	CMD_RIGHT  2	
#define	CMD_UP     4
#define CMD_DOWN   8
#define CMD_SHOOT 16
#define CMD_ESC   32


extern int life;                          //ȫ�ֱ���������
extern int score;						  //ȫ�ֱ���������
extern int chapter;                       //ȫ�ֱ������ؿ�
extern int GameState;                    //��Ϸ���е�ʲô״̬
extern int N_COIN;                        //Ӳ����
extern int N_EMERY;                       //������
struct Hero
{
	int id;
	int x;        //������(���Ͻ�)
	int y;        //�����꣨���Ͻǣ�
	int turn;     //�˶�����
	int state;    //���ز�ͬ��ͼ
	int jump;//
};

struct Map
{
	int id;//0 �� ��������
	int x;
	int y;
	int turn;//-1 �� 0���� 1 ��
	int xLeftMost;
	int xRightMost;
};


class SuperMushRoom                //��Ϸ��
{
public:
	SuperMushRoom();
	~SuperMushRoom();
	void start();        //��Ϸ��ʼ
	void init();         //��ʼ��,��ͼ��
	void randInit();     // �����ʼ��
	void show();         //��ʾ����
	void control(); //���������ƶ�
	void judge();        //���� �ж�
	void SuperMushRoomPause();    //��Ϸ��ͣ
	void dead();          //������
	void win();          //����Ӯ
	void setWindows();   //���ô���
	void loadBackGround();//���ر�����������
	void loadMusic();     //��������
	void loadCoins();	  //���ؽ��N_COIN
	void loadEnemy();     //���ص���N_EMERY					  
	void showBackground();	//��ʾ����					
	void showCoins();		//��ʾ���
	void showBullet();		//��ʾ�ӵ�
	void showWeapon();      //��ʾ��ȡ����
	void showHero();     //��ʾ����
	void showEnemy();    //��ʾ����
	void showScore();    //��ʾ����
	void showChapter(); //��ʾ�ؿ�
	void showLife();     //��ʾ����
						 //�����׼�������еĻ��浽��ʾ�Ļ�����
	void copyToBuffer(HANDLE,int,int,int,int,HANDLE,int,int,int,int); 
	void left();         //���������˶�
	void right();        //���������˶�
	void up();			 //������Ծ
	void shoot();        //���ǿ�ǹ
	bool BulletHit();    //�ӵ�ײ������
	bool HeroisHitWall();//����ײǽ
	bool HeroisLanded(); //վ�ڵ���
	bool HeroUpHitWall();//����ʱײ��ͷ
	bool HeroDownStep();//�½��ȵ�����
	bool iscoin();      //�Ƿ��������
	bool isEnemy();     //�Ƿ���������
	int  GetCommand();   //��ȡ������Ϣ
private:
	double v0;             //��ɫ��Ծ�ĳ��ٶ�
	double h;              //��ɫ��Ծ�ĸ߶�
	double t;              //��ɫ��Ծ��ʱ��
	int centerLine;        //��Ļ���� �������ƶ���Ļ
	Hero hero;
	char BMap[MY][MX * MK +1];//����ͼ����
	Map coins[100];     //������� , 100��Ӧ�ù�����
	Map enemy[100];    //�������� , 100��
	Map bullet[20];        //�ӵ� , 20��
	Map weapon;            //��������ĵ���
	int  arm;              //�Ƿ��ǹ
	int  n_bullet;         //����ڼ����ӵ�����n_bullet>=19 ,n_bullet = 0;
	FILE *fp;



};