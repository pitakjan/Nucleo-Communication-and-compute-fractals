/*
 * Structs
*/
#include "message.h"

typedef struct{ 
    char Running;
    bool Writing_Serial;
    bool Computing;
    int fd;
}Data_Struct;
Data_Struct Data;

//store values for communication with pc
typedef struct{
    char Read[BUFF_SIZE];
    char Write[BUFF_SIZE];
    char W_Idx;//write index
    char R_Idx;//Read index
}Queue_Struct;
Queue_Struct q;

typedef struct{
    int Width;//sirka
    int Height;//vyska
    double Start[2];
    double End[2];
    double C[2];
    int n;//number of iteration
}Image_Struct;
Image_Struct Image;


void Init_Data_Struct()
{
    Data.Running = true;
    Data.Writing_Serial = false; 
    Data.Computing = false;
    Data.fd = 0;
}

void Init_Image_Struct()
{
    Image.Width = DEFAULT_WIDTH;//sirka
    Image.Height = DEFAULT_HEIGHT;//vyska
    Image.Start[0] = START_REAL;
    Image.Start[1] = START_IMAG;
    Image.End[0] = END_REAL;
    Image.End[1] = END_IMAG;
    Image.C[0] = C_REAL;
    Image.C[1] = C_IMAG;
    Image.n = N_ITERATIONS;
    return;
}

void Init_Queue_Structs()
{
    q.W_Idx = 0;   
    q.R_Idx = 0;    
    return;
}

void Init_Structs()
{
    Init_Data_Struct();
    Init_Image_Struct();
    Init_Queue_Structs();
    return;    
}

//RawSerial serial(USBTX,USBRX,115200);
Serial serial(SERIAL_TX, SERIAL_RX);
InterruptIn button(USER_BUTTON);
DigitalOut led(LED1);

