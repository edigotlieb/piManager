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

#define RED_LED 14
#define BLUE_LED 15
#define ON 1
#define OFF 0

using namespace std;

std::string number2String(int num) {
    ostringstream strout;
    string str;
    strout << num;
    str = strout.str();
    return str;
}

int init_pin(int pin) {
    string pin_str = number2String(pin);
    string dir = "/sys/class/gpio/export";
    ofstream file(dir.c_str());
    if (file < 0) {
        //error
        return -1;
    }
    file << pin;
    file.close();
    dir = "/sys/class/gpio/gpio" + pin_str + "/direction";
    cout << dir << endl;
    file.open(dir.c_str());
    if (file < 0) {
        //error
        return -1;
    }
    file << "out";
    file.close();
    return 1;
}


int write_pin(int pin, int value) {
    if (value != 0 && value != 1) {
        //error
        return -1;
    }
    string value_str = number2String(value);
    string pin_str = number2String(pin);
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
    string pin_str = number2String(pin);
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


#endif	/* LEDS_H */

