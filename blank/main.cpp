/*  项目中的详尽解释：http://t.cn/RHjgacn  */

#include <graphics.h>
#include <math.h>
#include<string.h>
#include<stdio.h>
#include <windows.h>  
#pragma comment(lib, "Winmm.lib")            //For MCI(Media Control Interface，媒体控制接口)  

#define Path_Music_resPiano "..//resplus//piano"	//载入键音
#define Path_Music_Fail "..//resplus//fail.mp3"		//失败音乐
#define Path_Music_Launch "..//resplus//chuanshao.mp3"	//启动音乐
#define Path_pre "..//resplus//piano//"	//音频路径前缀
#define Path_end ".mp3"	//音频路径后缀
#define WINDOW_HIGHT 800	//窗口高度
#define WINDOW_WIDTH 480	//窗口宽度
#define Blank_HIGHT 160		//方块高度
#define Blank_WIDTH 120		//方块宽度
#define Blank_COLOR EGERGB(00,00,00)	//方块颜色
#define COLOR_WHITE EGERGB(0xFF, 0xFF, 0xFF)	//白色
#define FPS 100	//FPS
#define SLEEP 10 //延迟启动
#define TopLineHight 20	//顶部显示区域高度
#define v 1 // 初始速度

//-------函数声明--------
void drawTopLine(int);	//绘制计数器
void initFile();	//初始化文件资源
void debugouttext(int i, char c[]); 	//调试输出
DWORD WINAPI music_play(LPVOID pParam); 	//播放音乐
void music_play_control(char key[]);	//控制音乐播放
DWORD WINAPI GameControl();		//控制
void GameView();	//	绘图
void creatView();	//创建地图

												//-------全局变量--------
int mScore = 0;	//分数
char mCharsScore[50];	//分数字符串
char track[1024][12];	//乐谱
int mLength = 0;	//键总数
int below = 0;	//最下面一个方块定点的y轴
PIMAGE totalimg;	//总画布
PIMAGE topline;	//顶部显示区域
PIMAGE live;	//生命值图案
int randtrack[3000] = { -1000 };	//存轨道的随机数
int inter_y = 0;	//生成图片时用到的y
bool failplay = false;	//游戏是否结束
int accelerate = 0;	//游戏加速度
int liveValue = 10;	//生命值

char Path_A_Breeze_From_Alabama[] = "..//resplus//A Breeze From Alabama.txt";
char Path_Happy_New_Year[] = "..//resplus//Happy New Year.txt";
char Path_Jasmine[] = "..//resplus//Jasmine.txt";
char Path_Little_Star[] = "..//resplus//Little Star.txt";
char Path_lyricwaltz[] = "..//resplus//lyricwaltz.txt";
char Path_Merry_Christmas[] = "..//resplus//Merry Christmas.txt";
char Path_Music[50];

typedef struct ThreadParameter {	//音频播放线程参数
	char filename[200];
	char key[30];
};

ThreadParameter tpp;	//音频播放线程传参结构体
int main()	//主函数
{
	//--------播放启动音乐----------begin
	ThreadParameter tp;
	strcpy(tp.filename, Path_Music_Launch);
	CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)music_play, &tp, 0, NULL);
	//--------播放启动音乐----------end

	//------------初始化------------begin
	setinitmode(RENDER_MANUAL);
	initgraph(WINDOW_WIDTH, WINDOW_HIGHT, INIT_RENDERMANUAL);	//初始化窗口
	topline = newimage(WINDOW_WIDTH, TopLineHight + 1);	//初始化窗口顶部显示区域画布
	live = newimage(20, 20);
	setbkcolor(EGERGB(0xDC, 0x14, 0x3C), live);		//生命值图片背景
	cleardevice(live);
	strcpy(Path_Music, Path_Jasmine);	//默认关卡
	//------------初始化------------end

	//----------选择关卡------------begin
	setfont(20, 0, "宋体");
	outtextxy(10, 10, "按下对应字母进入关卡");
	outtextxy(10, 35, "A:	A Breeze From Alabama");
	outtextxy(10, 55, "B:	茉莉花");
	outtextxy(10, 75, "C:	Happy New Year");
	outtextxy(10, 95, "D:	Little Star");
	outtextxy(10, 115, "E:	lyricwaltz");
	outtextxy(10, 135, "F:	Merry Christmas");
	switch (getch())
	{
	case 'A':
		strcpy(Path_Music, Path_A_Breeze_From_Alabama);
		break;
	case 'B':
		strcpy(Path_Music, Path_Jasmine);
		break;
	case 'C':
		strcpy(Path_Music, Path_Happy_New_Year);
		break;
	case 'D':
		strcpy(Path_Music, Path_Little_Star);
		break;
	case 'E':
		strcpy(Path_Music, Path_lyricwaltz);
		break;
	case 'F':
		strcpy(Path_Music, Path_Merry_Christmas);
		break;
	default:
		break;
	}
	//----------选择关卡------------end
	initFile();	//读入相关文件
	creatView();	//创建地图
	CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)GameControl, NULL, 0, NULL);	//启动键盘控制线程

	GameView();	//绘图
	getch();	//防止游戏结束后退出
	return 0;
}

