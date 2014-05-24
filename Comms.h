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
#include <string.h>  /* String function definitions */
#include <unistd.h>  /* UNIX standard function definitions */
#include <fcntl.h>   /* File control definitions */
#include <errno.h>   /* Error number definitions */
#include <termios.h> /* POSIX terminal control definitions */
#ifdef __linux
#include <sys/ioctl.h>
#endif

#include "mavlink/v1.0/common/mavlink.h"
#include "mavlink/v1.0/ardupilotmega/mavlink_msg_pi_trigger.h"
#include "mavlink/v1.0/ardupilotmega/mavlink_msg_pi_cam_data.h"


#define MAX_NUM_OF_TREES 10
#define EXIT_FAILURE -1

#define CALIBRATION_MESSAGE "piCalibrat"
#define TRIGGER "piTrigger"


#define PORT "/dev/ttyACM0"
#define BAUD 115200

using namespace std;


bool silent = false; ///< Wether console output should be enabled

typedef enum read_results {
    IMAGE_SENT,
    CALIB_START,
    CALIB_STOP,
    TRIGGER_RECIEVED,
    NOTHING
} read_results;



#endif	/* COMMS_H */

