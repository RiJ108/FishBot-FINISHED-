#pragma once PROCESS_HPP

#include <iostream>
#include <vector>
#include <string>
#include <Windows.h>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

typedef struct HSI {
	int Hue_Lower_Value = 0, Hue_Upper_Value = 179;
	int Sat_Lower_Value = 0, Sat_Upper_Value = 255;
	int Value_Lower = 0, Value_Upper = 255;
}HSI;

class Process {
public:
	LPCWSTR windowTitle = L"New World";
	HWND hWND_desktop, hWND_game, tmp;
	Mat src, game_capture, center_capture, cc_clone, bait_capture;
	Mat gray_cc, gray_bc;
	Mat image_hsv;
	Mat mask_cc, masked_cc;
	Mat dura_capture;
	//Mat bp_image, bp_thresh;

	vector<Vec3f> circles;

	bool flag_csl = true;
	int gameCaptureRange[4] = { 0, 1085, 1920, 3840 };
	float centerCropRange[4] = {0.35, 0.65, 0.35, 0.55};
	float baitCropRange[4] = { 0.69, 0.72, 0.57, 0.582};
	float duraCropRange[4] = { 0.95, 0.96, 0.88, 0.975};

	HSI green, orange, red;
	
	void getHandlers();
	void captureDesktop();
	bool isGameWindowFocus();
	void cropZones();
	void processCenter(int min_radius, int max_radius);
	bool isRodOut();
	unsigned int getDurability();

	BITMAPINFOHEADER createBitmapHeader(int width, int height);
	Mat captureScreenMat(HWND hwnd);

	void setHSIs();
	bool isThereHUE(Mat src_hsv, HSI hsi);
	Mat filterHSI(Mat src_hsv, HSI hsi, int index);
	
private:
};