void initFile()	//初始化文件资源
{
	FILE *fp = NULL;
	fp = fopen(Path_Music, "r");
	if (fp != NULL)
	{
		while (fscanf(fp, "%s", track[mLength]) != EOF)
		{	//读入所有键
			mLength++;
		}
	}
	fclose(fp);
}

DWORD WINAPI ThreadDrawTrack()
{
	setrendermode(RENDER_MANUAL);
	int y = inter_y;
	int addh = 0;
	ThreadParameter tp;
	strcpy(tp.filename, Path_Music_Fail);
	for (; is_run(); delay_fps(FPS))
	{
		if (failplay)
		{
			cleardevice();
			sprintf(mCharsScore, "%d", mScore);
			setfont(100, 0, "宋体");
			setfontbkcolor(EGERGB(0xff, 0x00, 0x00));
			outtextxy(WINDOW_WIDTH / 2 + 20, WINDOW_HIGHT / 2, mCharsScore);
			outtextxy(0, WINDOW_HIGHT / 2, "得分:");
			break;
		}
		if (-1 * y >= 0)
		{
			break;
		}
		putimage(0, -1 * y, totalimg);
		y -= v + accelerate;
		below = addh - mScore*Blank_HIGHT - 160;
		if (below >= WINDOW_HIGHT)
		{
			CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)music_play, &tp, 0, NULL);
			failplay = true;
		}
		addh += v;
		drawTopLine(liveValue);	//更新显示得分
	}
	return 1;
}

void GameView()	//	绘图
{
	randomize();	//初始化随机数
	cleardevice();	//清屏
	setcaption("别踩白块 - 1707004544 李浩");		//窗口标题
	setbkcolor(EGERGB(0xff, 0xff, 0xff));			//背景色
	setbkcolor(EGERGB(0xff, 0xff, 0xff),topline);	//顶部显示区域背景色
	drawTopLine(liveValue);
	ThreadDrawTrack();
}

DWORD WINAPI GameControl()	//控制
{
	int curr = 0;
	int k = 0, index = 0;
	ThreadParameter tp;
	strcpy(tp.filename, Path_Music_Fail);	//给线程传参结构体赋值失败音乐路径
	for (; k != key_esc;)
	{
		if (kbhit())
		{
			k = getch();
			if (k == 'S')
			{
				if (randtrack[mLength - curr - 1] == 1)
				{
					music_play_control(track[index++]);		//将键音传入音乐处理函数
					mScore++;	//得分加一
					drawTopLine(liveValue);	//刷新计分标签
				}
				else
				{
					liveValue--;
					if (liveValue <= 0)
					{
						CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)music_play, &tp, 0, NULL);	//播放失败音乐线程
						failplay = true;	//设定失败标志为真
					}
					else
					{
						continue;
					}
				}
			}
			else if (k == 'D')
			{
				if (randtrack[mLength - curr - 1] == 2)
				{
					music_play_control(track[index++]);
					mScore++;
					drawTopLine(liveValue);
				}
				else
				{
					liveValue--;
					if (liveValue <= 0)
					{
						CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)music_play, &tp, 0, NULL);	//播放失败音乐线程
						failplay = true;	//设定失败标志为真
					}
					else
					{
						continue;
					}
				}
			}
			else if (k == '4')
			{
				if (randtrack[mLength - curr - 1] == 3)
				{
					music_play_control(track[index++]);
					mScore++;
					drawTopLine(liveValue);
				}
				else
				{
					liveValue--;
					if (liveValue <= 0)
					{
						CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)music_play, &tp, 0, NULL);	//播放失败音乐线程
						failplay = true;	//设定失败标志为真
					}
					else
					{
						continue;
					}
				}
			}
			else if (k == '5')
			{
				if (randtrack[mLength - curr - 1] == 4)
				{
					music_play_control(track[index++]);
					mScore++;
					drawTopLine(liveValue);
				}
				else
				{
					liveValue--;
					if (liveValue <= 0)
					{
						CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)music_play, &tp, 0, NULL);	//播放失败音乐线程
						failplay = true;	//设定失败标志为真
					}
					else
					{
						continue;
					}
				}
			}
			curr++;
			accelerate = mScore / 30;	//设定滑块每25分增加速度
		}
		if (failplay)
		{
			api_sleep(500);	//等待播放音乐线程启动
			return 0;
		}
	}
	return 0;
}

