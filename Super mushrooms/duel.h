#ifndef DUEL_H
#define DUEL_H
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

extern int GameState;                    //��Ϸ���е�ʲô״̬
extern int life;                         //������
extern int N_COIN;                        //Ӳ����,������Ϊ��Ѫҩ
struct Map1
{
	int id;//0 �� ��������
	int x;
	int y;
	int turn;//-1 �� 0���� 1 ��
	int xLeftMost;
	int xRightMost;
};

struct Person
{
	int id;
	int x;        //������(���Ͻ�)
	int y;        //�����꣨���Ͻǣ�
	int hp;       //����ֵ
	int turn;     //�˶�����
	int state;    //���ز�ͬ��ͼ
	int jump;    //�Ƿ���Ծ
	double v0 = 0;	 //��Ծ�ĳ��ٶ�
	double h = 0;	 //��Ծ�ĸ߶�
	double t = 0;	 //��Ծ��ʱ��
	Map1 bullet[20];        //�ӵ� , 20��
	int n_bullet;  //�ڼ����ӵ�
};


class Duel
{
public:
	Duel();
	~Duel();

	void start();        //��Ϸ��ʼ
	void init();         //��ʼ��,��ͼ��
	void show();         //��ʾ����
	void control(); //���������ƶ�
	void judge();        //���� �ж�
	void DuelPause();    //��Ϸ��ͣ
	void dead();          //������
	void win();          //����Ӯ
	void setWindows();   //���ô���
	void loadBackGround();//���ر�����������
	void loadMusic();     //��������
	void loadBlood();
	void showBackground();	//��ʾ����		
	void showBullet();		//��ʾ�ӵ�
	void showHero();     //��ʾ����
	void showEnemy();    //��ʾ����
	void showLife();     //��ʾ����
	void showCoins();

	void copyToBuffer(HANDLE, int, int, int, int, HANDLE, int, int, int, int);
	void left(Person  & Hero);         //���������˶�
	void right(Person & Hero);        //���������˶�
	void up(Person & Hero);			 //������Ծ
	void shoot();        //���ǿ�ǹ
	bool BulletHit();    //�ӵ�ײ������
	bool HeroisHitWall();//����ײǽ
	bool HeroisLanded(Person &); //վ�ڵ���
	bool HeroUpHitWall();//��������ʱײ��ͷ
	bool EnemyUpHitWall();//��������ʱײ��ͷ
	bool isblood();       //����Ѫ��
	int  GetCommand();   //��ȡ������Ϣ
	int GetAICommand();  //��ȡAI������Ϣ
	void EnemyShoot();  //���˿�ǹ



private:
	int centerLine;        //��Ļ���� �������ƶ���Ļ
	char BMap1[MY][MX * MK + 1];//����ͼ����
	Map1 blood[100];        //��Ѫҩ
	Person Hero;
	Person Enemy;
};



#endif