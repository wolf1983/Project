
#include <stdlib.h>
#include <string.h>
#include "WAVheader.h"
#include <math.h>

#define BLOCK_SIZE 16
#define MAX_NUM_CHANNEL 8

#define DB_TO_GAIN(X) pow(10.0, ((X) / 20.0))	//d=20*log(g), d/20=log(g), g=10^d/20

/*******************    histroy value    **********************/
double l_history_x[2] = {0, 0};
double l_history_y[2] = {0, 0};
double c_history_x[2] = {0, 0};
double c_history_y[2] = {0, 0};
double ls_history_x[2] = {0, 0};
double ls_history_y[2] = {0, 0};
double rs_history_x[2] = {0, 0};
double rs_history_y[2] = {0, 0};
double r_history_x[2] = {0, 0};
double r_history_y[2] = {0, 0};
double lfe_history_x[2] = {0, 0};
double lfe_history_y[2] = {0, 0};
/**************************************************************/

/**************************  coeficient   *********************/
double LPF_COEF[6] = { 0.56903558463233062000,
        1.13807116926466120000,
        0.56903558463233062000,
		1.00000000000000000000,
        0.94280904158206313000,
        0.33333333333333326000 };
double HPF_COEF[6] = { 0.92862571934096561000,
        -1.85725143868193120000,
        0.92862571934096561000,
		1.00000000000000000000,
        -1.85214648540773450000,
        0.86234862604085072000 };
		
double BPF_COEF[6] = { 0.56215995888375880000,
        0.00000000000000000000,
        -0.56215995888375880000,
		1.00000000000000000000,
        -0.77129294009044203000,
        -0.05240777927133287400 };
/**************************************************************/

enum Enable {ON, OFF};
enum Mode {MODE_0, MODE_1};

double sampleBuffer[MAX_NUM_CHANNEL][BLOCK_SIZE];
double sampleBufferTempL[BLOCK_SIZE];
double sampleBufferTempR[BLOCK_SIZE];

double iir_second_order(double input, double* coeficient, double* history_x, double* history_y)
{
	double sum_x = 0;
	double sum_y = 0;
	double output = 0;

	sum_x = coeficient[0] * input + coeficient[1] * history_x[0] + coeficient[2] * history_x[1];
	sum_y = coeficient[4] * history_y[0] + coeficient[5] * history_y[1];
	output = sum_x - sum_y;

	history_x[1] = history_x[0];
	history_x[0] = input;

	history_y[1] = history_y[0];
	history_y[0] = output;

	return output;
}


