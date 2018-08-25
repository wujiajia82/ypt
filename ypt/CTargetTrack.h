#pragma once
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include "CSerialPort.h"
#include <ctype.h>

using namespace std;
using namespace cv;

class CTargetTrack
{
public:
	static void    onMouse(int event, int x, int y, int flags, void* ustc); //鼠标回调函数  
	void	loop();
	CTargetTrack();
	CTargetTrack(int i,string windowname);
	~CTargetTrack();
	void init(int i, string windowname);
	void setSerialPort(CSerialPort* p);

private:
	void	onKey(char ch);
	
	void calc();

	int	comNo;
	const int pauseTime=10;
	string wname;	
	VideoCapture cap;
	CSerialPort	*serialPort;			//串口

	Rect	trackWindow;				//跟踪矩形
	Mat		frame;
};

