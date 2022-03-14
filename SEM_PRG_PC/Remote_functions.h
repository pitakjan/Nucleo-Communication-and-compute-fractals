#include "message.h"
#include "SDL.h"
#include "Struct.h"




void call_termios(int reset);
void Send_Comp_Parameters();
int serial_putc(int fd, char c);
int serial_getc_timeout(int fd, int timeout_ms, char *c);
char Do_Key(char c, int*fd);

pthread_mutex_t mtx;
pthread_cond_t convar;

// Control functions
char Animation_Control()
{
	if (Data.Animation != FALSE)
	{
		printf("WARN: Stop animation before this action!\n");
		return FALSE;
	}
	return TRUE;
}

int Nucleo_Control()
{
	if (Data.Nucleo_Compute != FALSE)
	{
		printf("WARN: Wait or abort compute on nucleo before this action\n");
		return FALSE;
	}
	return TRUE;
}

// Other functions

void Abort()
{
	printf("INFO: Program will exit.\n");
	Data.End_Read_Thread = FALSE;
	Data.Animation = FALSE;
	Data.Running = FALSE;
	return;
}
char Get_cksum(int cksum)
{
    cksum = cksum % 128;
    return (char)-cksum;

}

int Test_Serial()
{//test comunication with nucleo
	char c;
	int Control;
	for (int i  = 0; i < 500; i++)
	{//clear serial port
		serial_getc_timeout(Data.fd, 1, &c);
	}
	serial_getc_timeout(Data.fd, WAIT_FOREVER, &c);
	do
	{
		Control = serial_getc_timeout(Data.fd, REGULAR_WAIT_FOR_RESPONSE, &c);
		if (Control == -1)
		{
  	 		fprintf(stderr,"ERROR: Test of Serial port failed.\n");
  	 		fprintf(stderr,"INFO: Program exit.\n");
  	 		return ERROR_RETURN;
		}
	}while(c != MSG_END_TEST);
	printf("INFO: Test of serial port completed successful.\n");
	return 0;
}

void Init_Struct()
{
	Init_Data_Struct();
	Init_Image_Struct();
	return;
}

void Free_Memory()
{
	free(Image.Picture);
	return;
}

// if image are shown, Redraw image with actual data, else show image
void Show_Image()
{
	if (Image.Visible_Image && (Image.Old_Width != Image.Width || Image.Old_Height != Image.Height))
	{
		Image.Old_Width = Image.Width;
		Image.Old_Height = Image.Height;
		xwin_close();
		Image.Visible_Image = FALSE;
	}
	if (!Image.Visible_Image)
	{
		xwin_init(Image.Width,Image.Height);
	}
		
	xwin_redraw(Image.Width,Image.Height,Image.Picture);
	Image.Visible_Image = TRUE;
	return;
}

//return Absolute value of imaginary number
float Abs_value(float R, float I)
{
	return sqrt(R*R+I*I);	
}

//compute image by PC
void PC_Compute()
{	
	float Rozdil_R = Image.End[0]-Image.Start[0];
	float Rozdil_I = Image.End[1]-Image.Start[1];
	float Point_R = (float)Rozdil_R / Image.Width;
	float Point_I = (float)Rozdil_I / Image.Height;
	int Image_Index = 0;
	float I = Image.Start[1];
	pthread_mutex_unlock(&mtx);
	for (int h = 0; h < Image.Height; h++)
	{
		float R = Image.End[0];
		for (int w = 0; w < Image.Width; w++)
		{
			float Z[2] = {R,I};
			for(int n = 0; n < Image.n; n++)
			{
				float tmp = Z[0];
				Z[0] = (tmp*tmp) - (Z[1]*Z[1]) + Image.C[0]; 
				Z[1] = 2*tmp*Z[1] + Image.C[1];
				if (Abs_value(Z[0], Z[1]) >= 2)
				{	
					double t = (double )n/Image.n;
					Image.Picture[Image_Index++] = round(9*(1-t)*(t*t*t)*255);
					Image.Picture[Image_Index++] = round(15*((1-t)*(1-t))*(t*t)*255);
					Image.Picture[Image_Index++] = round(8.5*((1-t)*(1-t)*(1-t))*t*255); 		
					break;
				}
				else if (n == Image.n - 1)
				{
					Image.Picture[Image_Index++] = BLACK_COLOR;
					Image.Picture[Image_Index++] = BLACK_COLOR;
					Image.Picture[Image_Index++] = BLACK_COLOR;
				}
			}
			R -= Point_R;
		}
		I += Point_I;
	}
	pthread_mutex_lock(&mtx);
	Show_Image();
	return;
	
}

