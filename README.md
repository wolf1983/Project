# Project
My project
    1. Task  description


 
	1. Based on the channel combination scheme shown in the figure and the control table for a given channel combination, implement the reference C code in floating point arithmetic.
	2. Processing is done in blocks. One block represents 16 signal selections. There are 2 channels at the entrance, and the output is up to 8 channels.
	3. Use the WinFilter application to calculate the filter coefficients.
	4. Enable forwarding of input and output file names as well as control values via command line parameters.
	5. Perform a test of the correct operation of the module for the given test vectors (in the TestStreams directory). Perform the test using spectral analysis of the output files. Perform the test for all possible values of the control variables.

	Figure 1.
	
    2. Description of realization

Upon request of the task, we enabled the command parameters, enable, gain and mode to be entered through the command line, but they are not mandatory parameters, and next to them as mandatory parameters of the command line are input wav file and output wav file. In case of omission of input and output wav file, the program shows an error, and in case of omission of any of the piece parameters, the program sets the default values.
After the program successfully receives the input parameters, the read wav header of the input file is read by the function ReadWavHeader (FILE *refFile, WAV_HEADER &inWAVhdr). Figure 2 shows the appearance of the wav header.

Figure 2.

	The next step is to set the parameters of the output WAV header and then enter the set parameters using the function WriteWavHeader (FILE *refFile, WAV_HEADER &inWAVhdr).
	Then the sound content is processed in blocks. Each block contains 16 signal selections, and the input WAV file contains two channels, which are further processed and form 6 output channels that are filtered depending on the given mode parameter. Each individual sample of the left or right channel is amplified by a given gain, passes through a given IIR filter and is forwarded to one of the 6 output channels.
	The IIR filter is implemented in the function iir_second_order(double input, double* coeficient, double* history_x, double* history_y). IIR filter coefficients were generated using the WinFilter tool.
    3. Testing and verification


The test is performed with the help of given test vectors and Audacity software that enables spectral analysis. In case the enable parameter is set to OFF the output file will contain 6 channels that are neither filtered nor amplified. Otherwise, the output channels will be filtered and amplified depending on the set gain and mode parameters.
Figures 3 and 4 represent the original input signal of the left and right channels, Figures 5 and 6 are the signals for the given gain 0 and after filtering with the HPF800Hz filter for the left and right channels, we have Figures 7 and 8, gain 0 and filter BPF1200-14000Hz, and Figure 9, gain 0 and fit LPF18kHz for left channel. Figure 10 is the left channel signal for gain -5 and the BPF1200-14000Hz filter.


Figure 3.

Figure 4.

Figure 5

Figure 6

Figure 7

Figure 8

Slika 9

Slika 10

By testing, we concluded that the program works correctly for the given parameters, and the implementation of the IIR filter is successful, because at the output we get the correct filtering depending on the filter, which can be seen in the pictures.
