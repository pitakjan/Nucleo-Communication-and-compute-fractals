#include "Remote_functions.h"

//threads
void *Read_Serial(void *arg);
void *Read_Key(void *arg);
void *Update_Image();

int main(int argc, char *argv[])
{

	int ret = 0;
	//name of mbed in computer
	const char *serial = argc > 1 ? argv[1] : LOCATION_OF_NUCLEO; 
	int fd = serial_open(serial);
	if (fd == -1 || fd == ERROR_RETURN)
	{//control open serial
  	 	fprintf(stderr,"ERROR: Cannot open serial port.\n");
  	 	return ERROR_RETURN;
	}
	printf("INFO: Serial port was opened on port %i\n", fd);
	Init_Struct();
	Data.fd = fd;
	printf("INFO: Waiting for Start message from nucleo\n");
	if (Test_Serial())
	{
		Free_Memory();
		return ERROR_RETURN;
	}
	
	//start threads
	int Number_of_threads = 3;
    pthread_t Vlakna[Number_of_threads];
	pthread_create(&Vlakna[0],NULL,Read_Key,&fd);
	pthread_create(&Vlakna[1],NULL,Read_Serial,&fd);
    pthread_create(&Vlakna[2],NULL,Update_Image,NULL);
   
	pthread_cond_init(&convar, NULL);
	pthread_mutex_init(&mtx, NULL);
	//store terminal settings and set it to raw mode
	call_termios(0); 
	//START_PROGRAM
	
	
	
	
    //Wait for end of the program
	for (int i = 1; i < Number_of_threads; i++)
	{
		pthread_join(Vlakna[i],0);
	}
	if (Data.End_Read_Thread)
	{//Join Read_thread if the thread reach end
		pthread_join(Vlakna[0],0);
		printf("INFO: Thread Read_Key() was succesfull joined.\n");
	}else
	{//Cancel Read_thread if thread are blocked on "getchar()"
		pthread_cancel(Vlakna[0]);
		printf("INFO: Thread Read_Key() was succesfull canceled.\n");
	}
	serial_close(fd);//close serial
    printf("\n");//enter on the end
    call_termios(1); //set console back to normal
    if (Image.Visible_Image)
    {//close image window
    	xwin_close();
    }
    Free_Memory();
	return ret;
}



//________________THREADS_____________________


void *Read_Key(void *arg)
{
	printf("INFO: Thread Read_Key() Started\n");
	int *fd;
	fd = (int*) arg;
	while (Data.Running) 
	{// read from keyboard
	
		char c = getchar();//this thread take break here
		c = Do_Key(c,fd);
		if (c == MSG_ABORT)
		{//end all after received end_msg from nucleo
			break;
		}
	}//end while
	Data.End_Read_Thread = TRUE;
	return NULL;
}//end function

void *Update_Image()
{//also use this thread for animation
	printf("INFO: Thread Update_Image() Started\n");
	while (Data.Running)
	{
		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
 			Catch_SDL_events(event);
 		}
 		sleep(1);
		if (Data.Animation)
		{
			float Save_c[2] = {Image.C[0], Image.C[1]};
			int Save_n = Image.n;
			Image.n = 20;
			Animate();
			Image.n = Save_n;
			Image.C[0] = Save_c[0];
			Image.C[1] = Save_c[1];
		}
	}
	printf("INFO: Thread Update_Image() was succesfull joined.\n");
	return NULL;
}

