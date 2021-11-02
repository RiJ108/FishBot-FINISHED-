#include <process.hpp>

unsigned int Process::getDurability() {
    Mat gray, image_dst;
    cvtColor(dura_capture, gray, COLOR_BGR2GRAY);
    threshold(gray, image_dst, 254, 255, 0);
    //imshow("Durability Thresh", image_dst); moveWindow("Durability Thresh", -1950 + (2 * 400), 600);
    return countNonZero(image_dst);
}

bool Process::isRodOut() {
    Mat gray, image_dst;
    cvtColor(bait_capture, gray, COLOR_BGR2GRAY);
    threshold(gray, image_dst, 254, 255, 0);
    if (countNonZero(image_dst) >= 10)
        return true;
    return false;
}

void Process::processCenter(int min_radius, int max_radius) {
    cvtColor(center_capture, gray_cc, COLOR_BGR2GRAY);
    medianBlur(gray_cc, gray_cc, 5);
    mask_cc = Mat::zeros(gray_cc.size(), gray_cc.type());
    circles.clear();
    HoughCircles(gray_cc, circles, HOUGH_GRADIENT, 1, gray_cc.rows / 16, 100, 30, min_radius, max_radius );

    masked_cc = Mat::zeros(center_capture.size(), center_capture.type());
    cc_clone = center_capture.clone();
    if (circles.size() != 0) {
        for (size_t i = 0; i < circles.size(); i++) {
            Vec3i c = circles[i];
            circle(mask_cc, Point(c[0], c[1]), c[2], Scalar(255, 0, 0), -1, 8, 0);
            Point center = Point(c[0], c[1]);
            circle(cc_clone, center, 1, Scalar(0, 150, 100), 3, LINE_AA);
            int radius = c[2];
            circle(cc_clone, center, radius, Scalar(255, 0, 255), 3, LINE_AA);
        }
        center_capture.copyTo(masked_cc, mask_cc);
        cvtColor(masked_cc, image_hsv, COLOR_BGR2HSV);
    }
    //imshow("Capture", center_capture); moveWindow("Capture", -1950, 700);
    imshow("None approximation", cc_clone); moveWindow("None approximation", -1950 + (2 * 400), 700);
    //imshow("Mask", mask_cc); moveWindow("Mask", -1950 + (2 * 400), 700);
    //imshow("Masked CC", masked_cc); moveWindow("Masked CC", -1950 + (2 * 400), 700 - ( 1 * 450));
}

bool Process::isGameWindowFocus() {
    if (GetActiveWindow() == hWND_game)
        return true;
    return false;
}

void Process::getHandlers() {
    hWND_desktop = GetDesktopWindow();
    hWND_game = FindWindow(NULL, windowTitle);
    while (!hWND_game) {
        if (flag_csl) {
            printf("Start the game...\n");
            flag_csl = false;
        }
        waitKey(1000);
        hWND_game = FindWindow(NULL, windowTitle);
    }
    printf("hWNDs set...\n");
}

void Process::cropZones() {
    game_capture = src(Range(gameCaptureRange[0], gameCaptureRange[1]), Range(gameCaptureRange[2], gameCaptureRange[3]));
    center_capture = game_capture(Range(game_capture.size().height * centerCropRange[0], game_capture.size().height * centerCropRange[1]), Range(game_capture.size().width * centerCropRange[2], game_capture.size().width * centerCropRange[3]));
    bait_capture = game_capture(Range(game_capture.size().height * baitCropRange[0], game_capture.size().height * baitCropRange[1]), Range(game_capture.size().width * baitCropRange[2], game_capture.size().width * baitCropRange[3]));
    dura_capture = game_capture(Range(game_capture.size().height * duraCropRange[0], game_capture.size().height * duraCropRange[1]), Range(game_capture.size().width * duraCropRange[2], game_capture.size().width * duraCropRange[3]) );

    //imshow("Game capture", game_capture);
    //imshow("Center capture", center_capture);
    //imshow("Bait capture", bait_capture);
    //imshow("Durability capture", dura_capture);
}

void Process::captureDesktop() {
    src = captureScreenMat(hWND_desktop);
}

