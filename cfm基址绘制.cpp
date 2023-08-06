#include <MemoryTools.h>
// 内存插件
#include <RainyShareMemory.h>
// 共享内存
#include <TouchHelper.h>
// 滑屏函数
#include <conio.h>
#include <string>
#include <unistd.h>
#include <stdlib.h>

class AimPos
{
	public:
		const static int POSITION_HEAD = 0;
		const static int POSITION_BODY = 1;
		const static int POSITION_LEG = 2;
} position;

struct AimStruct
{
	float x = 0;
	//x
	float y = 0;
	//y
	float ScreenDistance = 0;
	//屏幕距离
	float WorldDistance = 0;
	//世界距离
} Aim[100] = {0};

void DrawInfo(R_Canvas* canvas);
//绘制信息
void findmin();
//查找最小数据

float px;						// 宽度
float py;						// 高度
float bar;					   // 状态栏高度
float CameraMatrix[16];

float AimRadius = 150.0f;
//自瞄范围

float SlideRadius = 150.0f;
//滑屏范围

int AimPosition = 0;
//自瞄位置
/*
	>> 自瞄位置 <<
	
    0:头部
    1:胸
    2:腿
*/

int AimMode = 2;
/*
    >> 自瞄模式 <<
    
	1:开火自瞄
	2:锁定自瞄
*/

int AutoPressure = 1;
/*
	自动压枪
	1:开启
	2:关闭
*/

float zxx,zxy;
//准星坐标

int Gmin=-1;
float SlideX = 0.0f;
float SlideY = 0.0f;
//划屏位置
int AimingActor = 0;

float zm_x,zm_y;
//自瞄位置
int ToReticleDistance;
//到准星距离
int WorldDistance;
//世界距离
int AimCount = 0;
int AimObjCount = 0;
int PlayerCount = 0,MaxPlayerCount = 0;

DWORD LibUnityModuleBase = 0;
DWORD LibMonoModuleBase = 0;
DWORD LibTersafeModuleBase = 0;
// 基址

DWORD isPressing;

int findminat()
{
	float min = 99999999;
	int minAt = 999;
	for (int i=0;i<MaxPlayerCount;i++)
	{
		if (Aim[i].ScreenDistance < min && Aim[i].ScreenDistance != 0)
		{
			min = Aim[i].ScreenDistance;
			minAt = i;
		}
	}
	if (minAt == 999)
	{
		Gmin = -1;
		return -1;
	}
	Gmin = minAt;
	zm_x = Aim[minAt].y;
	zm_y = Aim[minAt].x;
	ToReticleDistance = Aim[minAt].ScreenDistance;
	WorldDistance = Aim[minAt].WorldDistance;
	//printf("at:%d\n",minAt);
	return minAt;
}

DWORD GetIsFireButtonPressed()
{
	DWORD temp;
	/*GetAddressValue(LibMonoModuleBase + 0x3994CC, &temp, sizeof (DWORD));
	GetAddressValue(temp+0xA0, &temp, sizeof (DWORD));
	GetAddressValue(temp+0x0, &temp, sizeof (DWORD));
	GetAddressValue(temp+0xC, &temp, sizeof (DWORD));
	GetAddressValue(temp+0x8, &temp, sizeof (DWORD));
	GetAddressValue(temp+0x0, &temp, sizeof (DWORD));
	GetAddressValue(temp+0xAC, &temp, sizeof (DWORD));
	GetAddressValue(temp+0x18, &temp, sizeof (DWORD));
	GetAddressValue(temp+0x1C, &temp, sizeof (DWORD));
	GetAddressValue(temp+0xC, &temp, sizeof (DWORD));
	GetAddressValue(temp+0x64, &temp, sizeof (DWORD));*/
	
	GetBaseAddressValue(LibTersafeModuleBase + 0x353068, &temp, sizeof (DWORD),
		0xE4,
		0x29C,
		0x8,
		0x7AC,
		0x4,
		0x38,
		0x1C,
		0xC,
		0x64,
		0);
	
	return temp;
}

