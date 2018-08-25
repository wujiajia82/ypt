#include "pch.h"
#include<iostream>
#include<opencv2/opencv.hpp>
#include<opencv2/highgui/highgui.hpp>
#include<opencv2/core/core.hpp>
#include<opencv2/imgproc/imgproc.hpp>
#include<ctype.h>
using namespace std;
using namespace cv;

Mat image;         //��ǰ֡ͼ�� 
Mat imageCopy; //���ڿ����ĵ�ǰ֡ͼ��  
Mat rectImage;   //��ͼ��
Point beginPoint; //���ο����  
Point endPoint;  //���ο��յ�  
bool leftButtonDownFlag = false; //�����������Ƶ��ͣ���ŵı�־λ  
int frameCount = 0; //֡��ͳ�� 
int trackCount = 0;  //����1ʱ��ʼ��ֱ��ͼ
void onMouse(int event, int x, int y, int flags, void* ustc); //���ص�����  

int a() {

	//VideoCapture capture("C:\\Users\\14527\\Desktop\\Video\\emmm.AVI");
	VideoCapture capture(0);
	int capture_fps = capture.get(CV_CAP_PROP_FPS); //��ȡ��Ƶ֡�� 
	int capture_count = capture.get(CV_CAP_PROP_FRAME_COUNT);
	int capture_width = capture.get(CV_CAP_PROP_FRAME_WIDTH);
	int capture_height = capture.get(CV_CAP_PROP_FRAME_HEIGHT);
	cout << "��Ƶ֡�ʣ�" << capture_fps << endl;
	cout << "��Ƶ֡����" << capture_count << endl;
	cout << "��Ƶ��ȣ�" << capture_width << endl;
	cout << "��Ƶ�߶ȣ�" << capture_height << endl;
	int pauseTime = 1000 / capture_fps; //���������м���  
	//VideoWriter writer("C:\\Users\\14527\\Desktop\\Video\\out.avi", CV_FOURCC('X', 'V', 'I', 'D'), capture_fps, Size(capture_width, capture_height));
	namedWindow("Video");
	setMouseCallback("Video", onMouse);
	int vmin = 10, vmax = 256, smin = 30;//����HSV��V��S��ֵ
	int hbinNum = 16;//�Ҷȷּ�16
	float hranges[] = { 40,250 };
	const float* phranges = hranges;
	bool backprojectMode = false;

	namedWindow("Histogram", 0);
	namedWindow("Video", 0);
	createTrackbar("Vmin", "Video", &vmin, 256, 0);//createTrackbar�����Ĺ������ڶ�Ӧ�Ĵ��ڴ�����������������Vmin,vmin��ʾ��������ֵ�����Ϊ256  
	createTrackbar("Vmax", "Video", &vmax, 256, 0);//���һ������Ϊ0����û�е��û����϶�����Ӧ����  
	createTrackbar("Smin", "Video", &smin, 256, 0);//vmin,vmax,smin��ʼֵ�ֱ�Ϊ10,256,30  
	Mat hsvImg;//HSVͼ��
	capture >> image;
	Mat hue, mask, hist, histImg = Mat::zeros(image.size(), image.type()), backproj;
	Rect trackWindow;
	while (true) {
		if (!leftButtonDownFlag) //���������»��ƾ���ʱ����Ƶ��ͣ����  
		{
			capture >> image;
			frameCount++;   //֡��  
		}
		if (!image.data || waitKey(pauseTime + 30) == 27)  //ͼ��Ϊ�ջ�Esc�������˳�����  
		{
			break;
		}
		if (trackCount > 0) {
			cvtColor(image, hsvImg, CV_BGR2HSV);
			inRange(hsvImg, Scalar(0, smin, min(vmin, vmax)), Scalar(180, 256, max(vmin, vmax)), mask);
			int ch[] = { 0,0 };
			hue.create(hsvImg.size(), hsvImg.depth());//hue��ʼ��Ϊ��hsv��С���һ���ľ���  
			mixChannels(&hsvImg, 1, &hue, 1, ch, 1);//��hsv��һ��ͨ��(Ҳ����ɫ��)�������Ƶ�hue��  
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

//���ص�����    
void onMouse(int event, int x, int y, int flags, void *ustc)
{
	if (event == CV_EVENT_LBUTTONDOWN)
	{
		leftButtonDownFlag = true; //��־λ  
		beginPoint = Point(x, y);  //����������µ�ľ������  
		endPoint = beginPoint;
	}
	if (event == CV_EVENT_MOUSEMOVE && leftButtonDownFlag)
	{
		imageCopy = image.clone();
		endPoint = Point(x, y);
		if (beginPoint != endPoint)
		{
			//�ڸ��Ƶ�ͼ���ϻ��ƾ���  
			rectangle(imageCopy, beginPoint, endPoint, Scalar(0, 0, 255), 2);
		}
		imshow("Video", imageCopy);
	}
	if (event == CV_EVENT_LBUTTONUP)
	{
		leftButtonDownFlag = false;
		Mat subImage = image(Rect(beginPoint, endPoint)); //��ͼ��  
		rectImage = subImage.clone();
		trackCount = 1;
		//imshow("Sub Image", rectImage);
	}
}
