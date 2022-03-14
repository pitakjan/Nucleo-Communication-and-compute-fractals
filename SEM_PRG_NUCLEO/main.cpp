/*
 * Semestralni prace, CVUT FEL, Pitak Jan
 * 30.kvetna 2020
*/

/*
 * Main Functions and Interrupts
 * FUNC: Main, Read_Serial, Computing, 
 * INTERRUPTS: Rx, Tx and Button Interrupt
*/
#include "Functions.h"

void Read_Serial();
void Computing();

//Run this func when received Byte on serial port
void Rx_interrupt()
{
    Check_buffer_idx();
    while (serial.readable()) {
        q.Read[q.R_Idx] = serial.getc();
        push_to_queue(Q_Read,(void*)&q.Read[q.R_Idx++]);
    }
    return;
}

//send Byte
void Tx_interrupt()
{
    if (get_queue_size(Q_Write)) {
        char tmp = pop_from_queue(Q_Write);
        serial.putc(tmp);
    } else { // buffer sent out, disable Tx interrupt
        USART2->CR1 &= ~USART_CR1_TXEIE;
    }
    return;
}

//Run when button was pressed
void Button_interrupt()
{
    button.fall(&Nothing);
    Abort(ABORT_FROM_NUCLEO);
    
}

int main()
{
    Q_Write = create_queue(BUFF_SIZE);
    Q_Read = create_queue(BUFF_SIZE);
    button.fall(&Button_interrupt);
    serial.attach(&Rx_interrupt, Serial::RxIrq); // attach interrupt handler to receive data
    serial.attach(&Tx_interrupt, Serial::TxIrq); // attach interrupt handler to transmit data
    serial.baud(BAUD_RATE);//speed of serial
    Init_Structs();
    Test_Serial();

    for (int i = 0; i < 5; ++i) {
        led = !led;
        wait_ms(50);
    }
    
    led = START_PROGRAM_LED_STAT;
    Send_Startup_msg();
    Send_Version();
    
    //Start Program
    while(Data.Running) {
        Read_Serial();
        Computing();
    }

    while(get_queue_size(Q_Write) != 0) {
        //wait until serial send all bytes
        wait_ms(10);
    }


    //End program
    __disable_irq();
    delete_queue(Q_Read);
    delete_queue(Q_Write);

    for (int i = 0; i < 50; ++i) {
        led = !led;
        wait_ms(30);
    }

    return 0;
}

//________________THREADS_____________________//


void Computing()
{
    if (Data.Computing) {
        //int Index = 0;
        Disable();
        bool Print_Serial = false;
        float Rozdil_R = Image.End[0]-Image.Start[0];
        float Rozdil_I = Image.End[1]-Image.Start[1];
        float Point_R = (float)Rozdil_R / Image.Width;
        float Point_I = (float)Rozdil_I / Image.Height;
        float I = Image.Start[1];
        for (int h = 0; h < Image.Height; h++) {
            led = !led;//blink when next row was computed
            float R = Image.End[0];
            for (int w = 0; w < Image.Width; w++) {
                float Z[2] = {R,I};
                for(int n = 0; n < Image.n; n++) {
                    float tmp = Z[0];
                    Z[0] = (tmp*tmp) - (Z[1]*Z[1]) + Image.C[0];
                    Z[1] = 2*tmp*Z[1] + Image.C[1];
                    char Buff[3];
                    Print_Serial = false;
                    if (Abs_value(Z[0], Z[1]) >= 2) {
                        double t = (double)n/Image.n;
                        Buff[0] = (char)(round(9*(1-t)*(t*t*t)*255) - 128);
                        Buff[1] = (char)(round(15*((1-t)*(1-t))*(t*t)*255) - 128);
                        Buff[2] = (char)(round(8.5*((1-t)*(1-t)*(1-t))*t*255) - 128);
                        Print_Serial = true;
                        n = Image.n;
                    } else if (n == Image.n - 1) {
                        Buff[0] = (char)(BLACK_COLOR - 128);
                        Buff[1] = (char)(BLACK_COLOR - 128);
                        Buff[2] = (char)(BLACK_COLOR - 128);
                        Print_Serial = true;
                    }
                    if (Print_Serial) {
                        Check_buffer_idx();
                        int cksum = MSG_COMPUTE_DATA;
                        q.Write[q.W_Idx] = MSG_COMPUTE_DATA;
                        push_to_queue(Q_Write,(void*)&q.Write[q.W_Idx++]);

                        for (int i = 0; i < 3; i++) {
                            q.Write[q.W_Idx] = Buff[i];
                            push_to_queue(Q_Write,(void*)&q.Write[q.W_Idx++]);
                            cksum += (Buff[i] + 128);
                        }

                        q.Write[q.W_Idx] = Get_cksum(cksum);
                        push_to_queue(Q_Write,(void*)&q.Write[q.W_Idx++]);
                        Enable();
                        Send();
                        //wait until buffer gets 
                        do {
                            wait_us(500);
                        }while(get_queue_size(Q_Write) != 0);
                        Read_Serial();
                        //read Key
                        Disable();
                        break;
                    }

                    Read_Serial();//read Key
                    if (!Data.Computing) {
                        h = Image.Height;
                        w = Image.Width;
                        break;
                    }
                }
                R -= Point_R;
            }
            I += Point_I;
        }//finish Compute
        int cksum;
        if (Data.Computing) {
            cksum = MSG_COMPUTE_FINISHED;
        } else {
            cksum = MSG_COMPUTE_ABORTED;
        }
        Data.Computing = false;
        q.Write[q.W_Idx] = (char) cksum;
        push_to_queue(Q_Write,(void*)&q.Write[q.W_Idx++]);
        q.Write[q.W_Idx] = Get_cksum(cksum);
        push_to_queue(Q_Write,(void*)&q.Write[q.W_Idx++]);
        Enable();
        Send();
        
    }
    return;
}

void Read_Serial()
{
    //wait_ms(10);
    void* Void_Type = get_from_queue(Q_Read, 0);
    Enable();
    if(Void_Type == NULL) {
        return;
    }
    Disable();
    char Type = *(char*) Void_Type;
    bool tmp = (Msg_size(Type) <= get_queue_size(Q_Read));
    Enable();
    if (tmp) {
        Type = pop_from_queue(Q_Read);
        switch(Type) {
            case MSG_ABORT:
                if (Control_2byte_msg(Type)) {
                    Abort(ABORT_FROM_PC);
                }
                break;
            case MSG_GET_VERSION:
                if (Control_2byte_msg(Type)) {
                    Send_Version();
                }
                break;
            case MSG_SET_COMPUTE_DATA:
                Set_Compute_Data();
                break;
            case MSG_COMPUTE:
                if (Control_2byte_msg(Type)) {
                    Data.Computing = true;
                }
                break;
            case MSG_STOP_COMPUTE:
                if (Control_2byte_msg(Type)) {
                    Data.Computing = false;
                }
                break;
            case MSG_HIGHER_BOUD_RATE:
                if (Control_2byte_msg(Type)) {
                    serial.baud(HIGH_BAUD_RATE);
                }
            default:
                pop_from_queue(Q_Read);
                break;
        }

    }
    return;
}

