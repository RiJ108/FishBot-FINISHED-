#include <iostream>
#include <vector>
#include <Windows.h>
#include <chrono>
#include <time.h>
#include <ctime>
#include <opencv2/opencv.hpp>
#include <process.hpp>

using namespace std;
using namespace cv;

enum class State {init, rodChecking, rodIn, rodOut, readyToCast, casting, castCheck, waitingForFish, reeling, fixingRod};

int g_slider, g_slider_max;

void leftClickDown(INPUT input, int xPos, int yPos);
void leftClickUp(INPUT input, int xPos, int yPos);
void rightClickDown(INPUT input, int xPos, int yPos);
void rightClickUp(INPUT input, int xPos, int yPos);

void pressKey(BYTE key);
void releaseKey(BYTE key);
bool checkKey(BYTE key);

void on_trackbar(int, void*){ if (g_slider % 2 == 0) g_slider = g_slider + 1; }

int main(){
    int min_radius = 27, min_radius_max = 100;
    int max_radius = 40, max_radius_max = 100;
    int max = 65000;
    /*int Hue_Lower_Value = 63, Hue_Upper_Value = 100;
    int Saturation_Lower_Value = 100, Saturation_Upper_Value = 255;
    int Value_Lower = 0, Value_Upper = 255;*/
    
    /*namedWindow("FBot", WINDOW_NORMAL); moveWindow("FBot", -1950, 20);
    createTrackbar("min_radius", "FBot", &min_radius, min_radius_max, on_trackbar);
    createTrackbar("max_radius", "FBot", &max_radius, max_radius_max, on_trackbar);
    createTrackbar("Hue_Lower", "FBot", &Hue_Lower_Value, 179);//track-bar for lower hue//
    createTrackbar("Hue_Upper", "FBot", &Hue_Upper_Value, 179);//track-bar for lower-upper hue//
    createTrackbar("Sat_Lower", "FBot", &Saturation_Lower_Value, 255);//track-bar for lower saturation//
    createTrackbar("Sat_Upper", "FBot", &Saturation_Upper_Value, 255);//track-bar for higher saturation//
    createTrackbar("Val_Lower", "FBot", &Value_Lower, 255);//track-bar for lower value//
    createTrackbar("Val_Upper", "FBot", &Value_Upper, 255);//track-bar for upper value//*/
    
    Process process;
    process.getHandlers();
    process.setHSIs();
    bool flag = true;
    bool flag_csl = true;
    bool reeling_flag = true;
    bool red_flag = true, orange_flag = true, green_flag = true;

    time_t hold, hold_red;
    float durability = 0;
    int successive_red = 0;
    
    State fisher = State::init;
    State fisherNext = State::init;
    //HSI test_HSI;
    //Mat thresh_HSI_test;
    INPUT input;
    input.type = INPUT_MOUSE;

    POINT point;
    LPPOINT cursor;
    cursor = &point;
    while (!checkKey(VK_F2)) {
        /*test_HSI.Hue_Lower_Value = Hue_Lower_Value; test_HSI.Hue_Upper_Value = Hue_Upper_Value;
        test_HSI.Sat_Lower_Value = Saturation_Lower_Value; test_HSI.Sat_Upper_Value = Saturation_Upper_Value;
        test_HSI.Value_Lower = Value_Lower; test_HSI.Value_Upper = Value_Upper;*/

        /*GetCursorPos(cursor);
        printf("[%d, %d]\n", cursor->x, cursor->y);*/

        //GetMouseMovePointsEx(sizeof(tagMOUSEMOVEPOINT), in, out, 1, GMMP_USE_DISPLAY_POINTS);
        //printf("[%d, %d]\n", in->x, in->y);

        process.captureDesktop();
        process.cropZones();
        process.processCenter(min_radius, max_radius);

        /*if (process.circles.size() != 0) {
            process.filterHSI(process.image_hsv, process.green, 0);
            process.filterHSI(process.image_hsv, process.orange, 1);
            process.filterHSI(process.image_hsv, test_HSI, 2);
        }*/
        //printf("%d\n", process.isThereHUE(process.image_hsv, process.green));$
        
        //**F
        switch (fisher) {
        case State::init:
            if (flag_csl) { printf("STATE::Init, wainting key input (F4) to go...\n"); flag_csl = false; }

            /*if (checkKey(0x43)) {
                GetCursorPos(&point);
                printf("[%d, %d]\n", point.x, point.y);
            }*/

            if (checkKey(VK_F4)) {
                printf("  -> F4 pressed, starting...\n");
                pressKey(VK_TAB); waitKey(100); //Ugly way of fixing freelook bug
                releaseKey(VK_TAB); waitKey(2000);
                pressKey(VK_TAB); waitKey(100);
                releaseKey(VK_TAB); waitKey(2000);
                pressKey(VK_TAB); waitKey(100);
                releaseKey(VK_TAB); waitKey(2000);
                pressKey(VK_TAB); waitKey(100);
                releaseKey(VK_TAB); waitKey(2000);
                fisherNext = State::rodChecking;
                flag_csl = true;
            }
            break;

        case State::rodChecking:
            if (flag_csl) { printf("STATE::Rod checking...\n"); flag_csl = false; }

            if (process.isRodOut()) {
                printf("  -> Rod already out, will be ready to cast...\n");
                fisherNext = State::readyToCast;
                flag_csl = true;
            }
            else {
                printf("  -> Rod not out, pulling the rod...\n");
                fisherNext = State::rodIn;
                flag_csl = true;
            }

            break;

        case State::rodIn:
            if (flag_csl) { printf("STATE::Rod pulling...\n"); flag_csl = false; }

            if (process.isRodOut()) {
                printf("  -> Rod was been pulled out, will be ready to cast...\n");
                fisherNext = State::readyToCast;
                flag_csl = true;
            }
            break;

        case State::readyToCast:
            if (flag_csl) { printf("STATE::Ready to cast...\n"); flag_csl = false; }

            if (!process.isRodOut()) {
                printf("  -> Rod not out, pulling the rod...\n");
                fisherNext = State::rodIn;
                flag_csl = true;
            } else {
                //**Check durability
                durability = process.getDurability() / 543.0;
                printf("  ->  Durability of the rod is at %f\n", durability);
                if (durability == 0.0f) {
                    fisherNext = State::casting;
                    waitKey(100);
                    flag_csl = true;
                } else if (durability <= 0.1f) {
                    fisherNext = State::fixingRod;
                    flag_csl = true;
                } else {
                    fisherNext = State::casting;
                    waitKey(100);
                    flag_csl = true;
                }
            }
            break;

        case State::fixingRod:
            if (flag_csl) { printf("STATE::Fixing rod...\n"); flag_csl = false; }
            if (fisher == State::fixingRod) {
                //printf("  -> Checking the cast...\n");
                fisherNext = State::readyToCast;
                flag_csl = true;
            }
            break;

        case State::casting:
            if (flag_csl) { printf("STATE::Casting...\n"); flag_csl = false; }
            if (fisher == State::casting) {
                printf("  -> Checking the cast...\n");
                fisherNext = State::castCheck;
                flag_csl = true;
            }
            break;

        case State::castCheck:
            if (flag_csl) { printf("STATE::Checking the cast...\n"); flag_csl = false; }

            if (process.circles.size() != 0) {
                printf("  -> Casting sucessfull, will wait the fish...\n");
                fisherNext = State::waitingForFish;
                flag_csl = true;
            }
            else {
                if ((time(nullptr) - hold) >= 4) {
                    printf("  -> Timeout... Back to ready to cast\n");
                    fisherNext = State::readyToCast;
                    flag_csl = true;
                }
            }

            break;

        case State::waitingForFish:
            if (flag_csl) { printf("STATE::Waiting for fish...\n"); flag_csl = false; }

            if (process.circles.size() != 0) {
                if (process.isThereHUE(process.image_hsv, process.green)) {
                    printf("  -> FISH !!! Hooking !!\n");
                    fisherNext = State::reeling;
                    reeling_flag = true;
                    red_flag = true, green_flag = true;
                    flag_csl = true;
                }
                hold = time(nullptr);
            }
            else {
                if ((time(nullptr) - hold) >= 3) {
                    printf("  -> Timeout... Back to ready to cast...\n");
                    fisherNext = State::readyToCast;
                    flag_csl = true;
                }
            }
            break;

        case State::reeling:
            if (flag_csl) { printf("STATE::reeling..."); flag_csl = false; }
            
            if (process.circles.size() != 0) {
                if (process.isThereHUE(process.image_hsv, process.red)) {
                    if (red_flag) {
                        printf("\n  -> RED");
                        red_flag = false;
                        orange_flag = true;
                        green_flag = true;
                        hold_red = time(nullptr);
                    }
                    successive_red++;
                    printf(".");

                    if (successive_red > 10) {
                        printf("\n  -> Stuck in red...");
                        rightClickDown(input, 500, 500); waitKey(10);
                        rightClickUp(input, 500, 500); waitKey(10);
                    }

                    if (reeling_flag) {
                        reeling_flag = false;
                    }
                }else if (process.isThereHUE(process.image_hsv, process.green)) {
                    if (green_flag) {
                        printf("\n  -> Green");
                        red_flag = true;
                        orange_flag = true;
                        green_flag = false;
                    }
                    printf(".");
                    successive_red = 0;
                    if (!reeling_flag) {
                        reeling_flag = true;
                    }
                } else if (process.isThereHUE(process.image_hsv, process.orange)) {
                    if (orange_flag) {
                        printf("\n  -> Orange");

                        red_flag = true;
                        orange_flag = false;
                        green_flag = true;
                    }
                    printf(".");
                    successive_red = 0;
                }
                hold = time(nullptr);
            } else {
                if (process.isRodOut()) {
                    printf("\n  -> Reeling finished and rod out, will be ready to cast...\n");
                    fisherNext = State::readyToCast;
                    flag_csl = true;
                }

                if ((time(nullptr) - hold) >= 10) {
                    printf("\n  -> Timeout... Back to ready to cast\n");
                    fisherNext = State::readyToCast;
                    flag_csl = true;
                }
            }

            break;
        }

        //**M
        fisher = fisherNext;
        
        //**G
        switch (fisher) {
        case State::rodIn:
            pressKey(VK_F3);
            waitKey(10);
            releaseKey(VK_F3);
            waitKey(2000);
            break;

        case State::readyToCast:
            waitKey(1000);
            releaseKey(VK_F1);
            SetCursorPos(1920 / 2.0, 1080 / 2.0);
            break;

        case State::fixingRod:
            pressKey(VK_TAB); waitKey(1000);
            releaseKey(VK_TAB); waitKey(1000);
            pressKey(0x52); waitKey(1000);
            SetCursorPos(868, 665); waitKey(1000);
            leftClickDown(input, 0, 0); waitKey(1000);
            leftClickUp(input, 0, 0); waitKey(1000);
            releaseKey(0x52); waitKey(1000);
            pressKey(0x45); waitKey(1000);
            releaseKey(0x45); waitKey(1000);
            pressKey(VK_TAB); waitKey(1000);
            releaseKey(VK_TAB); waitKey(1000);
            break;

        case State::casting:
            leftClickDown(input, 1920 / 2, 1080 / 2);
            waitKey(2000);
            leftClickUp(input, 1920 / 2, 1080 / 2);
            waitKey(100);
            hold = time(nullptr);
            break;

        case State::reeling:
            pressKey(VK_F1);
            if (reeling_flag)
                leftClickDown(input, 1920 / 2, 1080 / 2);
            else
                leftClickUp(input, 1920 / 2, 1080 / 2);
            break;
        }
        waitKey(1);
    }
    leftClickUp(input, 500, 500);
    printf("\n\nFisher terminated !\n");
    return 0;
}

