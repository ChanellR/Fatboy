#include <SDL2/SDL.h>
#include <math.h>
#include <stdio.h> 

#include "header.h"
#include "memory.h"
#include "apu.h"
#include "src\include\SDL2\SDL_syswm.h"

#define FREQUENCY 48000
#define CHUNKSIZE 512

struct Pulse_Channel Pulse1;
struct Pulse_Channel Pulse2;

int frameCounter = 2048;
int sampleCounter = 22;
unsigned char timerFrame = 0; 
unsigned char buffer[CHUNKSIZE];
int bufferPointer = 0;
int totalSamples = 0;

SDL_AudioSpec Pulse1Spec = {
.freq = FREQUENCY, //512HZ minimum frequency 
.format = AUDIO_U8, 
.channels = 1,
.samples = CHUNKSIZE, // The size of each "chunk"
.callback = NULL, // user-defined function that provides the audio data
.userdata = NULL // an argument to the callback function (we dont need any)
};

SDL_AudioDeviceID ID; 

void ResetPulseWavelength (void)
{
	Pulse1.NRx3 = ReadByte(0xFF13);
	Pulse1.NRx4 = ReadByte(0xFF14);
	Pulse1.wavelength = (Pulse1.wavelengthHigh << 8) | Pulse1.NRx3;
	//amount of times it goes trough an 8 step waveform
}

