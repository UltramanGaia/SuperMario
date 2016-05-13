#pragma once
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


extern int life;                          //全局变量，生命
extern int score;						  //全局变量，分数
extern int chapter;                       //全局变量，关卡
extern int GameState;                    //游戏进行到什么状态
extern int N_COIN;                        //硬币数
extern int N_EMERY;                       //敌人数
struct Hero
{
	int id;
	int x;        //横坐标(左上角)
	int y;        //纵坐标（左上角）
	int turn;     //运动方向
	int state;    //加载不同的图
	int jump;//
};

struct Map
{
	int id;//0 则 不复存在
	int x;
	int y;
	int turn;//-1 左 0不懂 1 右
	int xLeftMost;
	int xRightMost;
};


class SuperMushRoom                //游戏类
{
public:
	SuperMushRoom();
	~SuperMushRoom();
	void start();        //游戏开始
	void init();         //初始化,地图等
	void randInit();     // 随机初始化
	void show();         //显示画面
	void control(); //控制主角移动
	void judge();        //处理 判断
	void SuperMushRoomPause();    //游戏暂停
	void dead();          //处理死
	void win();          //处理赢
	void setWindows();   //设置窗口
	void loadBackGround();//加载背景到数组中
	void loadMusic();     //加载音乐
	void loadCoins();	  //加载金币N_COIN
	void loadEnemy();     //加载敌人N_EMERY					  
	void showBackground();	//显示背景					
	void showCoins();		//显示金币
	void showBullet();		//显示子弹
	void showWeapon();      //显示获取武器
	void showHero();     //显示主角
	void showEnemy();    //显示敌人
	void showScore();    //显示分数
	void showChapter(); //显示关卡
	void showLife();     //显示生命
						 //传输标准缓冲区中的画面到显示的缓冲区
	void copyToBuffer(HANDLE,int,int,int,int,HANDLE,int,int,int,int); 
	void left();         //主角向左运动
	void right();        //主角向右运动
	void up();			 //主角跳跃
	void shoot();        //主角开枪
	bool BulletHit();    //子弹撞到东西
	bool HeroisHitWall();//左右撞墙
	bool HeroisLanded(); //站在地面
	bool HeroUpHitWall();//上升时撞到头
	bool HeroDownStep();//下降踩到敌人
	bool iscoin();      //是否碰到金币
	bool isEnemy();     //是否碰到敌人
	int  GetCommand();   //获取按键信息
private:
	double v0;             //角色跳跃的初速度
	double h;              //角色跳跃的高度
	double t;              //角色跳跃的时间
	int centerLine;        //屏幕中线 ，用于移动屏幕
	Hero hero;
	char BMap[MY][MX * MK +1];//背景图数组
	Map coins[100];     //金币数组 , 100个应该够用了
	Map enemy[100];    //敌人数组 , 100个
	Map bullet[20];        //子弹 , 20颗
	Map weapon;            //获得武器的道具
	int  arm;              //是否捡到枪
	int  n_bullet;         //射出第几颗子弹，当n_bullet>=19 ,n_bullet = 0;
	FILE *fp;



};