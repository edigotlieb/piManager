
#include "Comms.h"
#include <sstream>
bool silent = false; ///< Wether console output should be enabled
bool setup_port(int fd, int baud, int data_bits, int stop_bits, bool parity, bool hardware_control) {
    //struct termios options;

    struct termios config;
    if (!isatty(fd)) {
        fprintf(stderr, "\nERROR: file descriptor %d is NOT a serial port\n", fd);
        return false;
    }
    if (tcgetattr(fd, &config) < 0) {
        fprintf(stderr, "\nERROR: could not read configuration of fd %d\n", fd);
        return false;
    }
    //
    // Input flags - Turn off input processing
    // convert break to null byte, no CR to NL translation,
    // no NL to CR translation, don't mark parity errors or breaks
    // no input parity check, don't strip high bit off,
    // no XON/XOFF software flow control
    //
    config.c_iflag &= ~(IGNBRK | BRKINT | ICRNL |
            INLCR | PARMRK | INPCK | ISTRIP | IXON);
    //
    // Output flags - Turn off output processing
    // no CR to NL translation, no NL to CR-NL translation,
    // no NL to CR translation, no column 0 CR suppression,
    // no Ctrl-D suppression, no fill characters, no case mapping,
    // no local output processing
    //
    config.c_oflag &= ~(OCRNL | ONLCR | ONLRET |
            ONOCR | OFILL | OPOST);

#ifdef OLCUC
    config.c_oflag &= ~OLCUC;
#endif

#ifdef ONOEOT
    config.c_oflag &= ~ONOEOT;
#endif

    //
    // No line processing:
    // echo off, echo newline off, canonical mode off,
    // extended input processing off, signal chars off
    //
    config.c_lflag &= ~(ECHO | ECHONL | ICANON | IEXTEN | ISIG);
    //
    // Turn off character processing
    // clear current char size mask, no parity checking,
    // no output processing, force 8 bit input
    //
    config.c_cflag &= ~(CSIZE | PARENB);
    config.c_cflag |= CS8;
    //
    // One input byte is enough to return from read()
    // Inter-character timer off
    //
    config.c_cc[VMIN] = 1;
    config.c_cc[VTIME] = 10; // was 0

    // Get the current options for the port
    //tcgetattr(fd, &options);

    switch (baud) {
        case 1200:
            if (cfsetispeed(&config, B1200) < 0 || cfsetospeed(&config, B1200) < 0) {
                fprintf(stderr, "\nERROR: Could not set desired baud rate of %d Baud\n", baud);
                return false;
            }
            break;
        case 1800:
            cfsetispeed(&config, B1800);
            cfsetospeed(&config, B1800);
            break;
        case 9600:
            cfsetispeed(&config, B9600);
            cfsetospeed(&config, B9600);
            break;
        case 19200:
            cfsetispeed(&config, B19200);
            cfsetospeed(&config, B19200);
            break;
        case 38400:
            if (cfsetispeed(&config, B38400) < 0 || cfsetospeed(&config, B38400) < 0) {
                fprintf(stderr, "\nERROR: Could not set desired baud rate of %d Baud\n", baud);
                return false;
            }
            break;
        case 57600:
            if (cfsetispeed(&config, B57600) < 0 || cfsetospeed(&config, B57600) < 0) {
                fprintf(stderr, "\nERROR: Could not set desired baud rate of %d Baud\n", baud);
                return false;
            }
            break;
        case 115200:
            if (cfsetispeed(&config, B115200) < 0 || cfsetospeed(&config, B115200) < 0) {
                fprintf(stderr, "\nERROR: Could not set desired baud rate of %d Baud\n", baud);
                return false;
            }
            break;

            // These two non-standard (by the 70'ties ) rates are fully supported on
            // current Debian and Mac OS versions (tested since 2010).
        case 460800:
            if (cfsetispeed(&config, 460800) < 0 || cfsetospeed(&config, 460800) < 0) {
                fprintf(stderr, "\nERROR: Could not set desired baud rate of %d Baud\n", baud);
                return false;
            }
            break;
        case 921600:
            if (cfsetispeed(&config, 921600) < 0 || cfsetospeed(&config, 921600) < 0) {
                fprintf(stderr, "\nERROR: Could not set desired baud rate of %d Baud\n", baud);
                return false;
            }
            break;
        default:
            fprintf(stderr, "ERROR: Desired baud rate %d could not be set, aborting.\n", baud);
            return false;

            break;
    }

    //
    // Finally, apply the configuration
    //
    if (tcsetattr(fd, TCSAFLUSH, &config) < 0) {
        fprintf(stderr, "\nERROR: could not set configuration of fd %d\n", fd);
        return false;
    }
    return true;
}
int open_port(const char* port) {
    int fd; /* File descriptor for the port */

    // Open serial port
    // O_RDWR - Read and write
    // O_NOCTTY - Ignore special chars like CTRL-C
    fd = open(port, O_RDWR | O_NOCTTY | O_NDELAY);
    if (fd == -1) {
        /* Could not open the port. */
        return (-1);
    } else {
        fcntl(fd, F_SETFL, 0);
    }

    return (fd);
}
int setupPort(int* fd, char* uart_name, int baudrate) {
    // SETUP SERIAL PORT

    // Exit if opening port failed
    // Open the serial port.
    if (!silent) printf("Trying to connect to %s.. ", uart_name);
    fflush(stdout);
    *fd = open_port(uart_name);
    if (*fd == -1) {
        if (!silent) printf("failure, could not open port.\n");
        return (EXIT_FAILURE);
    } else {
        if (!silent) printf("success.\n");
    }
    if (!silent) printf("Trying to configure %s.. ", uart_name);
    bool setup = setup_port(*fd, baudrate, 8, 1, false, false);
    if (!setup) {
        if (!silent) printf("failure, could not configure port.\n");
        return (EXIT_FAILURE);
    } else {
        if (!silent) printf("success.\n");
    }

    int noErrors = 0;
    if (*fd == -1 || *fd == 0) {
        if (!silent) fprintf(stderr, "Connection attempt to port %s with %d baud, 8N1 failed, exiting.\n", uart_name, baudrate);
        return (EXIT_FAILURE);
    } else {
        if (!silent) fprintf(stderr, "\nConnected to %s with %d baud, 8 data bits, no parity, 1 stop bit (8N1)\n", uart_name, baudrate);
    }

    if (*fd < 0) {
        return (noErrors);
    }
    return 2;
}