void SetPulseWavelength (unsigned short newWavelength)
{
	Pulse1.NRx3 = Pulse1.wavelength & 0xFF;
	Pulse1.wavelengthHigh = (Pulse1.wavelength & 0x0700) >> 8;
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

void ResetPulse1Channel (void)
{
	Pulse1.NRx0 = ReadByte(0xFF10);
	Pulse1.NRx1 = ReadByte(0xFF11);
	Pulse1.NRx2 = ReadByte(0xFF12);
	Pulse1.NRx3 = ReadByte(0xFF13);
	Pulse1.NRx4 = ReadByte(0xFF14);
	Pulse1.currentLengthTimer = Pulse1.initialLengthTimer;
	Pulse1.volume = Pulse1.initialVolume;
	Pulse1.wavelength = (Pulse1.wavelengthHigh << 8) | Pulse1.NRx3;
	Pulse1.ticksTillSweep = Pulse1.wavelengthSweepPace;
}

void ResetPulse2Channel (void)
{
	Pulse2.NRx0 = 0x00;
	Pulse2.NRx1 = ReadByte(0xFF16);
	Pulse2.NRx2 = ReadByte(0xFF17);
	Pulse2.NRx3 = ReadByte(0xFF18);
	Pulse2.NRx4 = ReadByte(0xFF19);
	Pulse2.currentLengthTimer = Pulse2.initialLengthTimer;
	Pulse2.volume = Pulse2.initialVolume;
	Pulse2.wavelength = (Pulse2.wavelengthHigh << 8) | Pulse2.NRx3;
	
}

void OpenAudio (void)
{

	ResetPulse1Channel();
	ResetPulse2Channel();
	ID = SDL_OpenAudioDevice(NULL, 0, &Pulse1Spec, NULL, 0);
	SDL_PauseAudioDevice(ID, 0);

}

void CloseAudio (struct Pulse_Channel *Channel)
{
	//SDL_CloseAudio();
	// ResetPulseChannel();
	Channel->channelActive = 0;
}

unsigned char GetChannel1Sample (void)
{
	Pulse1.NRx3 = ReadByte(0xFF13);
	Pulse1.NRx4 = ReadByte(0xFF14);
	Pulse1.wavelength = (Pulse1.wavelengthHigh << 8) | Pulse1.NRx3;
	SetPulsefrequency();
	//default waveform 
	//22 cycles is how much time in a waveform

	return sin(Pulse1.frequency * totalSamples * 2 * M_PI / FREQUENCY) > 0 ? 0xFF : 0x00;
}

unsigned char GetChannel2Sample (void)
{
	Pulse2.NRx3 = ReadByte(0xFF18);
	Pulse2.NRx4 = ReadByte(0xFF19);
	Pulse2.wavelength = (Pulse2.wavelengthHigh << 8) | Pulse2.NRx3;
	Pulse2.frequency = 131072/(2048 - Pulse2.wavelength);
	//default waveform 
	//22 cycles is how much time in a waveform
	// printf("frequency: %i", Pulse2.frequency);
	return sin(Pulse2.frequency * totalSamples * 2 * M_PI / FREQUENCY) > 0 ? 0xFF : 0x00;
}

void LoadSamples (void)
{
	unsigned char sample = 0;

	if(Pulse1.channelActive)
	{
		sample += GetChannel1Sample() * (Pulse1.volume / 15.0f) / 2;
	}
	
	if(Pulse2.channelActive)
	{
		sample += GetChannel2Sample() * (Pulse2.volume / 15.0f) / 2;
	}

	buffer[bufferPointer] = sample;

	bufferPointer++;
	totalSamples++;
	
	if(bufferPointer == 512)
	{
		SDL_QueueAudio(ID, buffer, 512);
		bufferPointer = 0;
	}

	if(totalSamples == 48000)
	{
		totalSamples = 0;
	}
}

void UpdatePulseChannel (int cycles, struct Pulse_Channel *Channel)
{	

	//2048 Machine Cycles per Frame sequencer = 1 tick
	//2	ticks per Lengthtimer
	//8 ticks per volume env
	//4 ticks per wavelength sweep

	frameCounter -= cycles;
	if(frameCounter <= 0 ) //wont work with 0
	{
		int overflow = frameCounter;
		frameCounter = 2048;
		frameCounter += overflow;

		timerFrame++;

		if(timerFrame % 2 == 0)
		{

			if(Channel->lengthEnable) 
			{
				// printf("length enabled\n");
				Channel->currentLengthTimer--;
				if(Channel->currentLengthTimer == 0) {CloseAudio(Channel); return;}
			} 
		}
		
		ResetPulseWavelength();

		if(timerFrame % 4 == 0 && Channel->wavelengthSweepSlope && (Channel == &Pulse1))
		{

			if(Channel->wavelengthSweepPace != 0)
			{		
				Pulse1.ticksTillSweep--; //perhaps
				if(Pulse1.ticksTillSweep == 0)
				{
					Pulse1.ticksTillSweep = Pulse1.wavelengthSweepPace;

					int Sign = (Channel->wavelengthSweepSubtraction) ? -1 : 1;
					Channel->wavelength += Sign * (Channel->wavelength / pow(2, Channel->wavelengthSweepSlope));
					
					// printf("Pulse1: %04X\n", Pulse1.wavelength);
					SetPulseWavelength(Channel->wavelength);

					if(Channel->wavelength > 2047) 
					{
						CloseAudio(Channel); 
						return;
					}
				}		
			}	
		}
		
		if(timerFrame % 8 == 0)
		{
			if(Channel->volumeSweepPace)
			{
				//not specing pace yet
				Channel->volume += (Channel->volumeEnvelopeInc) ? 1 : -1;
				// printf("volume: %i\n", Channel->volume);
				if(Channel->volume == 0 && Channel->volumeEnvelopeInc == 0) {CloseAudio(Channel); return;}
				//Disable DAC
			}			
		}

	}

}

void TriggerChannel(int ChannelNum)
{
	//printf("Trigger Channel %i\n", ChannelNum);

	switch(ChannelNum) {	
		case 1:
			ResetPulse1Channel();
			// printf("Pulse 1: %02X %02X %02X %02X %02X \n", Pulse1.NRx0, Pulse1.NRx1,
			// Pulse1.NRx2, Pulse1.NRx3, Pulse1.NRx4);
			// printf("Pulse 1: %02X %02X %02X %02X %02X \n", ReadByte(0xFF10), ReadByte(0xFF11), 
			// ReadByte(0xFF12), ReadByte(0xFF13), ReadByte(0xFF14));
			Pulse1.channelActive = 1;
			break;
		case 2:
			ResetPulse2Channel();
			// printf("Pulse 2: %02X %02X %02X %02X %02X \n", Pulse2.NRx0, Pulse2.NRx1,
			// Pulse2.NRx2, Pulse2.NRx3, Pulse2.NRx4);
			Pulse2.channelActive = 1;
			break;
	}

}

void UpdateSoundChannels (int cycles)
{	
	unsigned char MasterRegister = ReadByte(0xFF26);
	//4096 M-cycles per length increment

	if(!(MasterRegister & 0x80)) return; //APU switch

	if(Pulse1.channelActive) UpdatePulseChannel(cycles, &Pulse1);
	if(Pulse2.channelActive) UpdatePulseChannel(cycles, &Pulse2);

	sampleCounter -= cycles;
	if(sampleCounter <= 0)
	{
		int overflow = -1*sampleCounter;
		sampleCounter = 22;
		sampleCounter -= overflow;

		LoadSamples();
	}


}
