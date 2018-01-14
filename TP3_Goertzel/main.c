#include "dsk6713_aic23.h"
#include "FiltreBasseBand_18500_22000_400Coef.inc"
#include "TablesPulsesAcoustiques.inc"
#include <math.h>




#define NP 40
#define   SEUIL_DETECTION 1000
#define RC 128
#define L 96

Uint32 input_sample();
void output_sample(int out_data);
void comm_poll();

Uint32 fs = DSK6713_AIC23_FREQ_48KHZ;
short Xn, Yn;
short XnBuffer[N+1], YnBuffer[N+1];
short i = 0, j = 0, f,np =0,  canal_libre = 0, pulse_detect = 0, Module_Maximum=0, fmax=0, Fx=-1;
short TabTrace[8] = {0}, IntgC= 0, Qn_1[8]={0}, Qn_2[8]={0}, Qn[8]={0},
		Coeff[8]={-24636	,-25997,	-27246,	-28378,	-29389,	-30274,	-31029,	-31651} , TabTrace[8];
unsigned int		Module[8];
int  Qn_32bit, Qn_2_32bit,  Xn_32bit, Coeff_Mul_Qn1, Module_32bit;


main()
{
	comm_poll();

	for( f =0; f<7; f++) TabTrace[f] =0;

	while(!canal_libre)
	{
		for( np =0; np<NP ;  np++)
		{
			IntgC= 0;
			j 	 = 0;
			for(i =0; i<N; i++)
			{
				XnBuffer[i] = 0;
				YnBuffer[i] = 0;

			}
			// attente detection d'un pulse
			do
			{
				// filtrage 18.5 ===> 22 KHz
				Xn = input_sample();
				Yn = 0;
				XnBuffer[0] = Xn;

				for ( i = N-1; i >=0 ; i-- )
				{
					Yn = Yn + (short) ((h[i] * XnBuffer[i]) >> k);
					XnBuffer[i+1] = XnBuffer[i];

				}

				j = (j + 1) % N;
				// Intégrateur court
				IntgC = IntgC + ( abs(Yn) -IntgC)/ RC;
				YnBuffer[j] = IntgC;
				if(IntgC > SEUIL_DETECTION) pulse_detect = 1;

			}while(!pulse_detect);


			// initialisation de Qn
			for( f =0; f<7; f++)
			{
				Qn_1[f] = Qn_2[f] = 0;

			}
			for(i = 0; i< L; i++)
			{
				Xn = input_sample();
				Xn_32bit = (int)(Xn<<7);
				for( f =0; f<7; f++)
				{
					Qn_2_32bit =(int) (Qn_2[f]<<14); // Q8 to Q22
					Coeff_Mul_Qn1 =(int) (Coeff[f]* Qn_1[f]);
					Qn_32bit  = Xn_32bit + Coeff_Mul_Qn1-Qn_2_32bit;
					Qn[f] = (short)(Qn_32bit >> 14);
					Qn_2[f] = Qn_1[f];
					Qn_1[f] = Qn[f];
				}

			}
			Module_Maximum = 0;

			for( f =0; f<7; f++)
			{
				//Qn_1_Mul_Qn_1 = (unsigned int)((((unsigned int)Qn_1[f])*(unsigned int)Qn_1[f]) << 14)
				//Module_32bit = (int)((Qn_1[f]*Qn_1[f]) << 14) + (int)()+ (int)();

				Qn_1_Mul_Qn_1 = (unsigned int)(Qn_1[f] * Qn_1[f]); // Q16
				Qn_2_Mul_Qn_2 = (unsigned int)(Qn_2[f] * Qn_2[f]); // Q16
				Coeff_Mul_Qn_1 = (int)(Qn_1[f]*Coeff[f]);
				Qn_1_Mul_Qn_2_Mul_Coeff = (long)(Coeff_Mul_Qn_1*Qn_2[f]);


				Module[f] = (unsigned int)(Qn_1_Mul_Qn_1 + Qn_2_Mul_Qn_2 - Qn_1_Mul_Qn_2_Mul_Coeff >> 14)   // Qn_1[f]*Qn_1[f] - Qn_2[f]*Qn_2[f] -Qn_1[f]*Qn_2[f] *Coeff[f];
				if(Module[f] >Module_Maximum )
				{
					Module_Maximum = Module[f];
					fmax = f;
				}
			}
			TabTrace[fmax] = 1;
		}

		for( f =0; f<7; f++)
		{
			if(TabTrace[f] == 0) canal_libre =1;
			Fx = f;
		}
	}
	while(1)
	{
		for(i=0; i<L; i++){
			output_sample(TablePulse[Fx][i]);
		}
		for(i=0; i<=(4800-97); i++)
		{
			output_sample(0);
		}

	}



}