void Process::setHSIs() {
    green.Hue_Lower_Value = 75, green.Hue_Upper_Value = 85, green.Sat_Lower_Value = 125;
    orange.Hue_Lower_Value = 11, orange.Hue_Upper_Value = 27, orange.Sat_Lower_Value = 101;
    red.Hue_Lower_Value = 130, red.Hue_Upper_Value = 179, red.Sat_Lower_Value = 40;
}

bool Process::isThereHUE(Mat src_hsv, HSI hsi) {
    Mat thresh, zero, diff;
    /*inRange(src_hsv, Scalar(hsi.Hue_Lower_Value, hsi.Sat_Lower_Value, hsi.Value_Lower), Scalar(hsi.Hue_Lower_Upper_Value, hsi.Sat_Upper_Value, hsi.Value_Upper), thresh);
    erode(thresh, thresh, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));//morphological opening for removing small objects from foreground//
    dilate(thresh, thresh, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));//morphological opening for removing small object from foreground//
    dilate(thresh, thresh, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));//morphological closing for filling up small holes in foreground//
    erode(thresh, thresh, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));//morphological closing for filling up small holes in foreground//*/
    thresh = filterHSI(src_hsv, hsi, -1);
    zero = Mat::zeros(thresh.size(), thresh.type());
    compare(thresh, zero, diff, CMP_NE);
    if (countNonZero(diff))
        return true;
    return false;
}

Mat Process::filterHSI(Mat src_hsv, HSI hsi, int index) {
    Mat thresh = Mat::zeros(src_hsv.size(), 0);
    inRange(src_hsv, Scalar(hsi.Hue_Lower_Value, hsi.Sat_Lower_Value, hsi.Value_Lower), Scalar(hsi.Hue_Upper_Value, hsi.Sat_Upper_Value, hsi.Value_Upper), thresh);
    erode(thresh, thresh, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));//morphological opening for removing small objects from foreground//
    dilate(thresh, thresh, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));//morphological opening for removing small object from foreground//
    dilate(thresh, thresh, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));//morphological closing for filling up small holes in foreground//
    erode(thresh, thresh, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));//morphological closing for filling up small holes in foreground//
    /*if (index >= 0)
        imshow("HSV Thresh" + to_string(index), thresh); moveWindow("HSV Thresh" + to_string(index), -1950 + (2 * 400), 700 - (index * 350));*/
    return thresh;
}

BITMAPINFOHEADER Process::createBitmapHeader(int width, int height) {
    BITMAPINFOHEADER  bi;
    bi.biSize = sizeof(BITMAPINFOHEADER);
    bi.biWidth = width; bi.biHeight = -height;  //this is the line that makes it draw upside down or not
    bi.biPlanes = 1; bi.biBitCount = 32;
    bi.biCompression = BI_RGB; bi.biSizeImage = 0;
    bi.biXPelsPerMeter = 0; bi.biYPelsPerMeter = 0;
    bi.biClrUsed = 0; bi.biClrImportant = 0;
    return bi;
}

Mat Process::captureScreenMat(HWND hwnd) {
    Mat src;
    HDC hwindowDC = GetDC(hwnd);
    HDC hwindowCompatibleDC = CreateCompatibleDC(hwindowDC);
    SetStretchBltMode(hwindowCompatibleDC, COLORONCOLOR);
    int screenx = GetSystemMetrics(SM_XVIRTUALSCREEN);
    int screeny = GetSystemMetrics(SM_YVIRTUALSCREEN);
    int width = GetSystemMetrics(SM_CXVIRTUALSCREEN);
    int height = GetSystemMetrics(SM_CYVIRTUALSCREEN);
    src.create(height, width, CV_8UC4);
    HBITMAP hbwindow = CreateCompatibleBitmap(hwindowDC, width, height);
    BITMAPINFOHEADER bi = createBitmapHeader(width, height);
    SelectObject(hwindowCompatibleDC, hbwindow);
    StretchBlt(hwindowCompatibleDC, 0, 0, width, height, hwindowDC, screenx, screeny, width, height, SRCCOPY);  //change SRCCOPY to NOTSRCCOPY for wacky colors !
    GetDIBits(hwindowCompatibleDC, hbwindow, 0, height, src.data, (BITMAPINFO*)&bi, DIB_RGB_COLORS);            //copy from hwindowCompatibleDC to hbwindow
    DeleteObject(hbwindow);
    DeleteDC(hwindowCompatibleDC);
    ReleaseDC(hwnd, hwindowDC);
    return src;
}