//change Image size by preset values
void Change_img_size(char c)
{
	switch (c)
	{
		case '5':
			Image.Width = DEFAULT_WIDTH_MOD_5;
			Image.Height = DEFAULT_HEIGHT_MOD_5;
			break;
		case '6':
			Image.Width = DEFAULT_WIDTH_MOD_6;
			Image.Height = DEFAULT_HEIGHT_MOD_6;
			break;
		case '7':
			Image.Width = DEFAULT_WIDTH_MOD_7;
			Image.Height = DEFAULT_HEIGHT_MOD_7;
			break;
		default:
			return;
	}
	unsigned char * TMP = (unsigned char*) realloc(Image.Picture, Image.Height*Image.Width*3*sizeof(char));
	if (TMP == NULL)
	{
		fprintf(stderr,"ERROR: Cannot allocated memory (REALLOC)\n");
		Abort();
	}
	Image.Picture = TMP;
	for (int i = 0; i < Image.Height*Image.Width*3; i++)
	{
		Image.Picture[i] = BLACK_COLOR;
	}
	Show_Image();
	Send_Comp_Parameters();
}

//change image size - by numeric keyboard, write, not interactive
void Set_image_size()
{
	call_termios(1);
	int tmp;
	int Control;
	pthread_mutex_unlock(&mtx);
	printf("\nSet image size \n");
	do{
		printf("Width: ");
		Control = scanf("%i", &tmp);
		if (Control != 1 || tmp > MAX_IMAGE_WIDTH || tmp < MIN_IMAGE_WIDTH)
		{
			while(getchar() != '\n');//clear console
			printf("ERROR: Input interger, smaller than 1000 and bigger than 32");
			Control = 0;
		}else{
			Image.Width = tmp;	
		}
	}
	while(Control != 1);
	
	do{
		printf("Height: ");
		Control = scanf("%i", &tmp);
		if (Control != 1 || tmp > MAX_IMAGE_HEIGHT || tmp < MIN_IMAGE_HEIGHT)
		{
			while(getchar() != '\n');
			printf("ERROR: Input interger, smaller than 1000 and bigger than 32");
			Control = 0;
		}else{
			Image.Height = tmp;	
		}
	}
	while(Control != 1);
	getchar();//free '\n'
	unsigned char * TMP = (unsigned char*) realloc(Image.Picture, Image.Height*Image.Width*3*sizeof(char));
	if (TMP == NULL)
	{
		fprintf(stderr,"ERROR: Cannot allocated memory (REALLOC)\n");
		Abort();
	}
	pthread_mutex_lock(&mtx);
	Image.Picture = TMP;
	for (int i = 0; i < Image.Height*Image.Width*3; i++)
	{
		Image.Picture[i] = BLACK_COLOR;
	}
	call_termios(0);
	Send_Comp_Parameters();
}