int main(int argc, char* argv[])	//input_file | output_file | enable | input_gain | mode
{
	//default value
	//-------------------------------------------------
	Enable enable = ON;
	double gain = DB_TO_GAIN(-3);
	Mode mode = MODE_0;
	//-------------------------------------------------
	

	FILE *wav_in = NULL;
	FILE *wav_out = NULL;
	char WavInputName[256];
	char WavOutputName[256];
	WAV_HEADER inputWAVhdr, outputWAVhdr;

	printf("Number of arguments command line: %d\n", argc);
	printf("%s\n%s\n%s\n%s\n%s\n%s\n", argv[0], argv[1], argv[2], argv[3], argv[4], argv[5]);
	if (argc < 3 || argc > 6)
	{
		printf("Wrong number of arguments\n");
		return -1;
	}

	if (argc > 5)
	{
		if (strcmp(argv[5], "MODE_0") == 0)
		{
			mode = MODE_0;
			printf("\nMode is: %s\n", argv[5]);
		}
		else if (strcmp(argv[5], "MODE_1") == 0)
		{
			mode = MODE_1;
			printf("\nMode is: %s\n", argv[5]);
		}
		else
		{
			printf("\n!!!Worng mode, set to default value\n");
			mode = MODE_0;
		}
	}

	if (argc > 4)
	{
		double gain_db = atof(argv[4]);
		if (gain_db <= 0)
			gain = DB_TO_GAIN(gain_db);
		else
			gain = DB_TO_GAIN(-3);
	}

	if (argc > 3)
	{
		if (strcmp(argv[3], "ON") == 0)
		{
			enable = ON;
			printf("Enable is: %s\n", argv[3]);
		}
		else if (strcmp(argv[3], "OFF") == 0)
		{
			enable = OFF;
			printf("Enable is: %s\n", argv[3]);
		}
		else
		{
			printf("\n!!!Worng enable, set to default value\n");
			enable = ON;
		}	
	}	

		
	// Init channel buffers
	for(int i=0; i<MAX_NUM_CHANNEL; i++)
		memset(&sampleBuffer[i],0,BLOCK_SIZE);//popunjava se sampleBuffer sa nulama

	// Open input and output wav files
	//-------------------------------------------------
	strcpy(WavInputName, argv[1]);
	wav_in = OpenWavFileForRead(WavInputName, "rb");
	strcpy(WavOutputName, argv[2]);
	wav_out = OpenWavFileForRead(WavOutputName, "wb");
	//-------------------------------------------------


	
	//-------------------------------------------------

	// Read input wav header
	//-------------------------------------------------
	ReadWavHeader(wav_in,inputWAVhdr);
	//-------------------------------------------------
	
	// Set up output WAV header
	//-------------------------------------------------	
	outputWAVhdr = inputWAVhdr;
	outputWAVhdr.fmt.NumChannels = 6;//inputWAVhdr.fmt.NumChannels; // change number of channels

	int oneChannelSubChunk2Size = inputWAVhdr.data.SubChunk2Size/inputWAVhdr.fmt.NumChannels;
	int oneChannelByteRate = inputWAVhdr.fmt.ByteRate/inputWAVhdr.fmt.NumChannels;
	int oneChannelBlockAlign = inputWAVhdr.fmt.BlockAlign/inputWAVhdr.fmt.NumChannels;
	
	outputWAVhdr.data.SubChunk2Size = oneChannelSubChunk2Size*outputWAVhdr.fmt.NumChannels;
	outputWAVhdr.fmt.ByteRate = oneChannelByteRate*outputWAVhdr.fmt.NumChannels;
	outputWAVhdr.fmt.BlockAlign = oneChannelBlockAlign*outputWAVhdr.fmt.NumChannels;


	// Write output WAV header to file
	//-------------------------------------------------
	WriteWavHeader(wav_out,outputWAVhdr);


	// Processing loop
	//-------------------------------------------------	
	{
		int sample;
		int BytesPerSample = inputWAVhdr.fmt.BitsPerSample/8;
		const double SAMPLE_SCALE = -(double)(1 << 31);		//2^31
		int iNumSamples = inputWAVhdr.data.SubChunk2Size/(inputWAVhdr.fmt.NumChannels*inputWAVhdr.fmt.BitsPerSample/8);
		
		// exact file length should be handled correctly...
		for(int i=0; i<iNumSamples/BLOCK_SIZE; i++)
		{	
			for(int j=0; j<BLOCK_SIZE; j++)
			{
				for(int k=0; k<inputWAVhdr.fmt.NumChannels; k++)
				{	
					sample = 0; //debug
					fread(&sample, BytesPerSample, 1, wav_in);
					sample = sample << (32 - inputWAVhdr.fmt.BitsPerSample); // force signextend
					sampleBuffer[k][j] = sample / SAMPLE_SCALE;				// scale sample to 1.0/-1.0 range		
				}
			}

			if (enable == ON)
			{
				if (mode == MODE_0)
				{
					//processing();
					for (int j = 0; j < BLOCK_SIZE; j++)
					{
						sampleBufferTempL[j] = gain * sampleBuffer[0][j];
						sampleBufferTempR[j] = gain * sampleBuffer[1][j];

						sampleBuffer[0][j] = sampleBufferTempL[j];																//to L		bez filtra
						sampleBuffer[1][j] = iir_second_order(sampleBufferTempL[j], LPF_COEF, c_history_x, c_history_y);		//to C		LPF 18kHz
						sampleBuffer[2][j] = iir_second_order(sampleBufferTempL[j], HPF_COEF, ls_history_x, ls_history_y);		//to Ls		HPF 800Hz
						sampleBuffer[3][j] = iir_second_order(sampleBufferTempL[j], BPF_COEF, rs_history_x, rs_history_y);		//to Rs		BPF 1200-14000Hz
						sampleBuffer[4][j] = iir_second_order(sampleBufferTempR[j], BPF_COEF, r_history_x, r_history_y);		//to R		BPF 1200-14000Hz
						sampleBuffer[5][j] = iir_second_order(sampleBufferTempR[j], HPF_COEF, lfe_history_x, lfe_history_y);	//to LFE	HPF 800Hz
					}
				}

				if (mode == MODE_1)
				{
					for (int j = 0; j < BLOCK_SIZE; j++)
					{
						sampleBufferTempL[j] = gain * sampleBuffer[0][j];
						sampleBufferTempR[j] = gain * sampleBuffer[1][j];

						sampleBuffer[0][j] = iir_second_order(sampleBufferTempL[j], LPF_COEF, c_history_x, c_history_y);		//to L		LPF 18kHz
						sampleBuffer[1][j] = iir_second_order(sampleBufferTempL[j], HPF_COEF, c_history_x, c_history_y);		//to C		HPF 800Hz
						sampleBuffer[2][j] = iir_second_order(sampleBufferTempL[j], BPF_COEF, ls_history_x, ls_history_y);	    //to Ls		BPF 1200-14000Hz
						sampleBuffer[3][j] = iir_second_order(sampleBufferTempR[j], BPF_COEF, rs_history_x, rs_history_y);	    //to Rs		BPF 1200-14000Hz
						sampleBuffer[4][j] = iir_second_order(sampleBufferTempR[j], HPF_COEF, r_history_x, r_history_y);		//to R		HPF 800Hz
						sampleBuffer[5][j] = iir_second_order(sampleBufferTempR[j], LPF_COEF, lfe_history_x, lfe_history_y);	//to LFE	LPF 18kHz
					}
				}
			}

			for (int j = 0; j < BLOCK_SIZE; j++)
			{
				for (int k = 0; k < outputWAVhdr.fmt.NumChannels; k++)
				{
					sample = sampleBuffer[k][j] * SAMPLE_SCALE;	// crude, non-rounding 			
					sample = sample >> (32 - inputWAVhdr.fmt.BitsPerSample);
					fwrite(&sample, outputWAVhdr.fmt.BitsPerSample / 8, 1, wav_out);
				}
			}
		}
	}
	
	// Close files
	//-------------------------------------------------	
	fclose(wav_in);
	fclose(wav_out);
	//-------------------------------------------------	

	return 0;
}
