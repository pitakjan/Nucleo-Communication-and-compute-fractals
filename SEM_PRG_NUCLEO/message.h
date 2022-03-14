/*
 * Defined all included library
 * all defined values (#define ...) 
*/ 

#include "mbed.h"

#define V_MAJOR 1
#define V_MINOR 5
#define V_PATCH 2

//Abort reasons
#define CANNOT_ALLOC_MEMORY (char)1
#define ABORT_FROM_PC (char)2
#define ABORT_FROM_NUCLEO (char)3

#define MAX_CHAR 255
#define CKSUM_OK 0
#define MAX_2BYTE_NUM 65025 

#define THREAD_SIGNAL 0x1

#define LEN_STARTUP_MSG 32
#define STARTUP_MSG "PRG Jan Pitak, SEMESTRALNI PRACE" 

#define MSG_TEST (char)0      // ack of the received message
#define MSG_END_TEST (char)80
#define MSG_ERROR (char)1            // report error on the previously received command
#define MSG_ABORT 'q'            // abort - from user button or from serial port
#define MSG_DONE (char)3             // report the requested work has been done
#define MSG_GET_VERSION 'g'     // request version of the firmware
#define MSG_VERSION 5          // send version of the firmware as major,minor, patch level, e.g., 1.0p1
#define MSG_STARTUP 6        // init of the message (id, up to 8 bytes long string, cksum
#define MSG_COMPUTE (char)'1'         // request computation of a batch of tasks (chunk_id, nbr_tasks)
#define MSG_COMPUTE_DATA (char)8    // computed result (chunk_id, result) (one pixel)
#define MSG_SET_COMPUTE_DATA (char)9 //send computation parameters
#define MSG_COMPUTE_FINISHED (char)10
#define MSG_COMPUTE_ABORTED (char)11
#define MSG_STOP_COMPUTE (char)'a'
#define MSG_NBR (char)12
#define MSG_OK (char)0
#define MSG_HIGHER_BOUD_RATE 'u'

#define ERROR_CKSUM_NOT_RESPOND (char)14 // send Nucleo if cksum does not respond

#define LITTLE_WAIT 1
#define TRUE 1
#define FALSE 0
#define ON 1
#define OFF 0
#define DEFAULT_INIT_VALUE (char)0
#define ERROR_RETURN (char)100
#define BUFF_SIZE 100


#define DEFAULT_WIDTH 1000
#define DEFAULT_HEIGHT 800
#define BLACK_COLOR 0

#define BOUNCE_EFFECT_BUTTON 10 //in miliseconds

#define START_PROGRAM_LED_STAT false

//Computing interval and constants
//Default parameters
#define START_REAL (-1.6)
#define START_IMAG (-1.1)
#define END_REAL (+1.6)
#define END_IMAG (+1.1)
#define C_REAL (-0.4)
#define C_IMAG (+0.6)
#define N_ITERATIONS 30

//time to take pc to start threads
#define THREAD_START_MS 500

#define HIGH_BAUD_RATE 230400
#define BAUD_RATE 115200





