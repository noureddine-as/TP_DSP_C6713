#include "dsk6713_aic23.h"
#include "math.h"
#include "FiltreBasseBand_18500_22000_400Coef.inc"
#include "TablesPulsesAcoustiques.inc"

Uint32 input_sample();
void output_sample(int out_data);
void comm_poll();

#define	SEUIL_Detection 10000

Uint32 fs = DSK6713_AIC23_FREQ_48KHZ;
short Xn, Yn;
short XnBuffer[N+1], YnBuffer[N];
short i = 0, j = 0, f, fmax, Fx, canal_libre = 0, Np;
short TabTrace[8], Qn[8], Qn_1[8], Qn_2[8];
short IntgC;
short Coeff[8] = {-24636,-25997,-27246,-28378,-29389,-30274,-31029,-31651};
int CoeffMulQn_1, Xn_32bit, Qn_2_32bit, XnPlusCoeffMulQn_1, XnPlusCoeffMulQn_1MoinsQn_2;
Uint32 Qn_1MulQn_1, Qn_2MulQn_2, Qn_1MulQn_1PlusQn_2MulQn_2;
int CoeffMulQn_1;
long CoeffMulQn_1MulQn_2;
Uint32 ModuleMaximum, ModuleMaxSuivant = 0, Module[8];
unsigned short Seuil_Detection;
Uint32 t;

main()
{
	comm_poll();
	canal_libre = 0;
	for(f = 0; f < 8; f++) TabTrace[f] = 0;
	while(!canal_libre){
		for(Np = 0; Np < 40; Np++){
			IntgC = 0;
			j = 0;
			t = 0;
			Seuil_Detection = SEUIL_Detection;
			for(i = 0; i < N; i++){
				XnBuffer[i] = 0;
				YnBuffer[i] = 0;
			}
			// Réglage de seuil, en comparant avec la variable Seuil_Detection
			//while(IntgC < Seuil_Detection ){
			//	Xn = input_sample();
			//	XnBuffer[0] = Xn;
			//	if(t++ > 48000) {
			//		Seuil_Detection = (unsigned) (Seuil_Detection >> 1);
			//		t = 0;
			//	}

			while(IntgC < 15){ // ca fait passer une atténuation de 54dB
				Xn = input_sample();
				XnBuffer[0] = Xn;
				Yn = 0;
				for(i = N-1; i >= 0; i--){
					Yn = Yn + (short) ((XnBuffer[i] * h[i]) >> k);
					XnBuffer[i+1] = XnBuffer[i];
				}
				IntgC = IntgC + (short) ((abs(Yn) - IntgC) >> 8);
				YnBuffer[j] = IntgC;
				j = (j + 1) % N;
			}
			for(f = 0; f < 8; f++){
				Qn_1[f] = 0;
				Qn_2[f] = 0;
			}
			for(i = 0; i < 96; i++){
				Xn = input_sample();
				Xn_32bit = (int) (Xn << 7);
				for(f = 0; f < 8; f++){
					CoeffMulQn_1 = (int) (Coeff[f] * Qn_1[f]);	// Q22(32)
					Qn_2_32bit = (int) (Qn_2[f] << 14);	// Q22(32)
					XnPlusCoeffMulQn_1 = (int) (Xn_32bit + CoeffMulQn_1);
					XnPlusCoeffMulQn_1MoinsQn_2 = (int) (XnPlusCoeffMulQn_1 - Qn_2_32bit);
					Qn[f] = (short) (XnPlusCoeffMulQn_1MoinsQn_2 >> 14);
					Qn_2[f] = Qn_1[f];
					Qn_1[f] = Qn[f];
				}
			}
			ModuleMaximum = 0;
			for(f = 0; f < 8; f++){
				Qn_1MulQn_1 = (unsigned int) (Qn_1[f] * Qn_1[f]);
				Qn_2MulQn_2 = (unsigned int) (Qn_2[f] * Qn_2[f]);
				Qn_1MulQn_1PlusQn_2MulQn_2 = (unsigned int) (Qn_1MulQn_1 + Qn_2MulQn_2);
				CoeffMulQn_1 = (int) (Coeff[f] * Qn_1[f]);
				CoeffMulQn_1MulQn_2 = (long) (CoeffMulQn_1 * Qn_2[f]);
				Module[f] = (unsigned int) (Qn_1MulQn_1PlusQn_2MulQn_2 - (CoeffMulQn_1MulQn_2 >> 14));
				if(Module[f] > ModuleMaximum){
					ModuleMaximum = Module[f];
					fmax = f;
				}
			}
			ModuleMaxSuivant = 0;
			for(f = 0; f < 8; f++){

				if( (Module[f] < ModuleMaximum) && (Module[f] > ModuleMaxSuivant) )	ModuleMaxSuivant = Module[f];
				//if(Module[f] < ModuleMaximum){
				//	if(Module[f] > ModuleMaxSuivant)	ModuleMaxSuivant = Module[f];
				//}
			}
			if( ModuleMaximum > (ModuleMaxSuivant << 2) ) TabTrace[fmax] = 1;
		}
		for(f = 0; f < 8; f++){
			if(TabTrace[f] == 0){
				Fx = f;
				canal_libre = 1;
			}
		}
	}

	while(1){
		for(i = 0; i < 96; i++) 			output_sample(TablePulse[Fx][i]);
		for(i = 0; i < (48000 - 97); i++)	output_sample(0);
	}
}
