#pragma once
#include <windows.h>
#include <string>
#include <opencv2/opencv.hpp>

using namespace cv;


class Tools
{
public:
	Tools();
	~Tools();
	static void GetStringSize(HDC hDC, const char* str, int* w, int* h);
	static void putTextZH(Mat &dst, const char* str, Point org, Scalar color, int fontSize,
		const char *fn = "Arial", bool italic = false, bool underline = false);
};

