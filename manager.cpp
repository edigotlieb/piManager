/* 
 * File:   manager.cpp
 * Author: root
 *
 * Created on May 24, 2014, 9:11 PM
 */

#include <cstdlib>


#include "Comms.h"
#include "ImageProcessing.h"
#include "leds.h"


#define NUM_MSG_NOT_TRIGGERED 10
#define NUM_MSG_TRIGGERED 10
#define NUM_PICTURES_FOR_CALIB 4
#define CALIB_WIDTH 640
#define CALIB_HEIGHT 480
#define DETECT_WIDTH 160
#define DETECT_HEIGHT 120

#define RAD2DEG(x) (x)*(57.2957795)

#define TIMES_TO_FLUSH 5


bool processNewMsg(mavlink_message_t msg);
bool calibrate();
void inError();

bool startTrigger = false;
bool triggered = false;
bool endTrigger = false;

bool startCalibration = false;
bool endCalibration = false;
bool inCalibration = false;


bool gotNewRoll = false;

int roll = 0;
FilterApplier fa;

// Image Capture Globals
VideoCapture vidCapCalib, vidCapNav;
pthread_t calibThread, navThread;
bool calibThreadOpen, navThreadOpen;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
Mat image;

// Log Files
clock_t startOfRun;
bool logFligths = true;
ofstream treeLogFile;

using namespace std;

void* imageTakingThreadCalib(void* parameters)
{
    while(calibThreadOpen)
    {
	pthread_mutex_lock(&mutex);
	if(vidCapCalib.isOpened())
	{
	    vidCapCalib.read(image);
	}
	pthread_mutex_unlock(&mutex);
	sleep(1);
    }
    pthread_exit(0);
}

void* imageTakingThreadNav(void* parameters)
{
    while(navThreadOpen)
    {
	pthread_mutex_lock(&mutex);
	if(vidCapNav.isOpened())
	{
	    vidCapNav.read(image);
	}
	pthread_mutex_unlock(&mutex);
	sleep(1);
    }
    pthread_exit(0);
}

/*
 * 
 */
int main(int argc, char** argv)
{
    // init stuff
    init_pin(BLUE_LED);
    init_pin(RED_LED);
    int trigger_id = 0;

    // Open Serial port    
    int port;
    int PortStat = setupPort(&port, PORT, BAUD);

    // Open Log Files
    treeLogFile.open("./logs/trees.log", ios_base::out | ios_base::app);
    if(logFligths && !treeLogFile.is_open())
    {
	inError();
    }
    string newFlight = "\n----------\nNew Flight\n----------\n";
    treeLogFile.write(newFlight.c_str(), newFlight.length());
    treeLogFile.flush();

    // Handle Errors 

    // Inits
    mavlink_message_t msg;
    int numberOfReadMsg = 0;
    bool readResult, rv;
    blink(RED_LED);
    blink(BLUE_LED);
    blink(RED_LED);
    blink(BLUE_LED);

    // Main Loop
    while(true)
    {
	// Read messages (up to some amount)
	numberOfReadMsg = 0;
	readResult = false;
	while((numberOfReadMsg < NUM_MSG_NOT_TRIGGERED && !triggered) || (numberOfReadMsg <
		NUM_MSG_TRIGGERED && triggered))
	{
	    rv = serial_readMSG(port, &msg);
	    // no new msg
	    readResult = false;
	    if(!rv)
	    {
		break;
	    }
	    else
	    {
		readResult = processNewMsg(msg);
		numberOfReadMsg++;
	    }
	    if(!readResult)
	    {
		continue;
	    }
	    if(startCalibration)
	    {
		// *** START CALIB ***
		// take pics for calibration and led up
		write_pin(BLUE_LED, ON);

		if(vidCapNav.isOpened())
		{
		    blink(RED_LED);
		    blink(RED_LED);
		    inError();
		}
		vidCapCalib.open(0);
		vidCapCalib.set(CV_CAP_PROP_FRAME_WIDTH, CALIB_WIDTH);
		vidCapCalib.set(CV_CAP_PROP_FRAME_HEIGHT, CALIB_HEIGHT);

		calibThreadOpen = true;
		int threadFailure = pthread_create(&calibThread, NULL, imageTakingThreadCalib, NULL);
		if(threadFailure)
		{
		    blink(RED_LED);
		    blink(RED_LED);
		    inError();
		}
		sleep(1);

		if(calibrate())
		{
		    write_pin(RED_LED, ON);
		}
		else
		{
		    inError();
		}
		startCalibration = false;
		inCalibration = true;
	    }
	    if(endCalibration)
	    {
		// *** END CALIB ***
		// reload conf files and led down

		calibThreadOpen = false;
		void **value;
		pthread_join(calibThread, value);
		vidCapCalib.release();

		fa.reload();
		write_pin(BLUE_LED, OFF);
		write_pin(RED_LED, OFF);
		endCalibration = false;
		inCalibration = false;
	    }
	    if(startTrigger)
	    {
		// do stuff
		startOfRun = clock();

		if(vidCapCalib.isOpened())
		{
		    blink(RED_LED);
		    blink(RED_LED);
		    inError();
		}
		vidCapNav.open(0);
		vidCapNav.set(CV_CAP_PROP_FRAME_WIDTH, DETECT_WIDTH);
		vidCapNav.set(CV_CAP_PROP_FRAME_HEIGHT, DETECT_HEIGHT);

		navThreadOpen = true;
		int threadFailure = pthread_create(&navThread, NULL, imageTakingThreadNav, NULL);
		if(threadFailure)
		{
		    blink(RED_LED);
		    blink(RED_LED);
		    inError();
		}
		sleep(1);

		startTrigger = false;
		triggered = true;
	    }
	    if(endTrigger)
	    {
		// do stuff

		navThreadOpen = false;
		void **value;
		pthread_join(navThread, value);
		vidCapNav.release();

		endTrigger = false;
		triggered = false;
	    }
	}
	if(triggered && gotNewRoll)
	{
	    sendTrigger(port, trigger_id);
	    cout << "got new roll, wants to process. roll = " << roll << endl;
	    clock_t start = clock();

	    // Get the latest frame
	    Mat matToProcess;
	    pthread_mutex_lock(&mutex);
	    image.copyTo(matToProcess);
	    pthread_mutex_unlock(&mutex);

	    // Apply image processing
	    int trees[MAX_NUM_OF_TREES][2];
	    fa.applyAllFilters(matToProcess, trees);

	    // Measure elapsed time
	    double duration = (clock() - start) / (double) CLOCKS_PER_SEC;
	    cout << "detected trees in " << duration << endl;

	    // Save data to log if necessary
	    if(logFligths)
	    {
		addTreesToLogFile(trees);
	    }

	    // Send data from image processing
	    sendCamData(port, trigger_id, trees);
	    blink(BLUE_LED);

	    // Update flags
	    trigger_id++;
	    gotNewRoll = false;
	}
    }
    return 0;
}

