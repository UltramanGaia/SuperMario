#pragma once
#include <iostream>
#include <conio.h>
#include <math.h>
#include <Windows.h>
using namespace std;

#define MX  180//一张图里的x
#define MY  50//一张图里的y
#define MK  3 //几张图连接在一起
#define XX  1//45                       //主角起始位置
#define YY  39 
#define JH  7 //跳跃高度
#define N_COIN   5 //硬币数
#define N_EMERY  5 //敌人数
extern int life;                          //全局变量，生命
extern int score;                         //全局变量，分数
struct Hero
{
	int id;
	int x;//横坐标
	int y;//纵坐标
	int turn;//运动方向
	
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
	int id;//0 则 不复存在
	int x;
	int y;
	int turn;//-1 左 0不懂 1 右
	//int XStep;//
	int xLeftMost;
	int xRightMost;

};


class game        //游戏类
{
public:
	game();
	~game();
	void start();        //游戏开始的界面，和按暂停键后的界面
	void init();         //初始化,地图等
	void show();         //显示画面
	void control(char ); //控制主角移动
	void judge();
	void gamePause();   //游戏暂停
	void end();         //处理游戏结束
	void left();        //主角向左运动
	void right();       //主角向右运动
	void up();			//主角跳跃
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
	int win;               //玩家是否过关
	int pause;             //玩家是否按Esc（暂停键）
};