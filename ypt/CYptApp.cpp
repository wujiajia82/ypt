#include "pch.h"
#include "CYptApp.h"

CYptApp::CYptApp()
{

}
CYptApp::CYptApp(int SerialPort, int CapPort, string wname)
{
	init(SerialPort, CapPort, wname);
}

int		CYptApp::init(int SerialPort, int CapPort, string wname)
{
	//���ö����̴߳��ڳ�ʼ��	
	if (!serialPort.InitPort(SerialPort)){
		std::cout << "initPort fail !" << std::endl;
		return -1;
	} else {
		std::cout << "initPort success !" << std::endl;
	}
	if (!serialPort.OpenListenThread()){
		std::cout << "OpenListenThread fail !" << std::endl;
	}else{
		std::cout << "OpenListenThread success !" << std::endl;
	}
	

	//��Ƶ�ɼ���ʼ��
	try {
		targetTrack.init(CapPort, wname);
	} catch (Exception &e) {
		cout << e.what() << endl;
		return -1;
	}

	//���������õ�ͼ��ɼ���
	targetTrack.setSerialPort(&serialPort);
	return 0;
}
void CYptApp::run()
{
	//���̴߳���ͼ��ɼ�
	targetTrack.loop();
}

CYptApp::~CYptApp()
{
}