bool processNewMsg(mavlink_message_t msg)
{
    switch(msg.msgid)
    {
	case MAVLINK_MSG_ID_ATTITUDE:
	{
	    mavlink_attitude_t att;
	    mavlink_msg_attitude_decode(&msg, &att);
	    roll = RAD2DEG(att.roll);
	    gotNewRoll = true;
	    return false;
	}
	    break;

	case MAVLINK_MSG_ID_NAMED_VALUE_FLOAT:
	{
	    mavlink_named_value_float_t named_float;
	    mavlink_msg_named_value_float_decode(&msg, &named_float);
	    cout << "got feedback: " << named_float.name << endl;
	}
	    break;

	case MAVLINK_MSG_ID_NAMED_VALUE_INT:
	{
	    mavlink_named_value_int_t named_int;
	    mavlink_msg_named_value_int_decode(&msg, &named_int);

	    if(!strcmp(named_int.name, CALIBRATION_MESSAGE))
	    {
		cout << "got calibrate, value = " << named_int.value << endl;
		if(named_int.value == 1)
		{
		    if(triggered || inCalibration)
			return false;
		    startCalibration = true;
		    return true;
		}
		else
		{
		    if(!inCalibration || triggered)
			return false;
		    endCalibration = true;
		    return true;
		}
	    }
	    else if(!strcmp(named_int.name, TRIGGER_MESSAGE))
	    {
		cout << "got trigger, value = " << named_int.value << endl;
		if(named_int.value == 1)
		{
		    if(triggered || inCalibration)
			return false;
		    startTrigger = true;
		}
		else
		{
		    if(!triggered || inCalibration)
			return false;
		    endTrigger = true;
		}
		return true;
	    }
	    return false;
	}
	    break;

	default:
	{
	    return false;
	}
    }
}

bool calibrate()
{
    // Make sure that the right VideoCapture is open
    if(!vidCapCalib.isOpened())
    {
	blink(RED_LED);
	blink(RED_LED);
	blink(RED_LED);
	return false;
    }

    // Take pictures every 5 seconds
    for(int i = 0; i < NUM_PICTURES_FOR_CALIB; i++)
    {
	string filename = "calib_" + numberToString(i) + ".jpg";
	pthread_mutex_lock(&mutex);
	imwrite(filename, image);
	pthread_mutex_unlock(&mutex);
	blink(RED_LED);
	sleep(5);
    }
    return true;
}

void addTreesToLogFile(int trees[MAX_NUM_OF_TREES][2])
{
    static int flush = 0;

    // Add headers
    clock_t time = clock() - startOfRun;
    string timestamp = "Timestamp: " + time + "\n";
    treeLogFile.write(timestamp.c_str(), timestamp.length());
    string treesHeader = "Trees were found in the following locations: \n";
    treeLogFile.write(treesHeader.c_str(), treesHeader.length());

    // Add tree info
    string trees;
    for(int i = 0; i < MAX_NUM_OF_TREES; i++)
    {
	trees = "Left Border = " + trees[i][0] + ", Right Border = " + trees[i][1] + "\n";
	treeLogFile.write(trees.c_str(), trees.length());
    }
    treeLogFile.put('\n');
    treeLogFile.put('\n');

    // Check for flush
    flush++;
    if(flush == TIMES_TO_FLUSH)
    {
	treeLogFile.flush();
	flush = 0;
    }
}

void inError()
{
    while(true)
    {
	blink(RED_LED);
    }
}