DWORD GetIsFireButtonPressedAddress()
{
	DWORD temp;
	GetBaseAddressValue(LibTersafeModuleBase + 0x353068, &temp, sizeof (DWORD),
		0xE4,
		0x29C,
		0x8,
		0x7AC,
		0x4,
		0x38,
		0x1C,
		0xC,
		0);
	
	return temp + 0x64;
}

void *AimBotThread()
{
	bool isDown = false;
	int kk = 0;
	double leenx = 0.0f;
	//x轴速度
	double leeny = 0.0f;
	//y轴速度
	
	double speedx = 20.0f;//150.0f;
	//速度值，此数值越大，速度越慢，抖动更稳定
	double de = 5.0f;
	
	double tx = SlideX, ty = SlideY;
	// 竖屏中心坐标:(1350，250)
	// 横屏坐标:(1350,830)
	// 中心到边:150 px
	// 这个是划屏范围
	
	float SpeedMin = 2.0f;
	
	double w = 0.0f, h = 0.0f, cmp = 0.0f;
	// 宽度 高度 正切
	
	double ScreenX = py*2, ScreenY = px*2;
	// 分辨率(竖屏)PS:滑屏用的坐标是竖屏状态下的
	
	double ScrXH = ScreenX / 2.0f;
	// 一半屏幕X
	
	double ScrYH = ScreenY / 2.0f;
	// 一半屏幕X
	
	int bu1,bu2;
	float bu3;
	
	//float Spe = 2.0f;
	
	RainyTouchDev dev;
	dev.SetTouchEventNum(3);
	dev.Touch_Init(3);
	
	DWORD isFireButtonPressed = 0, temp;
	// 开火按键是否被按下
	
	while (1)
	{
		findminat();
		if (Gmin == -1)
		    continue;
		if (zm_x == 0 && zm_y == 0)
		{
			continue;
		}
		if (ToReticleDistance <= AimRadius)
		{
			isFireButtonPressed = GetIsFireButtonPressed();
			if (isFireButtonPressed == 0)
			    continue;
			if (isDown == false)	// 如果没有按下
			{
				dev.Touch_Down(tx, ty);
				isDown = true;
				// 赋值为已经按下
			}
			
			if (zm_x <= 0 || zm_x >= ScreenX || zm_y <= 0 || zm_y >= ScreenY)
				continue;

			if (zm_x < ScrXH)
			{
				w = ScrXH - zm_x;
				// 敌人到准星的横向距离
				leenx = w / speedx;
				if (leenx < 3.0f)
				    leenx = ToReticleDistance / (speedx/de);
				if (leenx > 10.0f)
				    leenx = 10.0f;
				if (zm_y < ScrYH)
				{
					h = ScrYH - zm_y;
					// 敌人到准星的纵向距离
					if (w > SpeedMin)
					    dev.Touch_Move(tx, ty -= leenx);
					// 视角向左移动，敌人向右移动
				}
				else
				{
					h = zm_y - ScrYH;
					// 敌人到准星的纵向距离
					if (w > SpeedMin)
					    dev.Touch_Move(tx, ty += leenx);
					// 视角向右移动，敌人向左移动
				}
				cmp = w / h;
				// 正切值
				leeny = h / speedx * cmp;
				if (leeny < 3.0f)
				    leeny = ToReticleDistance / (speedx/de);
				if (leeny > 10.0f)
				    leeny = 10.0f;
				if (h > 1.0f)
				    dev.Touch_Move(tx -= leeny, ty);
				// 视角向下移动，敌人向上移动
			}
			else if (zm_x > ScrXH)
			{
				w = zm_x - ScrXH;
				leenx = w / speedx;
				if (leenx < 3.0f)
				    leenx = ToReticleDistance / (speedx/de);
				if (leenx > 10.0f)
				    leenx = 10.0f;
				if (zm_y < ScrYH)
				{
					h = ScrYH - zm_y;
					// 敌人到准星的纵向距离
					if (w > SpeedMin)
					    dev.Touch_Move(tx, ty -= leenx);
					// 视角向左移动，敌人向右移动
				}
				else
				{
					h = zm_y - ScrYH;
					// 敌人到准星的纵向距离
					if (w > SpeedMin)
					    dev.Touch_Move(tx, ty += leenx);
					// 视角向右移动，敌人向左移动
				}
				cmp = w / h;
				// 正切值
				leeny = h / speedx * cmp;
				if (leeny < 3.0f)
				    leeny = ToReticleDistance / (speedx/de);
				if (leeny > 10.0f)
				    leeny = 10.0f;
				if (h > 1.0f)
				    dev.Touch_Move(tx += leeny, ty);
				// 视角向下移动，敌人向上移动
			}
			
			if (tx >= SlideX+SlideRadius || tx <= SlideX-SlideRadius || ty >= SlideY+SlideRadius || ty <= SlideY-SlideRadius)
			{
				// 只要滑屏达到了边界，直接还原至中心
				tx = SlideX, ty = SlideY;
				// 恢复变量
				dev.Touch_Up();
				// 抬起
				usleep(18000);
				dev.Touch_Down(tx, ty);
				// 按下
				//dev.Touch_Move(tx, ty);
			}
		}
		else if (AimMode == 1 && isFireButtonPressed == 0)
		{
			if (isDown == true)
			{
				tx = SlideX, ty = SlideY;
				// 恢复变量
				usleep(18000);
				dev.Touch_Up();
				// 抬起
				isDown = false;
			}
		}
		else if (AimMode == 2 && ToReticleDistance > AimRadius)
		{
			if (isDown == true)
			{
				tx = SlideX, ty = SlideY;
				// 恢复变量
				usleep(18000);
				dev.Touch_Up();
				// 抬起
				isDown = false;
			}
		}
		usleep(25000);
		//延迟别改，会影响到速度
	}
}