//set compute parameters interactive
void Set_Parameters()
{	
	pthread_mutex_unlock(&mtx);
	int Control = 0;
	_Bool Redraw = true;
	printf("Enter for exit, Space for on/off to redraw actual image\n");
	printf("\rFrom - R: %.1f -> %.1f, I: %.1f -> %.1f, Const C - R: %.1f, I: %.1f, Iter: %i",
	Image.Start[0], Image.End[0],Image.Start[1], Image.End[1], Image.C[0], Image.C[1], Image.n);
	int c = getchar();
	while (c != 13) {//proc ?? c != '\n'
		if ( c == '\033') { // if the first value is esc
			getchar(); // skip the '['
			switch(getchar()) { // Arrow
				case 'A'://up
				    switch(Control)
				    {
				    	case 0:
				    		if (Image.Start[0] < Image.End[0] - STEP_CHANGE_VALUE)
				    		{
								Image.Start[0] += STEP_CHANGE_VALUE;				    			
				    		}
				    		break;
				   		case 1:
				    		Image.End[0] += STEP_CHANGE_VALUE;
				    		break;
				    	case 2:
				    		if (Image.Start[1] < Image.End[1] - STEP_CHANGE_VALUE)
				    		{
				    			Image.Start[1] += STEP_CHANGE_VALUE;
				    		}
				    		break;
				    	case 3:
				    		Image.End[1] += STEP_CHANGE_VALUE;
				    		break;
				    	case 4:
				    		Image.C[0] += STEP_CHANGE_VALUE;
				    		break;
				    	case 5:
				    		Image.C[1] += STEP_CHANGE_VALUE;
				    		break;
				    	case 6:
				    		Image.n += 1;
				    		break;
				    	default:
				    		break;
				    }
				    break;
				case 'B'://down
				    switch(Control)
				    {
				    	case 0:
							Image.Start[0] -= STEP_CHANGE_VALUE;				    			
				    		break;
				   		case 1:
				   			if (Image.Start[0] != Image.End[0] - STEP_CHANGE_VALUE)
				   			{
								Image.End[0] -= STEP_CHANGE_VALUE;				   				
				   			}
				    		break;
				    	case 2:
				    		Image.Start[1] -= STEP_CHANGE_VALUE;
				    		break;
				    	case 3:
				    	if (Image.Start[1] != Image.End[1] - STEP_CHANGE_VALUE)
				   			{
				    			Image.End[1] -= STEP_CHANGE_VALUE;
				    		}
				    		break;
				    	case 4:
				    		Image.C[0] -= STEP_CHANGE_VALUE;
				    		break;
				    	case 5:
				    		Image.C[1] -= STEP_CHANGE_VALUE;
				    		break;
				    	case 6:
				    		if (Image.n != 1)
				    		{
					    		Image.n -= 1;				    			
				    		}
				    		break;
				    	default:
				    		break;
				    }
				    break;
				case 'C'://right
					if (Control != 6)
					{
						Control += 1; 
					}
				    break;
				case 'D'://left
				    if (Control != 0)
					{
						Control -= 1; 
					}
				    break;
				default:
				   break;
			}
		}else
		{
			if (c == ' ')
			{
				Redraw = Redraw == 1 ? false : true;
			}
		}
		char Color[] = COLOR_SET_PARAMETERS;
		char End[] = "\033[0m"; 
		printf("\r ");
		for	(int a = 0; a < 80; a++)
		{//clear line
			printf(" ");
		}
		switch(Control)
		{
			case 0:
				printf("\rFrom - R: %s%.1f%s -> %.1f, I: %.1f -> %.1f, Const C - R: %.1f, I: %.1f, Iter: %i",
				Color,Image.Start[0],End, Image.End[0],Image.Start[1], Image.End[1], Image.C[0], Image.C[1], Image.n);
				break;
			case 1:
				printf("\rFrom - R: %.1f -> %s%.1f%s, I: %.1f -> %.1f, Const C - R: %.1f, I: %.1f, Iter: %i",
				Image.Start[0],Color, Image.End[0],End,Image.Start[1], Image.End[1], Image.C[0], Image.C[1], Image.n);
				break;
			case 2:
				printf("\rFrom - R: %.1f -> %.1f, I: %s%.1f%s -> %.1f, Const C - R: %.1f, I: %.1f, Iter: %i",
				Image.Start[0], Image.End[0],Color,Image.Start[1],End, Image.End[1], Image.C[0], Image.C[1], Image.n);
				break;
			case 3:
				printf("\rFrom - R: %.1f -> %.1f, I: %.1f -> %s%.1f%s, Const C - R: %.1f, I: %.1f, Iter: %i",
				Image.Start[0], Image.End[0],Image.Start[1], Color, Image.End[1],End, Image.C[0], Image.C[1], Image.n);
				break;
			case 4:
				printf("\rFrom - R: %.1f -> %.1f, I: %.1f -> %.1f, Const C - R: %s%.1f%s, I: %.1f, Iter: %i",
				Image.Start[0], Image.End[0],Image.Start[1], Image.End[1],Color, Image.C[0],End, Image.C[1], Image.n);
				break;
			case 5:
				printf("\rFrom - R: %.1f -> %.1f, I: %.1f -> %.1f, Const C - R: %.1f, I: %s%.1f%s, Iter: %i",
				Image.Start[0], Image.End[0],Image.Start[1], Image.End[1], Image.C[0],Color, Image.C[1],End, Image.n);
				break;
			case 6:
				printf("\rFrom - R: %.1f -> %.1f, I: %.1f -> %.1f, Const C - R: %.1f, I: %.1f, Iter: %s%i%s",
				Image.Start[0], Image.End[0],Image.Start[1], Image.End[1], Image.C[0], Image.C[1],Color, Image.n,End);
				break;
			default:
				break;
		}
		if (Redraw)
		{
			pthread_mutex_lock(&mtx);
			PC_Compute();
			pthread_mutex_unlock(&mtx);
		}
		c = getchar();
	}
	printf("\n");
	pthread_mutex_lock(&mtx);
	Send_Comp_Parameters();
	return;
}


