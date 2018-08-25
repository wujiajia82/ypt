#include "pch.h"
#include "CTargetTrack.h"
#include "Tools.h"

void CTargetTrack::onKey(char ch)
{
	switch (ch)
	{
	case 'p'://��ͣ����ͼ
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
	
	static Point pre_pt(-1,-1)  ;//��ʼ����  
	static Point cur_pt(-1,-1)  ;//ʵʱ����  
	char temp[16];
	if (event == CV_EVENT_LBUTTONDOWN)//������£���ȡ��ʼ���꣬����ͼ���ϸõ㴦��Բ  
	{
		sprintf_s(temp, "(%d,%d)", x, y);
		pre_pt = Point(x, y);
		putText(image, temp, pre_pt, FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 0, 0, 255), 1, 8);//�ڴ�������ʾ����  
		//circle(image, pre_pt, 2, Scalar(255, 0, 0, 0), CV_FILLED, CV_AA, 0);//��Բ  		
	}
	else if (event == CV_EVENT_MOUSEMOVE && !(flags & CV_EVENT_FLAG_LBUTTON))//���û�а��µ����������ƶ��Ĵ�����  
	{
		sprintf_s(temp, "(%d,%d)", x, y);
		cur_pt = Point(x, y);
		putText(image, temp, cur_pt, FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 0, 0, 255));//ֻ��ʵʱ��ʾ����ƶ�������  
	}
	else if (event == CV_EVENT_MOUSEMOVE && (flags & CV_EVENT_FLAG_LBUTTON))//�������ʱ������ƶ�������ͼ���ϻ�����  
	{
		sprintf_s(temp, "(%d,%d)", x, y);
		cur_pt = Point(x, y);
		putText(image, temp, cur_pt, FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 0, 0, 255));
		rectangle(image, pre_pt, cur_pt, Scalar(0, 255, 0, 0), 1, 8, 0);//����ʱͼ����ʵʱ��ʾ����϶�ʱ�γɵľ���  
	}
	else if (event == CV_EVENT_LBUTTONUP)//����ɿ�������ͼ���ϻ�����  
	{
		sprintf_s(temp, "(%d,%d)", x, y);
		cur_pt = Point(x, y);
		putText(image, temp, cur_pt, FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 0, 0, 255));
		//circle(image, pre_pt, 2, Scalar(255, 0, 0, 0), CV_FILLED, CV_AA, 0);
		rectangle(image, pre_pt, cur_pt, Scalar(0, 255, 0, 0), 1, 8, 0);//���ݳ�ʼ��ͽ����㣬�����λ���img��  
		//��ȡ���ΰ�Χ��ͼ�񣬲����浽dst��  
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

//����ֱ��ͼ
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

	//ROIֱ��ͼ����
	Mat roi(hue, trackWindow);
	Mat maskroi(mask, trackWindow);
	float hrange[] = { 0, 180 };
	const float* hranges = hrange;
	//ֱ��ͼ
	calcHist(&roi, 1, 0, maskroi, hist, 1, &bins, &hranges);
	normalize(hist, hist, 0, 255, NORM_MINMAX);

	//����ֱ��ͼ�ķ�ͶӰ
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
		if (frame.empty() || (ch=waitKey(pauseTime)) == 27)  //ͼ��Ϊ�ջ�Esc�������˳�����  
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

		printf("����ͼ�������ĵ��ƫ��(%d,%d)\n",p.x,p.y);

		SerialData data;
		//if (serialPort->getRecvBuff(data)) { //��������·���������
		if (true) {
			//���������·�����


			//��ʾ��ͼƬ
			printf("��������[%s]\n", "");
			sprintf_s(temp, "���˻����꣺\n          ���ȡ�%-8s��\n          γ�ȡ�%-8s��\n          �߶ȡ�%-8s��", "", "","");
			//putText(frame, temp, Point(10,10), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(255, 255, 255), 4, 8);//�ڴ�������ʾ����  
			Tools::putTextZH(frame, temp, Point(10, 10), Scalar(0, 0, 255), 10, "Arial");

			sprintf_s(temp, "���������꣺\n          ���ȡ�%-8s��\n          γ�ȡ�%-8s��\n          �߶ȡ�%-8s��", "", "","");
			//putText(frame, temp, Point(10,10), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(255, 255, 255), 4, 8);//�ڴ�������ʾ����  
			Tools::putTextZH(frame, temp, Point(10, 60), Scalar(0, 0, 255), 10, "Arial");
		}

		


		printf("����[%c]\n", ch);
		imshow(wname, frame);
	}
}