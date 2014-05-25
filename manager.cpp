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


#define RAD2DEG(x) (x)*(57.2957795)


bool processNewMsg(mavlink_message_t msg);

bool triggered = false;

bool startCalibration = false;
bool endCalibration = false;
bool inCalibration = false;

bool gotNewRoll = false;


int roll = 0;



using namespace std;

/*
 * 
 */
int main(int argc, char** argv) {
    
    // init stuff
    
    // Open Serial port    
    int port;
    int PortStat = setupPort(&port,PORT,BAUD);
    
    // handle errors 
        

   // inits
    int numberOfReadMsg = 0;
    
    // main loop
    while (true) {
        // read msg's (up to some amount)
        numberOfReadMsg = 0;
        mavlink_message_t msg;
        bool readResult = false;
        
        while((numberOfReadMsg < NUM_MSG_NOT_TRIGGERED && !triggered) || (numberOfReadMsg < NUM_MSG_TRIGGERED && triggered)) {
            msg = serial_readMSG(port);
            // no new msg
            readResult = false;
            if(msg == NULL) {
                break;
            } else {
                readResult = processNewMsg(msg);
                numberOfReadMsg++;
            }
            if(!readResult) {
                continue;
            }
            if(startCalibration) {
                // take pics for calibration and led up                             
                startCalibration = false;
                inCalibration = true;
            }
            if(endCalibration) {
                // reload conf files and led down
                endCalibration = false;
                inCalibration = false;
            }            
        }
        if(triggered && gotNewRoll) {          
           
            // take frame and process
            
            
            gotNewRoll = false;
        }
    }
    
    
    
    
    return 0;
}


bool processNewMsg(mavlink_message_t msg) {
        
    switch(msg.msgid) {
        case MAVLINK_MSG_ID_ATTITUDE: {
            mavlink_attitude_t att;
            mavlink_msg_attitude_decode(&msg,&att);
            roll = RAD2DEG(att.roll);
            gotNewRoll = true;
            return false;
        } break;
        case MAVLINK_MSG_ID_NAMED_VALUE_INT: {
            mavlink_named_value_int_t named_int;
            mavlink_msg_named_value_int_decode(&msg,&named_int);
            
            if(!strcmp(named_int.name,CALIBRATION_MESSAGE)) {
                if(named_int.value == 1) {
                    if(triggered || inCalibration) 
                        return false;                    
                    startCalibration = true;
                    return true;
                } else {
                    if(!inCalibration || triggered) 
                        return false;
                    endCalibration = true;
                    return true;
                }                
            } else if(!strcmp(named_int.name,TRIGGER_MESSAGE)) {
                if(named_int.value == 1) {
                    if(triggered || inCalibration) 
                        return false;
                    triggered = true;                    
                } else {
                    if(!triggered || inCalibration) 
                        return false;
                    triggered = false;                    
                }
                return false;
            } 
            return false;
        } break;
        default: {
            return false;
        }
    }
}