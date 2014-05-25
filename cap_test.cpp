#include <list>
#include <limits>
#include <string>
#include <fstream>
#include <cmath>
#include <stdio.h>
#include <opencv2/opencv.hpp>
#include <climits>
#include <iostream>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <time.h>
#include <ctime>
using namespace std;
using namespace cv;

VideoCapture vidCap(0);

void* imageTakingThread(void* parameters) {
    vidCap.set(CV_CAP_PROP_FRAME_WIDTH, 160);
    vidCap.set(CV_CAP_PROP_FRAME_HEIGHT, 120);
    while(1)
    {
        vidCap.grab();
    }
}

int main(){
    pthread_t thread;
    int threadSuccess = pthread_create(&thread, NULL, imageTakingThread, NULL);

    cout << "got one photo\n";
    imwrite("1st.jpg",mat);
    int a;
    cin >> a;

    Mat matToProcess;
    vidCap.retrieve(matToProcess);
    imwrite("2nd.jpg",matToProcess);
    cout<<"done"<<endl;
}