//Send compute parameters to nucleo
void Send_Comp_Parameters()
{/*send: type, Width, Height, Start_R, End_R, C_R, Start_I, End_I, C_I, n, cksum */
	int cksum = MSG_SET_COMPUTE_DATA + Image.Width + Image.Height + Image.n;
	int Control = 0;
	Control += serial_putc(Data.fd, MSG_SET_COMPUTE_DATA);
    Control += write(Data.fd, &Image.Width, 4);
    Control += write(Data.fd, &Image.Height, 4);
	for (int i = 0; i < 2; i++)
	{
		cksum += (int) Image.Start[i]*10;
		cksum += (int) Image.End[i]*10;
		cksum += (int) Image.C[i]*10;
		Control += write(Data.fd, &Image.Start[i], 8);
		Control += write(Data.fd, &Image.End[i], 8);
		Control += write(Data.fd, &Image.C[i], 8);
	}
	Control += write(Data.fd, &Image.n, 4);
	Control += serial_putc(Data.fd, Get_cksum(cksum));
	if (Control != (1 + 4*3 + 8*6 + 1))
	{
		printf("ERROR: Error in write to serial port %i", Data.fd);
		Abort();
	}
	return;
}

void Catch_SDL_events(SDL_Event event)
{
	if (event.type == SDL_WINDOWEVENT && 
 	event.window.event == SDL_WINDOWEVENT_CLOSE)
 	{//click on cross - close image
 		printf("INFO: Send request to close image.\n");
 		xwin_close();
 	}
 	else if(event.type == SDL_KEYDOWN){
 		char c = event.key.keysym.scancode +  'a' - 4;//letters
 		char n = event.key.keysym.scancode + 19;//numbers
 		if ((c >= 'a' && c <= 'z') && c != 's')
 		{
 			Do_Key(c, &Data.fd);
 		}else if(n >= '1' && n <= '7'){
 			Do_Key(n, &Data.fd);
 		}
 	}
 	return;
}
//start animation, there were 3 modes of animation
void Animate()
{
	float t = 0;
	while(Data.Animation)
	{
		pthread_mutex_lock(&mtx);
		PC_Compute();
		if (Data.Animation == '2')
		{
			Image.C[0] = sin(t);
			Image.C[1] = sin(t);			
		}
		else if (Data.Animation == '3')
		{
			Image.C[0] = sin(t);
			Image.C[1] = cos(t);	
		}else if(Data.Animation == '4')
		{
			Image.C[0] = cos(t)*cos(t);
			Image.C[1] = sin(t)*sin(t)-cos(t);		
		}
		pthread_mutex_unlock(&mtx);
		t += 0.04;
		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
 			Catch_SDL_events(event);
 		}
		usleep(50);
	}
}

