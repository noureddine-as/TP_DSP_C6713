#include "dsk6713_aic23.h" // Converter

Uint32 input_sample();
void output_sample(int out_data);
void comm_intr();

Uint32 fs = DSK6713_AIC23_FREQ_8KHZ;

short SinTable[64] = {0,401,799,1189,1567,1931,2276,2598,2896,3166,3406,3612,3784,3920,4017,4076,4096,4076,4017,3920,3784,3612,3406,3166,2896,2598,2276,1931,1567,1189,799,401,0,-401,-799,-1189,-1567,-1931,-2276,-2598,-2896,-3166,-3406,-3612,-3784,-3920,-4017,-4076,-4096,-4076,-4017,-3920,-3784,-3612,-3406,-3166,-2896,-2598,-2276,-1931,-1567,-1189,-799,-401};
short Xn;
short XnBuffer[64];// 16 bit signed

unsigned i = 0 , j = 0;
unsigned GAIN = 1 , STEP = 1;
interrupt void c_int11()
{
	Xn = GAIN * SinTable[j];
	j = (j + STEP) % 64;
	XnBuffer[i] = Xn;
	i = (i + 1) % 64;

	output_sample(Xn);
	return;
}

main()
{
	comm_intr(); // Init DSP
	while(1);
}

