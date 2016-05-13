#pragma once
#pragma warning (disable:4996)
#include<iostream>
#include<string.h>
#include<cstdio>
#include<time.h>
#include<windows.h>
#include <string>
#include <mmsystem.h>
#ifndef CHARACTERANIMATION_H
#define CHARACTERANIMATION_H
#pragma comment(lib,"winmm.lib")
using namespace std;
class CharacterAnimation
{
public:
	CharacterAnimation(char * txt, char * music ,int xx,int yy,int fps = 26);
	~CharacterAnimation();
	void recursur();
	void display();
	FILE *fp;

private:
	int frame, caf;
	char buf[1920], seat[20];
	int  XX0;
	int  YY0;
};

#endif