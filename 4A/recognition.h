#ifndef RECOGNITION_H
#define RECOGNITION_H

#define _CRT_SECURE_NO_DEPRECATE

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <unordered_map>

#include <opencv/cv.h>
#include <opencv/highgui.h>

using namespace std;

typedef std::unordered_map<std::string, int> INFO;
typedef std::pair<int, int> PAIR;
typedef std::vector<std::pair<PAIR, std::string>> KEYMAP;
typedef unordered_map<std::string, int> KEYCODE;

class Recognition
{
private:
	CvRect rect;
	KEYMAP keymaptable;
	KEYCODE keycodetable;
	int isClip, minArea, maxArea;
	vector<INFO> infoFormat(vector<CvRect> rect_container, int debug = 0);
	string keymap(PAIR pos);
	vector<string> split(string s);
	int Otsu(IplImage* src);
	vector<CvRect> rectFilter(vector<CvRect> rect_container, int minArea, int maxArea);
public:
	Recognition(CvRect rect, pair<int, int> range, int isClip = 0);
	IplImage* getSensitiveArea(IplImage *img);
	vector<INFO> getOneFrame(IplImage *img, int debug = 0);
	vector<PAIR> formatToPair(vector<INFO> info);
	vector<int> formatToKey(vector<INFO> info);
	~Recognition();
};

#endif