void drawTopLine(int n)	//绘制计数器
{
	int width = 0;
	cleardevice(topline);
	sprintf(mCharsScore, "%d", mScore);		//int转char*
	setfont(40, 0, "宋体");		//设定字体样式
	setfontbkcolor(EGERGB(0xff, 0x00, 0x00));	//设置字体背景色
	outtextrect(WINDOW_WIDTH - 40, 0, 20, 20, mCharsScore, topline);	//在窗口输出字体
	for (int i = 0; i < n; i++)
	{
		putimage(topline, width, 0, live);
		width += 22;
	}
	putimage(0, 0, topline);
}

void creatView()
{
	int point[8];	//四边形点集
	int x = 0, y = 0;	//基定点坐标
	randomize();	//初始化随机数
	totalimg = newimage(WINDOW_WIDTH, Blank_HIGHT*mLength);	//初始化总画布
	setbkcolor(COLOR_WHITE, totalimg);	//设置总画布背景
	cleardevice(totalimg);	//清空总画布
	for (int i = 0; i < mLength; i++)
	{
		randtrack[i] = random(4) + 1;	//参数0-3的随机数，加一后为轨道位置
		y = inter_y;
		x = (randtrack[i] - 1) * Blank_WIDTH;
		point[0] = x;
		point[1] = y;
		point[2] = x + Blank_WIDTH;
		point[3] = y;
		point[4] = x + Blank_WIDTH;
		point[5] = y + Blank_HIGHT;
		point[6] = x;
		point[7] = y + Blank_HIGHT;
		setfillstyle(SOLID_FILL, Blank_COLOR);	//设置方块颜色
		fillpoly(4, point, totalimg);	//绘制方块
		inter_y += Blank_HIGHT;	//设置滑块公共高度
	}
}

void music_play_control(char key[])	//控制音乐播放
{
	int flage = 0;
	char copy[30];
	char buff[10][30];	//同时按下的键值
	char temp[200];
	memset(temp, '\0', sizeof(temp));
	//按照特定格式读入每个键符
	if (key[0] != '(')
	{
		strcat(temp, Path_pre);
		strcat(temp, key);
		strcat(temp, Path_end);
		strcpy(tpp.filename, temp);
		CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)music_play, &tpp, 0, NULL);
	}
	else
	{
		memset(buff, '\0', sizeof(buff));
		strcpy(copy, key);
		for (int i = 0; i < strlen(copy); i++)
		{
			if (copy[i] == '.')
			{
				flage++;
				copy[i] = ' ';
			}
			else if (copy[i] == '(' || copy[i] == ')')
			{
				copy[i] = ' ';
			}
		}
		if (flage == 0)
		{
			sscanf(copy, "%s", buff[0]);
		}
		else if (flage == 1)
		{
			sscanf(copy, "%s %s", buff[0], buff[1]);
		}
		else if (flage == 2)
		{
			sscanf(copy, "%s %s %s", buff[0], buff[1], buff[2]);
		}
		else if (flage == 3)
		{
			sscanf(copy, "%s %s %s %s", buff[0], buff[1], buff[2], buff[3]);
		}
		for (int i = 0; i < flage; i++)
		{
			strcat(temp, Path_pre);
			strcat(temp, buff[i]);
			strcat(temp, Path_end);
			strcpy(tpp.filename, temp);
			CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)music_play, &tpp, 0, NULL);	//启动音乐播放线程
			api_sleep(200);
		}
	}
}
DWORD WINAPI music_play(LPVOID pParam)	//播放音乐
{
	//mciSendString(tp->filename, NULL, 0, NULL); //低效播放方式

	//高效率无阻塞播放
	ThreadParameter *tp = (ThreadParameter *)pParam;
	MCI_OPEN_PARMS open;//定义打开结构体变量
	open.lpstrElementName = tp->filename;//填充参数
	mciSendCommand(0, MCI_OPEN, MCI_OPEN_ELEMENT, DWORD_PTR(&open));//打开
	MCI_PLAY_PARMS play;//定义播放结构题变量
	mciSendCommand(open.wDeviceID, MCI_PLAY, 0, DWORD_PTR(&play));//非阻塞式播放
	return 0;
}

void debugouttext(int i, char c[])	//调试输出
{
	//在窗口上输出特定信息，调试用
	char temp[20];
	if (c != NULL)
	{
		outtextxy(0, 0, c);
	}
	else
	{
		sprintf(temp, "%d", i);
		outtextxy(10, 10, temp);
	}

}

