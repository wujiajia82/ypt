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
	//启用独立线程串口初始化	
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
	

	//视频采集初始化
	try {
		targetTrack.init(CapPort, wname);
	} catch (Exception &e) {
		cout << e.what() << endl;
		return -1;
	}

	//将串口设置到图像采集中
	targetTrack.setSerialPort(&serialPort);
	return 0;
}
void CYptApp::run()
{
	//主线程处理图像采集
	targetTrack.loop();
}

CYptApp::~CYptApp()
{
}
