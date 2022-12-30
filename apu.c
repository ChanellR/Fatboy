#include <SDL2/SDL.h>
#include <math.h>
#include <stdio.h> // printf

#include "header.h"
#include "memory.h"
#include "apu.h"
#include "src\include\SDL2\SDL_syswm.h"

#define FREQUENCY 48000
#define CHUNKSIZE 4800

struct Pulse_Channel Pulse1;
int frameCounter = 2048;
unsigned char timerFrame = 0; 

void Pulse1_Callback (void* userdata, unsigned char* stream, int len);

SDL_AudioSpec Pulse1Spec = {
.freq = FREQUENCY, //512HZ minimum frequency 
.format = AUDIO_U8, // Signed 16 bit integer format
.channels = 1,
.samples = CHUNKSIZE, // The size of each "chunk"
.callback = Pulse1_Callback, // user-defined function that provides the audio data
.userdata = NULL // an argument to the callback function (we dont need any)
};

void ResetPulseWavelength (void)
{
	Pulse1.NRx3 = ReadByte(0xFF13);
	Pulse1.NRx4 = ReadByte(0xFF14);
	Pulse1.wavelength = (Pulse1.wavelengthHigh << 8) | Pulse1.NRx3;
	//amount of times it goes trough an 8 step waveform
}

void SetPulsefrequency (void)
{
	Pulse1.frequency = 131072/(2048 - Pulse1.wavelength);
}

void ResetWaveDuty (void)
{
	Pulse1.NRx1 = ReadByte(0xFF11);
}

void ResetPulseLengthTimer (void)
{
	Pulse1.NRx1 = ReadByte(0xFF11);
	Pulse1.NRx4 = ReadByte(0xFF14);
	Pulse1.currentLengthTimer = Pulse1.initialLengthTimer;
}

void ResetPulseVolume (void)
{
	Pulse1.NRx2 = ReadByte(0xFF12);	
	Pulse1.volume = Pulse1.initialVolume;
}

void ResetPulseChannel (void)
{
	Pulse1.NRx0 = ReadByte(0xFF10);
	Pulse1.NRx1 = ReadByte(0xFF11);
	Pulse1.NRx2 = ReadByte(0xFF12);
	Pulse1.NRx3 = ReadByte(0xFF13);
	Pulse1.NRx4 = ReadByte(0xFF14);
	Pulse1.currentLengthTimer = Pulse1.initialLengthTimer;
	Pulse1.volume = Pulse1.initialVolume;
	Pulse1.wavelength = (Pulse1.wavelengthHigh << 8) | Pulse1.NRx3;
}

void OpenAudio (void)
{
	ResetPulseChannel();

	SDL_OpenAudio(&Pulse1Spec, NULL);
	SDL_PauseAudio(0);
	Pulse1.channelActive = 1;

}

void CloseAudio (void)
{
	SDL_CloseAudio();
	Pulse1.channelActive = 0;
}

double tone(double hz, unsigned long time) 
{
	return sin(time * hz * M_PI * 2 / FREQUENCY);
}

double square(double hz, unsigned long time) {
	double sine = tone(hz, time);
	return sine > 0.0 ? 1.0 : -1.0;
}

void Pulse1_Callback (void* userdata, unsigned char* stream, int len) 
{
	// SDL_memset(stream, Pulse1Spec.silence, len);
	// return;
	SetPulsefrequency();
	ResetWaveDuty();

	unsigned char WaveDuty [4] = {0x7F, 0x7E, 
								0x78, 0x60};
	
	unsigned char buffer[CHUNKSIZE];
	//for a 64 Hz tone, we need to divide Chunksize by 6.4 and have 
	//complete wavforms of that length
	//CHUNKSIZE is 10HZ based
	for(int i = 0; i < CHUNKSIZE; i++)
	{  
		// buffer[i] = (WaveDuty[Pulse1.waveDuty] & (0x80 >> (i/(CHUNKSIZE / Pulse1.frequency / (8 * 10))) )) ? 0xFF : 0x00;
		buffer[i] = (WaveDuty[Pulse1.waveDuty] & (0x80 >> (i/(CHUNKSIZE / 8 / 25) ))) ? 0xFF : 0x00;

	}

	SDL_memcpy(stream, buffer, len);	// SDL_memcpy(stream, buffer+buffer_pos, len*2);
}

void UpdatePulse1 (int cycles)
{	

	//2048 Machine Cycles per Frame sequencer = 1 tick
	//2	ticks per Lengthtimer
	//8 ticks per volume env
	//4 ticks per wavelength sweep

	frameCounter -= cycles;
	if(frameCounter <= 0)
	{
		int overflow = frameCounter;
		frameCounter = 4096;
		frameCounter += overflow;

		timerFrame++;

		if(timerFrame % 2 == 0)
		{
			if(Pulse1.lengthEnable) 
			{
				Pulse1.currentLengthTimer--;
				if(!Pulse1.currentLengthTimer) {CloseAudio(); return;}
			} 
		}
		
		if(timerFrame % 4 == 0 && Pulse1.wavelengthSweepSlope)
		{
			
			if(Pulse1.wavelengthSweepPace)
			{
				if(Pulse1.wavelength == 0) {CloseAudio(); return;}
				int Sign = (Pulse1.wavelengthSweepSubtraction) ? -1 : 1;
				Pulse1.wavelength += Sign * (Pulse1.wavelength / pow(2, Pulse1.wavelengthSweepSlope));
				if(Pulse1.wavelength > 2047) {CloseAudio(); return;}
			}	
		}
		
		if(timerFrame % 8 == 0)
		{
			if(Pulse1.volumeSweepPace)
			{
				//not specing pace yet
				Pulse1.volume += (Pulse1.volumeEnvelopeInc) ? 1 : -1;
				if(!Pulse1.volume && !Pulse1.volumeEnvelopeInc) {CloseAudio(); return;}
			}			
		}

	}

}

void TriggerChannel(int Channel)
{
	//OpenAudio();
}

void UpdateSoundChannels (int cycles)
{	
	unsigned char MasterRegister = ReadByte(0xFF26);
	//4096 M-cycles per length increment

	if(!(MasterRegister & 0x80)) return; //APU switch
	if(Pulse1.channelActive) UpdatePulse1(cycles);
}
