/* 
 * File:   leds.h
 * Author: root
 *
 * Created on May 24, 2014, 9:14 PM
 */

#ifndef LEDS_H
#define	LEDS_H

#include <iostream>
#include <ctime>
#include <cstdlib>
#include <unistd.h>
#include <cmath>
#include <string.h>
#include <inttypes.h>
#include <fstream>

using namespace std;

std::string numberToString(int num) {
    ostringstream strout;
    string str;
    strout << num;
    str = strout.str();
    return str;
}


int write_pin(int pin, int value) {
    if (value != 0 && value != 1) {
        //error
        return -1;
    }
    string value_str = numberToString(value);
    string pin_str = numberToString(pin);
    string dir = "/sys/class/gpio/gpio" + pin_str + "/value";
    ofstream file(dir.c_str());
    if (file < 0) {
        return -1;
    }
    file << value_str;
    file.close();
    return 1;
}

int read_pin(int pin){
    string pin_str = numberToString(pin);
    string dir = "/sys/class/gpio/gpio" + pin_str + "/value";
    ifstream file(dir.c_str());
    if (file < 0) {
        return -1;
    }
    string value;
    file >> value;
    return atoi(value.c_str());
}

void blink(int pin) {
    
    write_pin(pin, 1);
    usleep(250000);
    write_pin(pin, 0);
    usleep(250000);
}

void blink(int pin,int sleep_time) {
    int value = read_pin(pin);
    cout <<"old value "<< value<<endl;
    if (value!=0 && value!=1){
    value=0;
    }
    value = (value+1)%2;
    cout << "new value " <<value<<endl;
    write_pin(value,1);
    usleep(sleep_time);
    write_pin((value+1)%2,0);
    usleep(sleep_time);
}

#endif	/* LEDS_H */

