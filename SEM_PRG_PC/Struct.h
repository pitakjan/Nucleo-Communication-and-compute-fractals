//#include "stdio.h"

typedef struct{	
	char Running;
	char Input_Key;
	int fd;
	int Pic_Pos_Write;
	char Nucleo_Compute;
	unsigned char Animation;
	char End_Read_Thread;
}Data_Struct;
Data_Struct Data;

typedef struct{
	int Old_Width;
	int Old_Height;
	int Width;//sirka
	int Height;//vyska
	double Start[2];
    double End[2];
    double C[2];
	int n;//number of iteration
	unsigned char *Picture;
	char Visible_Image;
}Image_Struct;
Image_Struct Image;

void Init_Data_Struct()
{
	Data.Running = TRUE;
	Data.Input_Key = 0;	
	Data.Nucleo_Compute = 0;
	Data.fd = 0;
	Data.Pic_Pos_Write = 0;
	Data.Animation = FALSE;
	Data.End_Read_Thread = FALSE;
	return;
}

void Init_Image_Struct()
{
	Image.Old_Width = DEFAULT_WIDTH;
	Image.Old_Height = DEFAULT_HEIGHT;
	Image.Width = DEFAULT_WIDTH;//sirka
	Image.Height = DEFAULT_HEIGHT;//vyska
	Image.Start[0] = START_REAL;
	Image.Start[1] = START_IMAG;
	Image.End[0] = END_REAL;
	Image.End[1] = END_IMAG;
	Image.C[0] = C_REAL;
	Image.C[1] = C_IMAG;
	Image.n = N_ITERATIONS;
	unsigned char *TMP = (unsigned char*) calloc(DEFAULT_WIDTH * DEFAULT_HEIGHT * 3, sizeof(char));
	if (TMP == NULL)
	{
		fprintf(stderr,"ERROR: Cannot allocated memory (REALLOC)\n");
		Data.Running = FALSE;
		Data.Animation = FALSE;
		return;
	}
	Image.Picture = TMP;
	Image.Visible_Image = FALSE;
	return;
}
