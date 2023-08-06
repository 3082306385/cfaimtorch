#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <math.h>
#include <thread>
#include <iostream>
#include <dirent.h>
#include <sys/file.h>
#include <linux/input.h>
#include <sys/uio.h>

class RainyTouchDev
{
	public:
	    int Touch_Init(int FingerNum);
	    void Touch_Down(int x,int y);
	    void Touch_Up();
	    void Touch_Move(int x,int y);
	    void SetTouchEventNum(int num);
	private:
	    int TouchDeviceHandle;
	    FILE *DevFile;
	    int Finger;
	    int EventNum = -1;
	    int getTouchEventNum();
	    int Check();
	    int TrackingID;
	    void *wr;
};

int RainyTouchDev::Touch_Init(int FingerNum)
{
	if (this->EventNum <= 0)
	{
		this->EventNum = this->getTouchEventNum();
		if (this->EventNum < 0)
	    	return -1;
	}
	char tmp[256];
	sprintf(tmp,"/dev/input/event%d",this->EventNum);
	this->TouchDeviceHandle = open(tmp, O_RDWR);
	if (this->TouchDeviceHandle < 0)
		return -1;
	this->Finger = FingerNum;

	this->TrackingID = 90000;
	return 0;
}

int RainyTouchDev::getTouchEventNum()
{
	char name[64];
	char buf[256] = {0};
	int fd = 0;
	int i;
	for (i = 0; i < 32; i++)
	{
		sprintf(name, "/dev/input/event%d", i);
		if ((fd = open(name, O_RDONLY, 0)) >= 0)
		{
			ioctl(fd, EVIOCGPHYS(sizeof(buf)), buf);
			if (strstr(buf, "fts") != 0 || strstr(buf, "touch") != 0 || strstr(buf, "Touch"))
			{
				close(fd);
				return i;
			}
			close(fd);
		}
	}
	return -1;
}

void RainyTouchDev::SetTouchEventNum(int num)
{
	this->EventNum = num;
}

int RainyTouchDev::Check()
{
	struct input_event eve;
	loop:
	//eve = {0};
	memset(&eve,0,sizeof(input_event));
	int ret = read(this->TouchDeviceHandle,&eve,sizeof(input_event));
	if (eve.type != EV_SYN)
	{
		goto loop;
		//return -1;
	}
	return 0;
}

void RainyTouchDev::Touch_Down(int x, int y)
{
	struct input_event event;
	
	// init
	event.type = EV_KEY;
	event.code = BTN_TOUCH;
	gettimeofday(&event.time, 0);
	event.value = KEY_DOWN;
	write(this->TouchDeviceHandle, &event, sizeof(struct input_event));
	
	event.type = EV_KEY;
	event.code = BTN_TOOL_FINGER;
	gettimeofday(&event.time, 0);
	event.value = KEY_DOWN;
	write(this->TouchDeviceHandle, &event, sizeof(struct input_event));
	
	event.type = EV_SYN;
	event.code = SYN_REPORT;
	gettimeofday(&event.time, 0);
	event.value = 0;
	write(this->TouchDeviceHandle, &event, sizeof(struct input_event));
	
	event.type = EV_ABS;
	event.code = ABS_MT_SLOT;
	gettimeofday(&event.time, 0);
	event.value = Finger;
	write(this->TouchDeviceHandle, &event, sizeof(struct input_event));
	
	event.type = EV_ABS;
	event.code = ABS_MT_TRACKING_ID;
	gettimeofday(&event.time, 0);
	event.value = this->TrackingID;
	write(this->TouchDeviceHandle, &event, sizeof(struct input_event));
	
	event.type = EV_ABS;
	event.code = ABS_MT_POSITION_X;
	gettimeofday(&event.time, 0);
	event.value = x;
	write(this->TouchDeviceHandle, &event, sizeof(struct input_event));
	
	event.type = EV_ABS;
	event.code = ABS_MT_POSITION_Y;
	gettimeofday(&event.time, 0);
	event.value = y;
	write(this->TouchDeviceHandle, &event, sizeof(struct input_event));
	
	//1
	event.type = EV_ABS;
	event.code = ABS_MT_TOUCH_MAJOR;
	//gettimeofday(&event.time, 0);
	event.value = 4;
	write(this->TouchDeviceHandle, &event, sizeof(struct input_event));
	//1
	
	event.type = EV_SYN;
	event.code = SYN_REPORT;
	gettimeofday(&event.time, 0);
	event.value = 0;
	write(this->TouchDeviceHandle, &event, sizeof(struct input_event));
	// init
}

void RainyTouchDev::Touch_Move(int x, int y)
{
	struct input_event event[7] = {0};
	
	write(this->TouchDeviceHandle, event, sizeof(struct input_event)*3);
	
	event[0].type = EV_ABS;
	event[0].code = ABS_MT_SLOT;
	event[0].value = Finger;
	
	event[1].type = EV_ABS;
	event[1].code = ABS_MT_TRACKING_ID;
	event[1].value = 123;
	
	event[2].type = EV_KEY;
	event[2].code = BTN_TOUCH;
	event[2].value = KEY_DOWN;
	
	event[3].type = EV_ABS;
	event[3].code = ABS_MT_POSITION_X;
	event[3].value = x;
	
	event[4].type = EV_ABS;
	event[4].code = ABS_MT_POSITION_Y;
	event[4].value = y;
	
	event[5].type = EV_ABS;
	event[5].code = ABS_MT_TOUCH_MAJOR;
	event[5].value = 4;
	
	event[6].type = EV_SYN;
	event[6].code = SYN_REPORT;
	event[6].value = 0;
	
	write(this->TouchDeviceHandle, event, sizeof(struct input_event)*7);
}

void RainyTouchDev::Touch_Up()
{
	struct input_event event;

	event.type = EV_ABS;
	event.code = ABS_MT_SLOT;
	gettimeofday(&event.time, 0);
	event.value = Finger;
	write(this->TouchDeviceHandle, &event, sizeof(struct input_event));

	event.type = EV_ABS;
	event.code = ABS_MT_TRACKING_ID;
	gettimeofday(&event.time, 0);
	event.value = 0xFFFFFFFF;
	write(this->TouchDeviceHandle, &event, sizeof(struct input_event));

	event.type = EV_SYN;
	event.code = SYN_REPORT;
	gettimeofday(&event.time, 0);
	event.value = 0;
	write(this->TouchDeviceHandle, &event, sizeof(struct input_event));

	event.type = EV_KEY;
	event.code = BTN_TOUCH;
	gettimeofday(&event.time, 0);
	event.value = KEY_UP;
	write(this->TouchDeviceHandle, &event, sizeof(struct input_event));

	event.type = EV_KEY;
	event.code = BTN_TOOL_FINGER;
	gettimeofday(&event.time, 0);
	event.value = KEY_UP;
	write(this->TouchDeviceHandle, &event, sizeof(struct input_event));

	event.type = EV_SYN;
	event.code = SYN_REPORT;
	gettimeofday(&event.time, 0);
	event.value = 0;
	write(this->TouchDeviceHandle, &event, sizeof(struct input_event));
}