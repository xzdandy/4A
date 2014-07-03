#include "recognition.h"

Recognition::Recognition(CvRect rect, pair<int, int> range, int isClip){
	ifstream fin;
	string line;
	this->rect = rect;
	this->isClip = isClip;
	this->minArea = range.first;
	this->maxArea = range.second;

	fin.open("config/keycodetable.txt", ifstream::in);
	keycodetable.clear();
	while (getline(fin, line)){
		auto word = split(line);
		keycodetable.emplace(word[0], stoi(word[1],0,16));
	}
	fin.close();

	fin.open("config/keymaptable.txt", ifstream::in);
	keymaptable.clear();
	while (getline(fin, line)){
		auto word = split(line);
		keymaptable.emplace_back(make_pair(make_pair(stoi(word[0]), stoi(word[1])), word[2]));
	}
	fin.close();
}
Recognition::~Recognition(){}

vector<string> Recognition::split(string s){
	vector<string> res;
	string temp;

	res.clear();
	temp = "";
	for (int i = 0; i < s.length(); i++){
		if (s[i] == ' ' && temp.length() == 0) continue;
		else if (s[i] == ' ' && temp.length() > 0){
			res.emplace_back(temp);
			temp = "";
		}
		else{
			temp += s[i];
		}
	}
	if (temp.length() > 0){
		res.emplace_back(temp);
	}

	return res;
}

vector<INFO> Recognition::getOneFrame(IplImage *img, int debug){

	vector<INFO> info;
	vector<CvRect> rect_container;
	int thresh;
	CvSeq *first_contour = NULL;
	CvMemStorage *pStorage = NULL;

	if (isClip)	img = getSensitiveArea(img);

	//转换为灰度图
	IplImage *grey = cvCreateImage(cvGetSize(img), IPL_DEPTH_8U, 1);
	cvCvtColor(img, grey, CV_BGR2GRAY);
	thresh = Otsu(grey) > 30 ? Otsu(grey) : 30;
	cvThreshold(grey, grey, thresh, 255, CV_THRESH_BINARY);

	pStorage = cvCreateMemStorage(0);
	cvFindContours(grey, pStorage, &first_contour, sizeof(CvContour), CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);

	for (; first_contour != 0; first_contour = first_contour->h_next){
		CvRect rect = cvBoundingRect(first_contour, 0);
		rect_container.emplace_back(rect);
	}

	if (minArea != -1 || maxArea != -1)	rect_container = rectFilter(rect_container, minArea, maxArea);
	info = infoFormat(rect_container, debug);

	if (debug && rect_container.size() > 0) {
		//渲染效果
		IplImage *imgOut = cvCreateImage(cvGetSize(img), IPL_DEPTH_8U, 3); cvZero(imgOut);
		cvCopy(img, imgOut, NULL);

		for (auto &rect : rect_container){
			cvRectangle(imgOut, cvPoint(rect.x, rect.y), cvPoint(rect.x + rect.width, rect.y + rect.height), CV_RGB(255, 0, 0), 1, 8, 0);
		}

		//显示
		cvSaveImage("images/temp.jpg", imgOut);
		cvNamedWindow("test", 1);
		cvWaitKey(500);
		cvShowImage("test", imgOut);
		cvWaitKey(0);
		cvDestroyWindow("test");
		cvReleaseImage(&imgOut);
	}

	cvReleaseImage(&grey);
	cvReleaseMemStorage(&pStorage);

	return info;
}

IplImage* Recognition::getSensitiveArea(IplImage *img){
	IplImage *temp;
	cvSetImageROI(img, rect);
	temp = cvCreateImage(cvGetSize(img), img->depth, img->nChannels);
	cvResize(img, temp, CV_INTER_LINEAR);
	return temp;
}

vector<PAIR> Recognition::formatToPair(vector<INFO> info)
{
	vector<PAIR> res;
	for (auto& x : info){
		res.emplace_back(PAIR(x["posX"], x["posY"]));
	}
	return res;
}

string Recognition::keymap(PAIR pos)
{
	string key = "undefined";
	double minDistance = 400, temp;

	for (auto& x : keymaptable){
		temp = pow((pos.first - x.first.first), 2) + pow((pos.second - x.first.second), 2);
		if (temp < minDistance){
			key = x.second;
			minDistance = temp;
		}
	}

	return key;
}

vector<int> Recognition::formatToKey(vector<INFO> info)
{
	vector<int> res;

	auto keyPair = formatToPair(info);

	for (auto& x : keyPair){
		res.emplace_back(keycodetable[keymap(x)]);
	}
	return res;
}

vector<INFO> Recognition::infoFormat(vector<CvRect> rect_container, int debug)
{
	vector<INFO> info;
	int cnt = 1;
	//输出调试信息
	for (auto &rect : rect_container)
	{
		INFO temp; temp.clear();
		temp.emplace("area", rect.height * rect.width);
		temp.emplace("posX", rect.x + rect.width / 2);
		temp.emplace("posY", rect.y + rect.height / 2);
		info.emplace_back(temp);

		if (debug){
			cout << "Blob #" << dec << cnt << ": Area=" << rect.height * rect.width << ", Centroid=(" << rect.x + rect.width / 2 << ", " << rect.y + rect.height / 2 << ")";
			cout << ", Key: " << keymap(PAIR(rect.x + rect.width / 2, rect.y + rect.height / 2)) << " ";
			cout << "0x" << hex << setfill('0') << setw(2) << keycodetable[keymap(PAIR(rect.x + rect.width / 2, rect.y + rect.height / 2))] << endl;
			cnt++;
		}
	}
	return info;
}

int Recognition::Otsu(IplImage* src){
	int height = src->height;
	int width = src->width;
	//histogram        
	float histogram[256] = { 0 };
	for (int i = 0; i < height; i++)
	{
		unsigned char* p = (unsigned char*)src->imageData + src->widthStep * i;
		for (int j = 0; j < width; j++)
		{
			histogram[*p++]++;
		}
	}
	//normalize histogram        
	int size = height * width;
	for (int i = 0; i < 256; i++)
	{
		histogram[i] = histogram[i] / size;
	}

	//average pixel value        
	float avgValue = 0;
	for (int i = 0; i < 256; i++)
	{
		avgValue += i * histogram[i];  //整幅图像的平均灰度      
	}

	int threshold;
	float maxVariance = 0;
	float w = 0, u = 0;
	for (int i = 0; i < 256; i++)
	{
		w += histogram[i];  //假设当前灰度i为阈值, 0~i 灰度的像素(假设像素值在此范围的像素叫做前景像素) 所占整幅图像的比例      
		u += i * histogram[i];  // 灰度i 之前的像素(0~i)的平均灰度值： 前景像素的平均灰度值      

		float t = avgValue * w - u;
		float variance = t * t / (w * (1 - w));
		if (variance > maxVariance)
		{
			maxVariance = variance;
			threshold = i;
		}
	}
	return threshold;
}

vector<CvRect> Recognition::rectFilter(vector<CvRect> rect_container, int minArea, int maxArea){
	int area;
	vector<CvRect> res;
	for (auto &rect : rect_container){
		area = rect.width * rect.height;
		if (area >= minArea && area <= maxArea) res.emplace_back(rect);
	}
	return res;
}