long int get_module_base(int pid, const char *module_name)
{
	FILE *fp;
	long addr = 0;
	char *pch;
	char filename[32];
	char line[1024];
	snprintf(filename, sizeof(filename), "/proc/%d/maps", pid);
	fp = fopen(filename, "r");
	if (fp != NULL)
	{
		while (fgets(line, sizeof(line), fp))
		{
			if (strstr(line, module_name))
			{
				pch = strtok(line, "-");
				addr = strtoul(pch, NULL, 16);
				if (addr == 0x8000)
					addr = 0;
				break;
			}
		}
		fclose(fp);
	}
	return addr;
}

int main(int argc, char **argv)
{
	initMemoryTools(argv,(char*)"com.tencent.tmgp.cf",MODE_ROOT,(char*)"");
	
	// 分辨率获取
	FILE *fp = fopen("/storage/emulated/0/DevInfo.in","r");
	//此文件为绘制插件自动生成的设备信息文件
	if (fp == NULL)
	{
		px = 1124;
		py = 540;
		bar = 89;
		SlideX = 830.0f;
		SlideY = 1350.0f;
	}
	else
	{
		fscanf(fp,"%f,%f,%f",&px,&py,&bar);
		if (px < py)
		{
			//px必须要比py大
			//px是横屏长度，py是横屏高度
			float a;
			a = px;
			px = py;
			py = a;
		}
		
		SlideX = py*(780.0f/1080.0f);
		//滑屏坐标采用竖屏的坐标，所以这里用的py
		SlideY = px*(1500.0f/2248.0f);
		
		px /= 2;
		py /= 2;
		zxx = px;
		zxy = py;
		
		fclose(fp);
		#ifdef DEBUG
		printf("分辨率:%f,%f,%f\n",px,py,bar);
		#endif
	}
	RainyShareMemory r;
	r.CreateShareMemory(2048);
	// 创建共享内存
	
	DWORD Pid = get_pid_by_packagename("com.tencent.tmgp.cf");
	
	LibUnityModuleBase = get_module_base(Pid,"libunity.so");
	LibMonoModuleBase = get_module_base(Pid,"libmono.so");
	LibTersafeModuleBase = get_module_base(Pid,"libtersafe.so");
	
	printf("UnityBase:0x%X MonoBase:0x%X Pid:%d\n",LibUnityModuleBase,LibMonoModuleBase,Pid);
	
	DWORD MatrixAddress = 0;
	// 矩阵
	DWORD ActorArray = 0;
	// 对象数组
	DWORD ActorCount = 0;
	// Actor数量
	
	float ActorXPosition, ActorYPosition, ActorZPosition;
	// 对象xyz
	float ActorHealth, ActorDistance;
	// 对象血量，距离临时变量
	float tmp[3], tmpptr;
	// 临时数组,临时指针
	char TempDrawData[2048] = {0};
	char TempArray[128] = {0};
	// 临时绘制数据以及临时数组
	
	R_Canvas canvas;
	R_Paint paint1,paint2,paint3;
	canvas.InitCanvas(CANVAS_MEM);
	
	paint1.setAntiAlias(true);
	paint1.setAlpha(180);
	paint1.setStrokeWidth(2);
	paint1.setStyle(STYLE_STROKE);
	paint1.setTextSize(25);
	
	paint2.setAntiAlias(true);
	paint2.setAlpha(180);
	paint2.setStrokeWidth(8);
	paint2.setStyle(STYLE_STROKE);
	paint2.setColor(PAINT_COLOR_GREEN);
	
	paint3.setColor(PAINT_COLOR_WHITE);
	paint3.setStrokeWidth(2);
	paint3.setStyle(STYLE_FILL);
	paint3.setAlpha(200);
	paint3.setAntiAlias(true);
	paint3.setTextSize(25);
	
	std::thread thr1(AimBotThread);
	//自瞄线程
	
	DWORD MyTeam;
	//自己队伍
	
	DWORD ActorTeam;
	//玩家队伍
	
	while (1)
	{
		DWORD Firing = GetIsFireButtonPressed();
		
		AimCount = 0;
		AimObjCount = 0;
		PlayerCount = 0;
		
		GetBaseAddressValue(LibUnityModuleBase + 0x13B7938, &MatrixAddress, sizeof (DWORD), 0xC, 0x10, 0);
		MatrixAddress += 0xA8;
		// 矩阵地址
		
		memset(TempDrawData, 0, 2048);
		GetAddressValue(MatrixAddress,&CameraMatrix,64);
		// 读取矩阵
		
		//GetAddressValue(ActorArray-0x8, &ActorCount, sizeof (DWORD));
		// 读取ActorCount
		GetBaseAddressValue(LibMonoModuleBase + 0x3B4D20, &ActorArray, sizeof (DWORD), 
			0x10, 
			0x80,
			0);
		//printf("ActorArr:%x\n",ActorArray);
		GetAddressValue(ActorArray,&ActorArray,4);
		//printf("ActorArr:%x\n",ActorArray);
		GetBaseAddressValue(ActorArray + 0x10, &ActorArray, sizeof (DWORD), 
			0x8, 
			0x1C,
			0x1C,
			0x10,
			0x24,
			0x8,
			0);
		ActorArray += 0x10;
		// 数组地址
		
		GetAddressValue(ActorArray - 4, &ActorCount, 4);
		// 获取数组长度
		//printf("ActorArr:%x\n",ActorArray);
		for (int i = 0; i < ActorCount; i++)
		{
			/*if (i % 2 == 0)
			    continue;*/
			DWORD ActorCoords,ActorStruct;
			DWORD IsSelf;
			
			GetAddressValue(ActorArray + i*4, &ActorStruct, sizeof (DWORD));
			// 获取对象指针
			if (ActorStruct == 0)
			    continue;
			
			GetBaseAddressValue(ActorArray + i*4, &ActorTeam, sizeof (DWORD),
				    0xF0, 
				    0x13C,
				    0);
				// 读阵营
			if (ActorTeam == MyTeam && ActorTeam != 0 && MyTeam != 0)
			    continue;
			
			GetBaseAddressValue(ActorArray + i*4, &ActorHealth, sizeof (DWORD), 0x40, 0);
			// 读对象血量
			
			if (ActorHealth <= 0.0f || ActorHealth > 120.0f)
			    continue;
			// 过滤
			
			GetBaseAddressValue(ActorArray + i*4, &IsSelf, sizeof (DWORD), 480, 0);
			// 判断自身
			
			if (IsSelf != 0)
			{
				GetBaseAddressValue(ActorArray + i*4, &MyTeam, sizeof (DWORD),
				    0xF0, 
				    0x13C,
				    0);
				// 读阵营
			}
			
			GetBaseAddressValue(ActorArray + i*4, &ActorCoords, sizeof (FLOAT), 0x724, 0x14, 0x8, 0);
			// 获取对象坐标指针
			
			GetAddressValue(ActorCoords + 0x60, tmp, 12);
			// 读对象坐标
			
			ActorXPosition = tmp[0];
			ActorZPosition = tmp[1];
			ActorYPosition = tmp[2];
			// 保存对象坐标
			
			if (ActorXPosition == 0 || ActorYPosition == 0 || ActorZPosition == 0)
			    continue;
			// 过滤无用数据
			//printf("x:%.5f, y:%.5f, z:%.5f, hp:%.1f\n",tmp[0], tmp[2], tmp[1], ActorHealth);
			
			float CameraDistance =
				CameraMatrix[3] * ActorXPosition + CameraMatrix[7] * ActorZPosition + CameraMatrix[11] * ActorYPosition + CameraMatrix[15];
			
			if (CameraDistance <= 1.0f || CameraDistance > 100.0f)
			    continue;
			
			float r_x =
				px + (CameraMatrix[0] * ActorXPosition + CameraMatrix[4] * (ActorZPosition) + CameraMatrix[8] * ActorYPosition +
					  CameraMatrix[12]) / CameraDistance * px;

			float r_y =
				py - (CameraMatrix[1] * ActorXPosition + CameraMatrix[5] * (ActorZPosition + 0.5f) + CameraMatrix[9] * ActorYPosition +
					  CameraMatrix[13]) / CameraDistance * py;

			float r_w =
				py - (CameraMatrix[1] * ActorXPosition + CameraMatrix[5] * (ActorZPosition - 0.5f) + CameraMatrix[9] * ActorYPosition +
					  CameraMatrix[13]) / CameraDistance * py;
			float x = r_x;
			float y = r_y;
			float w = (r_w - r_y) / 2;
			float h = r_w - r_y;
			if (h < 0)
			    continue;
			    
			Aim[AimCount].WorldDistance = CameraDistance;
			//世界距离
			Aim[AimCount].x = x;
			//x
			switch (AimPosition)
			{
				case position.POSITION_HEAD:
				    if (Firing != 0 && AutoPressure == 1)//开火键被按下
				    {
				    	Aim[AimCount].y = py*2 - (y+w/1.0);
						Aim[AimCount].ScreenDistance = sqrt(pow(zxx-x,2) + pow(zxy-(y+w/1.0),2));
						//到准星距离
				    }
				    else
				    {
				    	Aim[AimCount].y = py*2 - (y+w/2.5);
						Aim[AimCount].ScreenDistance = sqrt(pow(zxx-x,2) + pow(zxy-(y+w/2.5),2));
						//到准星距离
				    }
					break;
				case position.POSITION_BODY:
				    if (Firing != 0 && AutoPressure == 1)//开火键被按下
				    {
				    	Aim[AimCount].y = py*2 - (y+w/0.5);
						Aim[AimCount].ScreenDistance = sqrt(pow(zxx-x,2) + pow(zxy-(y+w/0.5),2));
						//到准星距离
				    }
				    else
				    {
				    	Aim[AimCount].y = py*2 - (y+w);
						Aim[AimCount].ScreenDistance = sqrt(pow(zxx-x,2) + pow(zxy-(y+w),2));
						//到准星距离
				    }
				    break;
				case position.POSITION_LEG:
					if (Firing != 0 && AutoPressure == 1)//开火键被按下
				    {
				    	Aim[AimCount].y = py*2 - (y+w*3.3);
						Aim[AimCount].ScreenDistance = sqrt(pow(zxx-x,2) + pow(zxy-(y+w*3.3),2));
						//到准星距离
				    }
				    else
				    {
				    	Aim[AimCount].y = py*2 - (y+w*1.7);
						Aim[AimCount].ScreenDistance = sqrt(pow(zxx-x,2) + pow(zxy-(y+w*1.7),2));
						//到准星距离
				    }
				    break;
				
			}
			//自瞄y位置
			AimCount++;
				
			canvas.drawLine(px-bar,0,x-bar,y,paint1);
			canvas.drawRect(x-w/2-bar,y,x+w/2-bar,y+h,paint1);
			canvas.drawLine(x+w/2+10-bar,y+h,x+w/2+10-bar,y+h-h*(ActorHealth/100.0f),paint2);
			
			char t[256];
			sprintf(t,"血量:%.1f 距离:%.1f",ActorHealth,CameraDistance);
			canvas.drawText(x-w/2-bar,y+h+5,t,paint1);
			
			sprintf(t,"对象地址:0x%X",ActorStruct);
			canvas.drawText(x-w/2-bar,y+h+30,t,paint1);
			/*sprintf(TempArray, "%f,%f,%f,%f,%.0f;\n", r_x, r_y, (r_w - r_y) / 2, CameraDistance, ActorHealth);
			strcat(TempDrawData, TempArray);*/
		}
		
		/*
		sprintf(TempArray,"实体数量: %d",ActorCount);
		canvas.drawText(200,270,TempArray,paint3);
		*/
		DrawInfo(&canvas);
		MaxPlayerCount = AimCount;
		
		canvas.refresh();
		//r.WriteShareMemoryData(TempDrawData,2048);
		usleep(500);
	}
}

