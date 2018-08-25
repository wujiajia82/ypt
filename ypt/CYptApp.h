#pragma once
#include "CSerialPort.h"
#include "CTargetTrack.h"
class CYptApp
{
public:
	CYptApp();
	CYptApp(int SerialPort, int CapPort, string wname);
	~CYptApp();
	void	run();
	int		init(int SerialPort, int CapPort, string wname);
private:
	CSerialPort		serialPort;
	CTargetTrack	targetTrack;
};

