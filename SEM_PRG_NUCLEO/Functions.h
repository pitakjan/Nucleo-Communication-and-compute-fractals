/*
 * Small functions to be remoted from main
*/


#include "Struct.h"
#include "Queue.h"
queue_t *Q_Write;
queue_t *Q_Read;

void Disable()
{
    __disable_irq();
}

void Enable()
{
    __enable_irq();
}

void Check_buffer_idx()
{
    if (q.W_Idx > BUFF_SIZE-10) {
        q.W_Idx = 0;
    }
    if (q.R_Idx > BUFF_SIZE-10) {
        q.R_Idx = 0;
    }
    return;
}

int round(double Num)
{

    if(((int)(Num*100))%100 >= 50) {
        return ((int) Num) + 1;
    } else {
        return (int)Num;
    }
}

void Send()
{
    //enable Tx_interupt
    USART2->CR1 |= USART_CR1_TXEIE;
    return;
}

char Get_cksum(int cksum)
{
    cksum = cksum % 128;
    return (char)-cksum;

}

float Abs_value(float R, float I)
{
    return sqrt(R*R+I*I);
}



void Test_Serial()
{
    //send 10 bytes and stop byte to Control
    Disable();
    char tmp =  MSG_TEST;
    for (char i = 0; i < 10; ++i) {
        push_to_queue(Q_Write,(void*)&tmp);
    }
    char tml = MSG_END_TEST;
    push_to_queue(Q_Write,(void*)&tml);
    Enable();
    Send();
    wait_ms(THREAD_START_MS);
    return;
}

void Cksum_not_respond()
{
    Disable();
    q.Write[q.W_Idx] = ERROR_CKSUM_NOT_RESPOND;
    push_to_queue(Q_Write,(void*)&q.Write[q.W_Idx++]);
    q.Write[q.W_Idx] = Get_cksum(q.Write[q.W_Idx-1]);
    push_to_queue(Q_Write,(void*)&q.Write[q.W_Idx++]);
    Enable();
    Send();
    return;
}

bool Control_2byte_msg(char Type)
{
    bool ret = true;
    char cksum = pop_from_queue(Q_Read);
    //why? there must be retype (char) ???
    if ((char)(Type + cksum) != CKSUM_OK) {
        Cksum_not_respond();
        ret = false;
    }
    return ret;
}

void Send_Version()
{
    Check_buffer_idx();
    Disable();
    q.Write[q.W_Idx] = MSG_VERSION;
    push_to_queue(Q_Write,(void*)&q.Write[q.W_Idx++]);
    q.Write[q.W_Idx] = V_MAJOR;
    push_to_queue(Q_Write,(void*)&q.Write[q.W_Idx++]);
    q.Write[q.W_Idx] = V_MINOR;
    push_to_queue(Q_Write,(void*)&q.Write[q.W_Idx++]);
    q.Write[q.W_Idx] = V_PATCH;
    push_to_queue(Q_Write,(void*)&q.Write[q.W_Idx++]);
    int cksum = MSG_VERSION + V_MAJOR + V_MINOR + V_PATCH;
    q.Write[q.W_Idx] = Get_cksum(cksum);
    push_to_queue(Q_Write,(void*)&q.Write[q.W_Idx++]);

    Enable();
    Send();
    return;
}

void Send_Startup_msg()
{
    Check_buffer_idx();
    q.Write[q.W_Idx] = MSG_STARTUP;
    push_to_queue(Q_Write,(void*)&q.Write[q.W_Idx++]);
    char Buff[] = STARTUP_MSG;
    int cksumm = (int) MSG_STARTUP;
    for (int i = 0; i < LEN_STARTUP_MSG; i++) {
        q.Write[q.W_Idx] = Buff[i];
        push_to_queue(Q_Write,(void*)&q.Write[q.W_Idx++]);
        cksumm += Buff[i];
    }
    q.Write[q.W_Idx] = Get_cksum(cksumm);
    push_to_queue(Q_Write,(void*)&q.Write[q.W_Idx++]);
    Send();
    return;
}

void Abort(char Reason)
{
    Check_buffer_idx();
    Disable();
    Data.Running = false;
    Data.Computing = false;
    q.Write[q.W_Idx] = MSG_ABORT;
    push_to_queue(Q_Write,(void*)&q.Write[q.W_Idx++]);
    q.Write[q.W_Idx] = Reason;
    push_to_queue(Q_Write,(void*)&q.Write[q.W_Idx++]);
    q.Write[q.W_Idx] = MSG_ABORT + Reason;
    q.Write[q.W_Idx] = Get_cksum(q.Write[q.W_Idx]);
    push_to_queue(Q_Write,(void*)&q.Write[q.W_Idx++]);
    Enable();
    Send();
    return;
}

//Int - true if Receive int, false if receive double
int Receive_4byte_Number(int *Int, double *Double)
{
    int Num = 0;
    int m1,m2,m3,cksum = 0;
    m1 = pop_from_queue(Q_Read) + 128;
    q.Write[q.W_Idx] = m1-128;
    push_to_queue(Q_Write,(void*)&q.Write[q.W_Idx++]);
    m2 = pop_from_queue(Q_Read) + 128;
    q.Write[q.W_Idx] = m2-128;
    push_to_queue(Q_Write,(void*)&q.Write[q.W_Idx++]);
    m3 = pop_from_queue(Q_Read) + 128;
    q.Write[q.W_Idx] = m3-128;
    push_to_queue(Q_Write,(void*)&q.Write[q.W_Idx++]);
    cksum += m1 + m2 + m3;
    Num = m1 * MAX_2BYTE_NUM + m2 * MAX_CHAR + m3;
    if(Int != NULL) {
        *Int = Num;
    } else {
        *Double = (double)Num/100000;
    }
    Enable();
    Send();
    Disable();
    return cksum;
}

void Write_N_Byte_to_memory(int n, void* Pos)
{
    char* C = (char*) Pos;
    for (int i = 0; i < n; i++) {
        *(C+i) = pop_from_queue(Q_Read);
    }
    return;
}

void Set_Compute_Data()
{
    Disable();
    Write_N_Byte_to_memory(4,&Image.Width);
    Write_N_Byte_to_memory(4,&Image.Height);
    int cksum = MSG_SET_COMPUTE_DATA + Image.Width + Image.Height;
    for (int i = 0; i < 2; i++) {
        Write_N_Byte_to_memory(8,&Image.Start[i]);
        Write_N_Byte_to_memory(8,&Image.End[i]);
        Write_N_Byte_to_memory(8,&Image.C[i]);
        cksum += (int) Image.Start[i]*10;
        cksum += (int) Image.End[i]*10;
        cksum += (int) Image.C[i]*10;
    }
    Write_N_Byte_to_memory(4,&Image.n);
    cksum += pop_from_queue(Q_Read) + Image.n;
    Enable();
    cksum = cksum % 128;
    if (cksum != CKSUM_OK) {
        Cksum_not_respond();
    }
    return;
}

int Msg_size(char Msg)
{
    switch (Msg) {
        case MSG_ABORT:
        case MSG_COMPUTE:
        case MSG_STOP_COMPUTE:
        case MSG_HIGHER_BOUD_RATE:
            return 2;
        case MSG_GET_VERSION:
            return 2;
        case MSG_SET_COMPUTE_DATA:
            return (1 + 4*3 + 8*6 + 1);
        default:
            pop_from_queue(Q_Read);
            return 100;
    }
}

void Nothing()
{}