void DrawInfo(R_Canvas *canvas)
{
	R_Paint paint,paint1,paint2;
	paint.setColor(PAINT_COLOR_RED);
	paint.setStrokeWidth(2);
	paint.setStyle(STYLE_STROKE);
	paint.setAlpha(150);
	paint.setAntiAlias(true);
	
	paint1.setColor(PAINT_COLOR_WHITE);
	paint1.setStrokeWidth(2);
	paint1.setStyle(STYLE_FILL);
	paint1.setAlpha(200);
	paint1.setAntiAlias(true);
	paint1.setTextSize(25);
	
	paint2.setStrokeWidth(2);
	paint2.setStyle(STYLE_STROKE);
	paint2.setAlpha(150);
	paint2.setAntiAlias(true);
	
	DWORD temp;
	
	isPressing = GetIsFireButtonPressed();
	temp = GetIsFireButtonPressedAddress();
	
	char t[256];
	switch (AimPosition)
	{
		case position.POSITION_HEAD:
			canvas->drawText(200,150,"自瞄位置: 头部",paint1);
		break;
		
		case position.POSITION_BODY:
		    canvas->drawText(200,150,"自瞄位置: 胸部",paint1);
		break;
		
		case position.POSITION_LEG:
			canvas->drawText(200,150,"自瞄位置: 脚部",paint1);
		break;
	}
	
	sprintf(t,"自瞄范围: %.1f",AimRadius);
	canvas->drawText(200,180,t,paint1);
	
	sprintf(t,"自动压枪: %s",AutoPressure == 1 ? "开启":"关闭");
	canvas->drawText(200,210,t,paint1);
	
	/*sprintf(t,"开火地址: %X (%d)",temp,isPressing);
	canvas->drawText(200,240,t,paint1);*/
	
	canvas->drawRect(SlideY-SlideRadius-bar,py*2-(SlideX-SlideRadius),SlideY+SlideRadius-bar,py*2-(SlideX+SlideRadius),paint2);
	canvas->drawCircle(px-bar,py,AimRadius,paint);
}