char Do_Key(char c, int*fd)
{
	char Write_to_serial = FALSE;
	pthread_mutex_lock(&mtx);
	Data.Input_Key = c;
	switch (c)
	{
		case '2'://start animation #1
		case '3':
		case '4':
			if (Nucleo_Control())
			{
				printf("INFO: Animation #%i started.\n",c-'0');
				Data.Animation = c;					
			}else{
				printf("WARN: Stop compute on nucleo to start animation\n");
			}
			break;
		case '5':
		case '6':
		case '7':
			if (Animation_Control() && Nucleo_Control())
			{
				Change_img_size(c);
			}
			break;
		case 'b'://stop animation
			if (Data.Animation != FALSE)
			{
				printf("INFO: Animation is stopped.\n");
				Data.Animation = FALSE;					
			}else{
				printf("INFO: Animation was already stopped.\n");
			}
			break;
		case 'g'://Get Firmware (MSG_GET_VERSION) 
			printf("INFO: Request nucleo version\n");
			Write_to_serial = TRUE;
			break;
		case 's'://nastaví parametry výpočtu (MSG_SET_COMPUTE) 
			if (Animation_Control() && Nucleo_Control())
			{
				Set_Parameters();
			}
			break;
		case 'i'://set Image size
			if (Animation_Control() && Nucleo_Control())
			{
				Set_image_size();
			}
			break;
		case '1'://spustí výpočet (MSG_COMPUTE) 
			if (Animation_Control())
			{
				Data.Nucleo_Compute = TRUE;
				Write_to_serial = TRUE;
				Data.Pic_Pos_Write = 0;
			}
			break;
		case 'a'://přeruší probíhající výpočet	
			if (Data.Nucleo_Compute)
			{
				Write_to_serial = TRUE;								
				printf("INFO: Send request to nucleo - Abort Compute.\n");
			}else{
				printf("WARN: Nucleo do not computing!\n");
			}
			break;
		case 'r'://resetuje cid 
			printf("INFO: CID is on zero.\n");
			Data.Pic_Pos_Write = 0;//picture position write
			break;
		case 'l':// smaže aktuální obsah výpočtu (bufferu) 
			for (int i = 0; i < Image.Height*Image.Width*3; i++)
			{
				Image.Picture[i] = BLACK_COLOR;
			}
			printf("INFO: Current image was deleted.\n");
			break;
		case 'p'://překreslí obsah okna aktuálním stavem výpočtu (bufferem) 
			printf("INFO: Redraw image.\n");
			Show_Image();
			break;
		case 'c'://spočte fraktál na PC
			if (Animation_Control() && Nucleo_Control())
			{
				printf("INFO: Image will be computed on PC.\n");
				PC_Compute();
			}
			break;	
		case 'q'://korektně ukončí program terminací jednotlivých vláken a ukončením hlavního vlákna programu voláním 
			printf("INFO: Send request to exit program\n");
			Write_to_serial = TRUE;
			break;	
		case 'm': //show Menu
			Print_menu();
			break;
		case 'd'://show chunk Index
			printf("Index: %i\n",Data.Pic_Pos_Write/3);
			break;
		case 'x'://close image
			if (Animation_Control())
			{
				if (Image.Visible_Image)
				{
					printf("INFO: Send request to close image.\n");
					xwin_close();	
					Image.Visible_Image = FALSE;			
				}else
				{
					printf("WARN: Image are not shown. No image to close.\n");
				}
			}	
 			break;
		default:
			printf("WARN: Input invalid character\n");
			break;
	}//end switch
	if (Write_to_serial)
	{
		int r = serial_putc(*fd, c);//send input to serial
		int b = serial_putc(*fd,Get_cksum(c));
		if (r == -1 || b == -1) 
		{
			fprintf(stderr, "ERROR: Error in write to serial port\n");
			printf("Program will exit\n");
			Data.Animation = FALSE;
			Data.Running = FALSE;
			c = MSG_ABORT;
		} 
	}//end if
	pthread_mutex_unlock(&mtx);
	return c;
}


//________________managed FAIGL's FUNCTIONS_____________________


//0 to save setting, 1 to restore default
void call_termios(int reset)
{
   static struct termios tio, tioOld;
   tcgetattr(STDIN_FILENO, &tio);
   if (reset) {
      tcsetattr(STDIN_FILENO, TCSANOW, &tioOld);
   } else {
      tioOld = tio; //backup 
      cfmakeraw(&tio);
      tio.c_oflag |= OPOST;
      tcsetattr(STDIN_FILENO, TCSANOW, &tio);
   }
}

