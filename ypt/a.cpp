#include "pch.h"
#include<iostream>
#include<opencv2/opencv.hpp>
#include<opencv2/highgui/highgui.hpp>
#include<opencv2/core/core.hpp>
#include<opencv2/imgproc/imgproc.hpp>
#include<ctype.h>
using namespace std;
using namespace cv;

Mat image;         //当前帧图像 
Mat imageCopy; //用于拷贝的当前帧图像  
Mat rectImage;   //子图像
Point beginPoint; //矩形框起点  
Point endPoint;  //矩形框终点  
bool leftButtonDownFlag = false; //左键单击后视频暂停播放的标志位  
int frameCount = 0; //帧数统计 
int trackCount = 0;  //等于1时初始化直方图
void onMouse(int event, int x, int y, int flags, void* ustc); //鼠标回调函数  

int a() {

	//VideoCapture capture("C:\\Users\\14527\\Desktop\\Video\\emmm.AVI");
	VideoCapture capture(0);
	int capture_fps = capture.get(CV_CAP_PROP_FPS); //获取视频帧率 
	int capture_count = capture.get(CV_CAP_PROP_FRAME_COUNT);
	int capture_width = capture.get(CV_CAP_PROP_FRAME_WIDTH);
	int capture_height = capture.get(CV_CAP_PROP_FRAME_HEIGHT);
	cout << "视频帧率：" << capture_fps << endl;
	cout << "视频帧数：" << capture_count << endl;
	cout << "视频宽度：" << capture_width << endl;
	cout << "视频高度：" << capture_height << endl;
	int pauseTime = 1000 / capture_fps; //两幅画面中间间隔  
	//VideoWriter writer("C:\\Users\\14527\\Desktop\\Video\\out.avi", CV_FOURCC('X', 'V', 'I', 'D'), capture_fps, Size(capture_width, capture_height));
	namedWindow("Video");
	setMouseCallback("Video", onMouse);
	int vmin = 10, vmax = 256, smin = 30;//设置HSV中V和S的值
	int hbinNum = 16;//灰度分级16
	float hranges[] = { 40,250 };
	const float* phranges = hranges;
	bool backprojectMode = false;

	namedWindow("Histogram", 0);
	namedWindow("Video", 0);
	createTrackbar("Vmin", "Video", &vmin, 256, 0);//createTrackbar函数的功能是在对应的窗口创建滑动条，滑动条Vmin,vmin表示滑动条的值，最大为256  
	createTrackbar("Vmax", "Video", &vmax, 256, 0);//最后一个参数为0代表没有调用滑动拖动的响应函数  
	createTrackbar("Smin", "Video", &smin, 256, 0);//vmin,vmax,smin初始值分别为10,256,30  
	Mat hsvImg;//HSV图像
	capture >> image;
	Mat hue, mask, hist, histImg = Mat::zeros(image.size(), image.type()), backproj;
	Rect trackWindow;
	while (true) {
		if (!leftButtonDownFlag) //鼠标左键按下绘制矩形时，视频暂停播放  
		{
			capture >> image;
			frameCount++;   //帧数  
		}
		if (!image.data || waitKey(pauseTime + 30) == 27)  //图像为空或Esc键按下退出播放  
		{
			break;
		}
		if (trackCount > 0) {
			cvtColor(image, hsvImg, CV_BGR2HSV);
			inRange(hsvImg, Scalar(0, smin, min(vmin, vmax)), Scalar(180, 256, max(vmin, vmax)), mask);
			int ch[] = { 0,0 };
			hue.create(hsvImg.size(), hsvImg.depth());//hue初始化为与hsv大小深度一样的矩阵  
			mixChannels(&hsvImg, 1, &hue, 1, ch, 1);//将hsv第一个通道(也就是色调)的数复制到hue中  
			if (trackCount == 1) {
				histImg = Scalar::all(0);
				Mat roi(hue, Rect(beginPoint, endPoint)), maskroi(mask, Rect(beginPoint, endPoint));
				calcHist(&roi, 1, 0, maskroi, hist, 1, &hbinNum, &phranges);
				normalize(hist, hist, 0, 255, CV_MINMAX);
				trackCount++;
				trackWindow = Rect(beginPoint, endPoint);
			}

			calcBackProject(&hue, 1, 0, hist, backproj, &phranges);
			backproj &= mask;
			meanShift(backproj, trackWindow, TermCriteria(CV_TERMCRIT_EPS | CV_TERMCRIT_ITER, 10, 1));
			if (backprojectMode) {
				cvtColor(backproj, image, CV_GRAY2BGR);
			}
			rectangle(image, Point(trackWindow.x, trackWindow.y), Point(trackWindow.x + trackWindow.width, trackWindow.y + trackWindow.height), Scalar(0, 0, 255), 1, CV_AA);
			trackCount++;

			//  writer << image;
		}
		imshow("Video", image);

	}
	waitKey(0);
	return 0;
}

//鼠标回调函数    
void onMouse(int event, int x, int y, int flags, void *ustc)
{
	if (event == CV_EVENT_LBUTTONDOWN)
	{
		leftButtonDownFlag = true; //标志位  
		beginPoint = Point(x, y);  //设置左键按下点的矩形起点  
		endPoint = beginPoint;
	}
	if (event == CV_EVENT_MOUSEMOVE && leftButtonDownFlag)
	{
		imageCopy = image.clone();
		endPoint = Point(x, y);
		if (beginPoint != endPoint)
		{
			//在复制的图像上绘制矩形  
			rectangle(imageCopy, beginPoint, endPoint, Scalar(0, 0, 255), 2);
		}
		imshow("Video", imageCopy);
	}
	if (event == CV_EVENT_LBUTTONUP)
	{
		leftButtonDownFlag = false;
		Mat subImage = image(Rect(beginPoint, endPoint)); //子图像  
		rectImage = subImage.clone();
		trackCount = 1;
		//imshow("Sub Image", rectImage);
	}
}
