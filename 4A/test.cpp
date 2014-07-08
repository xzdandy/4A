#include "recognition.h"

int main()
{
	IplImage *img, *showImg;
	Recognition *reco = new Recognition(cvRect(0, 180, 610, 180), make_pair(10,2500), 1);
	//参数解释 （CvRect rect, pair<int,int> range, int isclip = 0）
	//第一个参数 CvRect为敏感区域，将只对敏感区域内的图片做识别
	//推荐 键盘的铭感区域为 cvRect(0, 180, 610, 180)
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
		showImg = reco->showSensitiveArea(img); //显示敏感区域
		cvShowImage("camera", showImg);
		cvWaitKey(2);
		auto info = reco->getOneFrame(img,0); //识别摄像头， 参数：(IplImage *img，int debug=0) debug代表是否开启调试模式，
											  //默认不开启，开启后会有图片和数据的输出
		auto keycode = reco->formatToKey(info); // 将信息转换成 vector<int>格式的按键16进制码
		auto point = reco->formatToPair(info);	// 将信息转换成 vector<pair<int,int>>格式的坐标位置
	}
	cvDestroyWindow("camera");

	cvReleaseCapture(&capture);
	
  return 0;
}