int serial_open(const char *fname)
{
   int fd = open(fname, O_RDWR | O_NOCTTY | O_SYNC);
   if (fd == -1)
   {
   		Abort();
   		return ERROR_RETURN;
   };
   struct termios term;
   assert(tcgetattr(fd, &term) >= 0);
   cfmakeraw(&term);
   term.c_cc[VTIME] = 0; //set vtime 
   term.c_cc[VMIN] = 1;
   cfsetispeed(&term, BAUD_RATE);
   cfsetospeed(&term, BAUD_RATE);
   assert(tcsetattr(fd, TCSADRAIN, &term) >= 0);
   assert(fcntl(fd, F_GETFL) >= 0);
   assert(tcsetattr(fd, TCSADRAIN, &term) >= 0);
   assert(fcntl(fd, F_GETFL) >= 0);
   tcflush(fd, TCIFLUSH);
   tcsetattr(fd, TCSANOW, &term);

   // Set the serial port to non block mode
   int flags = fcntl(fd, F_GETFL);
   flags &= ~O_NONBLOCK;
   assert(fcntl(fd, F_SETFL, flags) >= 0);
   return fd;
}

int serial_close(int fd)
{
   return close(fd);
}

int serial_putc(int fd, char c)
{
   return write(fd, &c, 1);
}

int serial_getc(int fd)
{
   char c;
   int r = read(fd, &c, 1);
   return r == 1 ? c : -1;
}

int serial_getc_timeout(int fd, int timeout_ms, char *c)
{
   struct pollfd ufdr[1];
   int r = 0;
   ufdr[0].fd = fd;
   ufdr[0].events = POLLIN | POLLRDNORM;
   if ((poll(&ufdr[0], 1, timeout_ms) > 0) && (ufdr[0].revents & (POLLIN | POLLRDNORM))) {
      r = read(fd, c, 1);
   }
   return r;
}

//____________Not used function____________//


/*This function below is to set compute parameters by keybord-write, not interactive*/

/*void Write_Parameters()
{
	//Default parameters
	//This paragraph is to show to user
	#define START_REAL_C "-1.6"
	#define START_IMAG_C "-1.1"
	#define END_REAL_C "+1.6"
	#define END_IMAG_C "+1.1"
	#define C_REAL_C "-0.4"
	#define C_IMAG_C "+0.6"
	#define N_ITERATIONS_C "30"
	call_termios(1);
	do{
		double tmp = 0;
		int TMP = 0;
		printf("\nSet computating Parameters, or send any letter for default \n");
		printf("Default: START = " START_REAL_C START_IMAG_C "j, END = " END_REAL_C END_IMAG_C
		"j, C = " C_REAL_C C_IMAG_C  "j, N = " N_ITERATIONS_C "\n");
		printf("Compute from START to END, with cons C and N Iterations\n");
		
		printf("Real Component - START: ");
		Control = scanf("%le", &tmp);
		while(getchar() != '\n');
		Image.Start[0] = Control == 1 ? tmp : START_REAL;
		
		printf("Imaginary Component - START: ");
		Control = scanf("%le", &tmp);
		while(getchar() != '\n');
		Image.Start[1] = Control == 1 ? tmp : START_IMAG;
		
		printf("Real Component - END: ");
		Control = scanf("%le", &tmp);
		while(getchar() != '\n');
		Image.End[0] = Control == 1 ? tmp : END_REAL;
		
		printf("Imaginary Component - END: ");
		Control = scanf("%le", &tmp);
		while(getchar() != '\n');
		Image.End[1] = Control == 1 ? tmp : END_IMAG;
		
		printf("Real Component - C: ");
		Control = scanf("%le", &tmp);
		while(getchar() != '\n');
		Image.C[0] = Control == 1 ? tmp : C_REAL;
		
		printf("Imaginary Component - START: ");
		Control = scanf("%le", &tmp);
		while(getchar() != '\n');
		Image.C[1] = Control == 1 ? tmp : C_IMAG;
		
		printf("Number of iterations - N: ");
		Control = scanf("%i", &TMP);
		while(getchar() != '\n');
		Image.n = Control == 1 ? TMP : N_ITERATIONS;
		
		//control of send
		if (Image.Start[0] >= Image.End[0] || Image.Start[1] >= Image.End[1])
		{
			printf("Set START smaller than END\n");
			Control = 0;
		}else{
			Control = 1;
		}
	}
	while(Control == 0);
	//call_termios(0);
}*/
