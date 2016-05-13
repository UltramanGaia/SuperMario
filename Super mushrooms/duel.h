#ifndef DUEL_H
#define DUEL_H
#include <iostream>
#include <conio.h>
#include <math.h>
#include <Windows.h>
using namespace std;
#define G	9.8   //重力加速度 
#define MX  260//一张图里的x
#define MY  80//一张图里的y
#define MK  9 //几张图连接在一起
#define XX  1//45                       //主角起始位置
#define YY  60 
#define HX  16  //主角  ，， 生物的 ，宽度
#define HY  10  //主角  ，， 高度
#define JH   (2 * HX + 1) //跳跃高度
#define STEP  2 //主角走一步相距的像素个数


#define	CMD_LEFT   1                 //方向键的宏定义
#define	CMD_RIGHT  2	
#define	CMD_UP     4
#define CMD_DOWN   8
#define CMD_SHOOT 16
#define CMD_ESC   32

extern int GameState;                    //游戏进行到什么状态
extern int life;                         //生命数
extern int N_COIN;                        //硬币数,这里作为补血药
struct Map1
{
	int id;//0 则 不复存在
	int x;
	int y;
	int turn;//-1 左 0不懂 1 右
	int xLeftMost;
	int xRightMost;
};

struct Person
{
	int id;
	int x;        //横坐标(左上角)
	int y;        //纵坐标（左上角）
	int hp;       //生命值
	int turn;     //运动方向
	int state;    //加载不同的图
	int jump;    //是否跳跃
	double v0 = 0;	 //跳跃的初速度
	double h = 0;	 //跳跃的高度
	double t = 0;	 //跳跃的时间
	Map1 bullet[20];        //子弹 , 20颗
	int n_bullet;  //第几颗子弹
};


class Duel
{
public:
	Duel();
	~Duel();

	void start();        //游戏开始
	void init();         //初始化,地图等
	void show();         //显示画面
	void control(); //控制主角移动
	void judge();        //处理 判断
	void DuelPause();    //游戏暂停
	void dead();          //处理死
	void win();          //处理赢
	void setWindows();   //设置窗口
	void loadBackGround();//加载背景到数组中
	void loadMusic();     //加载音乐
	void loadBlood();
	void showBackground();	//显示背景		
	void showBullet();		//显示子弹
	void showHero();     //显示主角
	void showEnemy();    //显示敌人
	void showLife();     //显示生命
	void showCoins();

	void copyToBuffer(HANDLE, int, int, int, int, HANDLE, int, int, int, int);
	void left(Person  & Hero);         //主角向左运动
	void right(Person & Hero);        //主角向右运动
	void up(Person & Hero);			 //主角跳跃
	void shoot();        //主角开枪
	bool BulletHit();    //子弹撞到东西
	bool HeroisHitWall();//左右撞墙
	bool HeroisLanded(Person &); //站在地面
	bool HeroUpHitWall();//主角上升时撞到头
	bool EnemyUpHitWall();//敌人上升时撞到头
	bool isblood();       //捡到了血包
	int  GetCommand();   //获取按键信息
	int GetAICommand();  //获取AI按键信息
	void EnemyShoot();  //敌人开枪



private:
	int centerLine;        //屏幕中线 ，用于移动屏幕
	char BMap1[MY][MX * MK + 1];//背景图数组
	Map1 blood[100];        //补血药
	Person Hero;
	Person Enemy;
};



#endif