void pressKey(BYTE key) {
    BYTE keyState[256];
    keybd_event(key, 0x45, KEYEVENTF_EXTENDEDKEY | 0, 0);
}

void releaseKey(BYTE key) {
    BYTE keyState[256];
    keybd_event(key, 0x45, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
}

bool checkKey(BYTE key) {
    return GetKeyState(key);
}

void leftClickDown(INPUT input, int xPos, int yPos) {
    input.mi.dx = xPos;
    input.mi.dy = yPos;
    input.mi.dwFlags = (MOUSEEVENTF_LEFTDOWN);
    input.mi.mouseData = 0;
    input.mi.dwExtraInfo = NULL;
    input.mi.time = 0;
    SendInput(1, &input, sizeof(INPUT));
}

void leftClickUp(INPUT input, int xPos, int yPos) {
    input.mi.dx = xPos;
    input.mi.dy = yPos;
    input.mi.dwFlags = (MOUSEEVENTF_LEFTUP);
    input.mi.mouseData = 0;
    input.mi.dwExtraInfo = NULL;
    input.mi.time = 0;
    SendInput(1, &input, sizeof(INPUT));
}

void rightClickDown(INPUT input, int xPos, int yPos) {
    input.mi.dx = xPos;
    input.mi.dy = yPos;
    input.mi.dwFlags = (MOUSEEVENTF_RIGHTDOWN);
    input.mi.mouseData = 0;
    input.mi.dwExtraInfo = NULL;
    input.mi.time = 0;
    SendInput(1, &input, sizeof(INPUT));
}

void rightClickUp(INPUT input, int xPos, int yPos) {
    input.mi.dx = xPos;
    input.mi.dy = yPos;
    input.mi.dwFlags = (MOUSEEVENTF_RIGHTUP);
    input.mi.mouseData = 0;
    input.mi.dwExtraInfo = NULL;
    input.mi.time = 0;
    SendInput(1, &input, sizeof(INPUT));
}