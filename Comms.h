/* 
 * File:   Comms.h
 * Author: root
 *
 * Created on May 24, 2014, 9:12 PM
 */

#ifndef COMMS_H
#define	COMMS_H

// Serial includes
#include <stdio.h>   /* Standard input/output definitions */
#include <string>  /* String function definitions */
#include <unistd.h>  /* UNIX standard function definitions */
#include <fcntl.h>   /* File control definitions */
#include <errno.h>   /* Error number definitions */
#include <termios.h> /* POSIX terminal control definitions */
#include <stdlib.h>
#ifdef __linux
#include <sys/ioctl.h>
#endif

#include "mavlink/v1.0/common/mavlink.h"
#include "mavlink/v1.0/ardupilotmega/mavlink_msg_pi_trigger.h"
#include "mavlink/v1.0/ardupilotmega/mavlink_msg_pi_cam_data.h"


#define MAX_NUM_OF_TREES 10

#define CALIBRATION_MESSAGE "piCalib"
#define TRIGGER_MESSAGE "piTrigger"


#define PORT "/dev/ttyACM0"
#define BAUD 115200

using namespace std;



typedef enum read_results {
    IMAGE_SENT,
    CALIB_START,
    CALIB_STOP,
    TRIGGER_RECIEVED,
    NOTHING
} read_results;


int setupPort(int* fd, char* uart_name, int baudrate);
bool serial_readMSG(int serial_fd,mavlink_message_t* msg);
int sendTrigger(int serial_fd, uint16_t trigger_id);
int sendCamData(int serial_fd, uint16_t trigger_id, int trees[MAX_NUM_OF_TREES][2]);
string numberToString(int num);


#endif	/* COMMS_H */

