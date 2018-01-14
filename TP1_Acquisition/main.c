#include "dsk6713_aic23.h" // Converter

Uint32 input_sample();
void output_sample(int out_data);
void comm_intr();

Uint32 fs = DSK6713_AIC23_FREQ_8KHZ;
short Xn;
short XnBuffer[64];// 16 bit signed

unsigned i = 0;

interrupt void c_int11() // receiving a serial signal will launch this interrupt
{
	Xn = input_sample();
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

