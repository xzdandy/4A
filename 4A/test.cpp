// Copyright (C) 2007 by Cristóbal Carnero Liñán
// grendel.ccl@gmail.com
//
// This file is part of cvBlob.
//
// cvBlob is free software: you can redistribute it and/or modify
// it under the terms of the Lesser GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// cvBlob is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// Lesser GNU General Public License for more details.
//
// You should have received a copy of the Lesser GNU General Public License
// along with cvBlob.  If not, see <http://www.gnu.org/licenses/>.
//
/*
#include <iostream>

#include <vector>
#include <string>
#include <unordered_map>


using namespace std;

#if (defined(_WIN32) || defined(__WIN32__) || defined(__TOS_WIN__) || defined(__WINDOWS__) || (defined(__APPLE__) & defined(__MACH__)))
#include <cv.h>
#include <highgui.h>
#else
#include <opencv/cv.h>
#include <opencv/highgui.h>
#endif

#include <cvblob.h>
using namespace cvb;

void recognize(IplImage *img);
IplImage* getSensitiveArea(IplImage *img);
std::vector<std::unordered_map<std::string, double>> infoFormat(CvBlobs blobs);
std::vector<std::pair<double, double>> formatToPair(std::vector<std::unordered_map<std::string, double>> info);
std::string keymap(std::pair<double, double> pos);
std::vector<int> formatToKey(std::vector<std::unordered_map<std::string, double>> info);*/


#include "recognition.h"

int main()
{
	IplImage *img;
	Recognition *reco = new Recognition(cvRect(0, 180, 640, 150), {0,2500}, 0);
	//参数解释 （CvRect rect, pair<int,int> range, int isclip = 0）
	//第一个参数 CvRect为敏感区域，将只对敏感区域内的图片做识别
	//推荐 键盘的铭感区域为 cvRect(0, 180, 640, 150)
	//第二个参数 range 代表过滤范围 比如 {0, 2500}表示保留 面积在 0-2500内的之别点
	//推荐 键盘的过滤范围大概在 {10,50}待检验！
	//第三个参数 int isclip，是否根据第一个参数截取敏感区域，主要用于测试值设置为0
	//正式运行时推荐为将值设置为1，减少干扰。

	//获取摄像头
	CvCapture* capture = cvCaptureFromCAM(0);
	if (!cvGrabFrame(capture)) { 
		cout << "无法获得camera的帧" << endl;
		exit(0);
	}

	cvNamedWindow("camera", 1);
	cvWaitKey(0);
	while (true) {
		img = cvQueryFrame(capture); //从摄像头中获取一帧的图片
		cvShowImage("camera", img);
		cvWaitKey(2);
		auto info = reco->getOneFrame(img,1); //识别摄像头， 参数：(IplImage *img，int debug=0) debug代表是否开启调试模式，
											  //默认不开启，开启后会有图片和数据的输出
		auto keycode = reco->formatToKey(info); // 将信息转换成 vector<int>格式的按键16进制码
		auto point = reco->formatToPair(info);	// 将信息转换成 vector<pair<int,int>>格式的坐标位置
	}
	cvDestroyWindow("camera");

	cvReleaseCapture(&capture);
	
  return 0;
}

