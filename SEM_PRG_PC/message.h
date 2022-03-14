#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>
#include <pthread.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <poll.h>
#include <sys/stat.h>
#include <math.h>

#define LOCATION_OF_NUCLEO "/dev/ttyACM0"

#define MAX_IMAGE_WIDTH 1600
#define MIN_IMAGE_WIDTH 32
#define MAX_IMAGE_HEIGHT 1000
#define MIN_IMAGE_HEIGHT 32

#define CANNOT_ALLOC_MEMORY 1
#define ABORT_FROM_PC 2
#define ABORT_FROM_NUCLEO 3

#define MSG_TEST 0             // ack of the received message
#define MSG_END_TEST 80        // ack of the received message
#define MSG_ERROR 1            // report error on the previously received command
#define MSG_ABORT 'q'            // abort - from user button or from serial port
#define MSG_DONE 3             // report the requested work has been done
#define MSG_GET_VERSION 'g'     // request version of the firmware
#define MSG_VERSION 5          // send version of the firmware as major,minor, patch level, e.g., 1.0p1
 
#define MSG_STARTUP 6        // init of the message (id, up to 8 bytes long string, cksum
#define MSG_COMPUTE '1'        // request computation of a batch of tasks (chunk_id, nbr_tasks)
#define MSG_COMPUTE_DATA 8    // computed result (chunk_id, result) (one pixel)
#define MSG_SET_COMPUTE_DATA 9 //send computation parameters
#define MSG_COMPUTE_FINISHED 10
#define MSG_COMPUTE_ABORTED (char)11
#define MSG_STOP_COMPUTE 'a'
#define MSG_NBR 12
#define MSG_OK 0

#define ERROR_CKSUM_NOT_RESPOND 14 //send from nucleo

#define ERROR_CKSUM_DOES_NOT_RESPOND "ERROR: Cksum does not respond\n" //print to console

#define TRUE 1
#define FALSE 0
#define ON 1
#define OFF 0
#define DEFAULT_INIT_VALUE 0
#define ERROR_RETURN 100

#define STEP_CHANGE_VALUE 0.1

#define MAX_CHAR 0
#define LEN_STARTUP_MSG 32

#define DEFAULT_WIDTH 1000
#define	DEFAULT_HEIGHT 800

#define BUFF_SIZE 20
#define BLACK_COLOR 0
#define WAIT_FOREVER 2147483647
#define REGULAR_WAIT_FOR_RESPONSE 100

//Computing interval and constants

//This paragraph is to calculate
#define START_REAL (-1.6)
#define START_IMAG (-1.1)
#define END_REAL (+1.6)
#define END_IMAG (+1.1)
#define C_REAL (-0.4)
#define C_IMAG (+0.6)
#define N_ITERATIONS 30

#define DEFAULT_WIDTH_MOD_5 200
#define DEFAULT_HEIGHT_MOD_5 100
#define DEFAULT_WIDTH_MOD_6 700
#define DEFAULT_HEIGHT_MOD_6 500
#define DEFAULT_WIDTH_MOD_7 1000
#define DEFAULT_HEIGHT_MOD_7 800

#define COLOR_SET_PARAMETERS "\033[1;31m"

#define BAUD_RATE B115200

void Print_menu()
{
	printf("\n\033[1;34m ____Menu____\033[0m\n"
	"g - Get Firmware\n"
	"s - Set Parameters\n"
	"i - Set image size\n"
	"1 - Start Compute on Nucleo \n"
	"a - Abort Computing\n"
	"r - Reset cid\n"
	"l - Delete actual result\n"
	"p - Redraw actual stat\n"
	"c - Start Compute on PC\n"
	"3-5 - Animations mode\n"
	"b - Stop animation\n"
	"q - End Program\n"
	"d - Show actual received chunk \n"
	"x - Close Image\n"
	"m - Show Menu\n"
	);
	return;
}





