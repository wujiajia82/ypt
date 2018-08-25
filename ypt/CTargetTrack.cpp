#include "pch.h"
#include "CTargetTrack.h"
#include "Tools.h"

void CTargetTrack::onKey(char ch)
{
	switch (ch)
	{
	case 'p'://暂停并截图
		setMouseCallback(wname, NULL, NULL);
		trackWindow = selectROI(wname, frame);
		setMouseCallback(wname, CTargetTrack::onMouse, (void *)this);
		break;
	default:
		break;
	}
}

void CTargetTrack::onMouse(int event, int x, int y, int flags, void *udata)
{
	CTargetTrack *pTR = (CTargetTrack*)udata;
	Mat  image = pTR->frame.clone();
	
	static Point pre_pt(-1,-1)  ;//初始坐标  
	static Point cur_pt(-1,-1)  ;//实时坐标  
	char temp[16];
	if (event == CV_EVENT_LBUTTONDOWN)//左键按下，读取初始坐标，并在图像上该点处划圆  
	{
		sprintf_s(temp, "(%d,%d)", x, y);
		pre_pt = Point(x, y);
		putText(image, temp, pre_pt, FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 0, 0, 255), 1, 8);//在窗口上显示坐标  
		//circle(image, pre_pt, 2, Scalar(255, 0, 0, 0), CV_FILLED, CV_AA, 0);//划圆  		
	}
	else if (event == CV_EVENT_MOUSEMOVE && !(flags & CV_EVENT_FLAG_LBUTTON))//左键没有按下的情况下鼠标移动的处理函数  
	{
		sprintf_s(temp, "(%d,%d)", x, y);
		cur_pt = Point(x, y);
		putText(image, temp, cur_pt, FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 0, 0, 255));//只是实时显示鼠标移动的坐标  
	}
	else if (event == CV_EVENT_MOUSEMOVE && (flags & CV_EVENT_FLAG_LBUTTON))//左键按下时，鼠标移动，则在图像上划矩形  
	{
		sprintf_s(temp, "(%d,%d)", x, y);
		cur_pt = Point(x, y);
		putText(image, temp, cur_pt, FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 0, 0, 255));
		rectangle(image, pre_pt, cur_pt, Scalar(0, 255, 0, 0), 1, 8, 0);//在临时图像上实时显示鼠标拖动时形成的矩形  
	}
	else if (event == CV_EVENT_LBUTTONUP)//左键松开，将在图像上划矩形  
	{
		sprintf_s(temp, "(%d,%d)", x, y);
		cur_pt = Point(x, y);
		putText(image, temp, cur_pt, FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 0, 0, 255));
		//circle(image, pre_pt, 2, Scalar(255, 0, 0, 0), CV_FILLED, CV_AA, 0);
		rectangle(image, pre_pt, cur_pt, Scalar(0, 255, 0, 0), 1, 8, 0);//根据初始点和结束点，将矩形画到img上  
		//截取矩形包围的图像，并保存到dst中  
		int width = abs(pre_pt.x - cur_pt.x);
		int height = abs(pre_pt.y - cur_pt.y);
		if (width == 0 || height == 0)
		{
			printf("width == 0 || height == 0");
			return;
		}
		pTR->trackWindow = Rect(min(cur_pt.x, pre_pt.x), min(cur_pt.y, pre_pt.y), width, height);		
	}
	imshow(pTR->wname, image);
}

CTargetTrack::CTargetTrack()
{

}

CTargetTrack::CTargetTrack(int i, string windowname) 
{
	init(i, windowname);
}

void CTargetTrack::setSerialPort(CSerialPort* p)
{
	serialPort = p;
}

void CTargetTrack::init(int i, string windowname)
{
	comNo = i;
	wname = windowname;
	cap.open(comNo);
	cap.set(CV_CAP_PROP_FRAME_WIDTH, 640);
	cap.set(CV_CAP_PROP_FRAME_HEIGHT, 480);

	namedWindow(wname, 0);
	setMouseCallback(wname, CTargetTrack::onMouse, (void *)this);
}
CTargetTrack::~CTargetTrack()
{
}

//计算直方图
void CTargetTrack::calc()
{
	Mat mask, hist;
	Mat backproject;
	Mat hsvimage;
	int bins = 120;
	//Rect2d first = selectROI(wname, frame);

	cvtColor(frame, hsvimage, CV_BGR2HSV);
	inRange(hsvimage, Scalar(25, 43, 46), Scalar(35, 256, 256), mask);
	Mat hue = Mat(hsvimage.size(), hsvimage.depth());
	int channels[] = { 0, 0 };
	mixChannels(&hsvimage, 1, &hue, 1, channels, 1);

	//ROI直方图计算
	Mat roi(hue, trackWindow);
	Mat maskroi(mask, trackWindow);
	float hrange[] = { 0, 180 };
	const float* hranges = hrange;
	//直方图
	calcHist(&roi, 1, 0, maskroi, hist, 1, &bins, &hranges);
	normalize(hist, hist, 0, 255, NORM_MINMAX);

	//计算直方图的反投影
	calcBackProject(&hue, 1, 0, hist, backproject, &hranges);
	backproject &= mask;

	meanShift(backproject, trackWindow, TermCriteria(CV_TERMCRIT_EPS | CV_TERMCRIT_ITER, 10, 1));

}

void CTargetTrack::loop()
{
	char	temp[200];
	char	ch;
	Point	p;
	
	while (1) {
		cap >> frame;
		if (frame.empty() || (ch=waitKey(pauseTime)) == 27)  //图像为空或Esc键按下退出播放  
		{
			break;
		}
		onKey(ch);

		if (trackWindow.x != 0 ) {
			calc();
			rectangle(frame, trackWindow, Scalar(0, 0, 255), 1, CV_AA);
			int col = frame.cols / 2;
			int row = frame.rows / 2;
			p.x = trackWindow.x + trackWindow.width - col;
			p.y = trackWindow.y + trackWindow.height - row;
		}

		printf("跟踪图像与中心点的偏差(%d,%d)\n",p.x,p.y);

		SerialData data;
		//if (serialPort->getRecvBuff(data)) { //如果串口下发数据正常
		if (true) {
			//解析串口下发数据


			//显示到图片
			printf("串口数据[%s]\n", "");
			sprintf_s(temp, "无人机坐标：\n          经度【%-8s】\n          纬度【%-8s】\n          高度【%-8s】", "", "","");
			//putText(frame, temp, Point(10,10), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(255, 255, 255), 4, 8);//在窗口上显示坐标  
			Tools::putTextZH(frame, temp, Point(10, 10), Scalar(0, 0, 255), 10, "Arial");

			sprintf_s(temp, "跟踪物坐标：\n          经度【%-8s】\n          纬度【%-8s】\n          高度【%-8s】", "", "","");
			//putText(frame, temp, Point(10,10), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(255, 255, 255), 4, 8);//在窗口上显示坐标  
			Tools::putTextZH(frame, temp, Point(10, 60), Scalar(0, 0, 255), 10, "Arial");
		}

		


		printf("键盘[%c]\n", ch);
		imshow(wname, frame);
	}
}