//read from serial
void *Read_Serial(void *arg)
{
	printf("INFO: Thread Read_Serial() Started\n");
	int *fd;
	fd = (int*) arg;
	int Counter = 0;
	while (Data.Running)
	{
		char c = 0;
		int Control;
		do {
			Control = serial_getc_timeout(*fd, REGULAR_WAIT_FOR_RESPONSE, &c);
			if (!Data.Running)
			{
				c = -1;
				break;
			}
		} while(Control != 1);
		char Buff[LEN_STARTUP_MSG];
		int cksum = 0;
		pthread_mutex_lock(&mtx);
		switch (c)
		{
			case MSG_OK:
				pthread_mutex_unlock(&mtx);
				while(c == MSG_TEST)
				{
					printf("koko\n");
					serial_getc_timeout(*fd, REGULAR_WAIT_FOR_RESPONSE, &c);
				}
				pthread_mutex_lock(&mtx);
				if (c == MSG_END_TEST)
				{
					printf("INFO: Nucleo was restarted.\n");
					printf("INFO: Test of communication was succesfull.\n");
					Init_Struct();
					printf("INFO: Data was restored to default.\n");
					
				}else{
					printf("INFO: Nucleo send OK.\n");
				}
				break;
			case MSG_STARTUP:
				cksum = MSG_STARTUP;
				for (int i = 0; i < LEN_STARTUP_MSG; i++)
				{
					serial_getc_timeout(*fd, REGULAR_WAIT_FOR_RESPONSE, &Buff[i]);
					cksum += Buff[i];
				}
				char tmp;
				serial_getc_timeout(*fd, REGULAR_WAIT_FOR_RESPONSE, &tmp);
				cksum += tmp;
				cksum = cksum % 128;// ???????
				if (cksum != MAX_CHAR)
				{
					printf("ERROR: Cksum does not respond\n");
				}
				printf("INFO: Nucleo send startup message\n");
				for (int i = 0; i < LEN_STARTUP_MSG; i++)
				{
					printf("%c",Buff[i]);
				}
				printf("\n");
				break;
			case MSG_VERSION:
				cksum = MSG_VERSION;
				for (int i = 0; i < 4; i++)
				{//major,minor,patch, cksum
					serial_getc_timeout(*fd, REGULAR_WAIT_FOR_RESPONSE, &Buff[i]);
					cksum += Buff[i];
				}
				if (cksum == MAX_CHAR)
				{
					printf("INFO: Received Nucleo version: %i.%ip%i\n",Buff[0],Buff[1],Buff[2]);
				}else{					
					fprintf(stderr,ERROR_CKSUM_DOES_NOT_RESPOND);
				}
				break;
			case MSG_ABORT:
				cksum = MSG_ABORT;
				for (int i = 0; i < 2; i++)
				{					
					serial_getc_timeout(*fd, REGULAR_WAIT_FOR_RESPONSE, &Buff[i]);
					cksum += Buff[i];
				}
				if (cksum == MAX_CHAR)
				{
					if (Buff[0] == ABORT_FROM_PC)
					{
						printf("INFO: Nucleo was ended, NUCLEO SEND: ABORT FROM PC\n");	
					}else if (Buff[0] == ABORT_FROM_NUCLEO)
					{
						printf("INFO: Nucleo was ended, NUCLEO SEND: ABORT FROM NUCLEO\n");	
						Data.End_Read_Thread = FALSE;
					}else
					{
						fprintf(stderr,"ERROR: ABORT FROM NUCLEO, CANNOT ALLOC MEMORY\n");
					}
					Data.Animation = FALSE;
					Data.Running = FALSE;
				}else{
					fprintf(stderr,ERROR_CKSUM_DOES_NOT_RESPOND);
				}
				break;
			case ERROR_CKSUM_NOT_RESPOND:
				cksum = ERROR_CKSUM_NOT_RESPOND;
				serial_getc_timeout(*fd, REGULAR_WAIT_FOR_RESPONSE, &Buff[0]);
				cksum += Buff[0];
				if (cksum == MAX_CHAR)
				{
					printf("ERROR: Nucleo send: cksum does not respond\n");
				}else{
					fprintf(stderr,ERROR_CKSUM_DOES_NOT_RESPOND);
				}
				break;
			case MSG_COMPUTE_DATA:
				cksum = MSG_COMPUTE_DATA;
				for (int i = 0; i < 4; i++)
				{
					serial_getc_timeout(*fd, REGULAR_WAIT_FOR_RESPONSE, &Buff[i]);
					if (i == 3)
					{
						cksum = cksum % 128;
						cksum -= 128;
					}
					cksum += Buff[i] + 128;
				}
				if (cksum == MAX_CHAR)
				{
					for (int i = 0; i < 3; i++)
					{
						Image.Picture[Data.Pic_Pos_Write++] = (unsigned char)((int)Buff[i]+128);
					}
					if (Image.Width == Counter++)
					{
						
						printf("INFO: Chunk ID %i received %ix times, Row %i was added.\n",c,Counter -1,(Data.Pic_Pos_Write/3)/Image.Width);	
						Counter = 0;					
					}
				}else{
					fprintf(stderr,ERROR_CKSUM_DOES_NOT_RESPOND);
				}
				
				break;
			case MSG_COMPUTE_FINISHED:
				cksum = MSG_COMPUTE_FINISHED;
				serial_getc_timeout(*fd, REGULAR_WAIT_FOR_RESPONSE, &Buff[0]);
				cksum += Buff[0];
				if (cksum == MAX_CHAR)
				{
					printf("INFO: Nucleo send: Computed finished.\n");
					Show_Image();	
					Data.Pic_Pos_Write = 0;	
					Data.Nucleo_Compute = FALSE;		
				}else{
					fprintf(stderr,ERROR_CKSUM_DOES_NOT_RESPOND);
				}
				break;
			case MSG_COMPUTE_ABORTED:
				cksum = MSG_COMPUTE_ABORTED;
				serial_getc_timeout(*fd, REGULAR_WAIT_FOR_RESPONSE, &Buff[0]);
				cksum += Buff[0];
				if (cksum == MAX_CHAR)
				{
					printf("INFO: Nucleo send: Computed aborted.\n");
					Show_Image();	
					Data.Pic_Pos_Write = 0;	
					Data.Nucleo_Compute = FALSE;				
				}else{
					fprintf(stderr,ERROR_CKSUM_DOES_NOT_RESPOND);
				}
				
			
				break;
			case -1://exit thread
				break;
			default:
				printf("INFO: Nucleo send not defined char: %i %c\n", c, c);
				break;
		}
		pthread_mutex_unlock(&mtx);
	}
	printf("INFO: Thread Read_Serial() was succesfull joined.\n");
	return NULL;	
}