/**
 *
 *
 * Returns the file descriptor on success or -1 on error.
 */





void close_port(int fd) {
    close(fd);
}


bool serial_readMSG(int serial_fd,mavlink_message_t* msg) {
    int fd = serial_fd;
    // Blocking wait for new data
    uint8_t msgReceived = false;

    uint8_t cp;
    mavlink_status_t status;

    while (read(fd, &cp, 1) > 0) {      
            // Check if a message could be decoded, return the message in case yes
        msgReceived = mavlink_parse_char(MAVLINK_COMM_1, cp, msg, &status);
        if(msgReceived) {
            return true;
        }
    }
    return false;
}

int sendCamData(int serial_fd, uint16_t trigger_id, int trees[MAX_NUM_OF_TREES][2]) {
    int fd = serial_fd;
    char buf[300];
    mavlink_message_t message;
    mavlink_pi_cam_data_t data;

    data.start_tree_1 = trees[0][0];
    data.end_tree_1 = trees[0][1];
    data.start_tree_2 = trees[1][0];
    data.end_tree_2 = trees[1][1];
    data.start_tree_3 = trees[2][0];
    data.end_tree_3 = trees[2][1];
    data.start_tree_4 = trees[3][0];
    data.end_tree_4 = trees[3][1];
    data.start_tree_5 = trees[4][0];
    data.end_tree_5 = trees[4][1];
    data.start_tree_6 = trees[5][0];
    data.end_tree_6 = trees[5][1];
    data.start_tree_7 = trees[6][0];
    data.end_tree_7 = trees[6][1];
    data.start_tree_8 = trees[7][0];
    data.end_tree_8 = trees[7][1];
    data.start_tree_9 = trees[8][0];
    data.end_tree_9 = trees[8][1];
    data.start_tree_10 = trees[9][0];
    data.end_tree_10 = trees[9][1];
    data.data_id = trigger_id;
    mavlink_msg_pi_cam_data_encode(255, 1, &message, &data);

    unsigned len = mavlink_msg_to_send_buffer((uint8_t*) buf, &message);
    //   printf("before write\n");
    /* write packet via serial link */
    write(fd, buf, len);

    /* wait until all data has been written
    tcdrain(fd);
     */
    //     printf("after write , sys_id = %d\n",requestParam.target_system);


    return 0;

}

int sendTrigger(int serial_fd, uint16_t trigger_id) {
    int fd = serial_fd;
    char buf[300];
    mavlink_message_t message;
    mavlink_pi_trigger_t trigger;
    trigger.trigger_id = trigger_id;
    trigger.timestamp_pi = 0; // TODO
    mavlink_msg_pi_trigger_encode(255, 1, &message, &trigger);

    unsigned len = mavlink_msg_to_send_buffer((uint8_t*) buf, &message);
    //   printf("before write\n");
    /* write packet via serial link */
    write(fd, buf, len);

    /* wait until all data has been written
    tcdrain(fd);
     */
    //     printf("after write , sys_id = %d\n",requestParam.target_system);


    return 0;

}


std::string numberToString(int num) {
    ostringstream strout;
    string str;
    strout << num;
    str = strout.str();
    return str;
}