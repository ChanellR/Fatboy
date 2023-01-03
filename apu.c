#include <SDL2/SDL.h>
#include <math.h>

#include <stdio.h> 

#include "header.h"
#include "memory.h"
#include "apu.h"
#include "src\include\SDL2\SDL_syswm.h"

#define FREQUENCY 32768
#define CHUNKSIZE 400

struct Pulse_Channel Pulse1;
struct Pulse_Channel Pulse2;
struct Wave_Channel Wave;
struct Noise_Channel Noise;


int frameCounter = 8196;
double sampleCounter = 128;
double LSFRCounter = 0;
unsigned char timerFrame = 0; 
unsigned char buffer[CHUNKSIZE];
int bufferPointer = 0;
int totalSamples = 0;

unsigned short LFSR = 0x0;

void Callback (void* userdata, unsigned char* stream, int len);

SDL_AudioSpec Spec = {
.freq = FREQUENCY, //512HZ minimum frequency 
.format = AUDIO_U8, 
.channels = 1,
.samples = CHUNKSIZE, // The size of each "chunk"
.callback = NULL, // user-defined function that provides the audio data
.userdata = NULL // an argument to the callback function (we dont need any)
};

SDL_AudioDeviceID ID; 

void UpdateSampleRate(double fps)
{
	//doesn't work with speedup unexpectedly after syncing with audio
	Spec.freq = (int)(round(FREQUENCY * fps / 59.73));
	printf("freq: %i", Spec.freq);
	SDL_CloseAudioDevice(ID);
	ID = SDL_OpenAudioDevice(NULL, 0, &Spec, NULL, 0);
	SDL_PauseAudioDevice(ID, 0);
}

void SetPulseWavelength (unsigned short newWavelength)
{
	Pulse1.NRx3 = Pulse1.wavelength & 0xFF;
	WriteByte(0xFF13, Pulse1.NRx3);

	Pulse1.wavelengthHigh = (Pulse1.wavelength & 0x0700) >> 8;
	WriteByte(0xFF14, Pulse1.NRx4 & 0x7F);
}

void ResetWaveDuty (void)
{
	Pulse1.NRx1 = ReadByte(0xFF11);
}

void EnagagePulseLength (int ChannelNum)
{
	switch(ChannelNum)
	{
		case 1:
			Pulse1.NRx4 = ReadByte(0xFF14);
			break;
		case 2:
			Pulse2.NRx4 = ReadByte(0xFF19);
			break;
		case 3:
			Wave.NRx4 = ReadByte(0xFF1E);
			break;
		case 4:
			Noise.NRx4 = ReadByte(0xFF23);
			break;
	}
}

void ResetPulse1Channel (void)
{
	Pulse1.NRx0 = ReadByte(0xFF10);
	Pulse1.NRx1 = ReadByte(0xFF11);
	Pulse1.NRx2 = ReadByte(0xFF12);
	Pulse1.NRx3 = ReadByte(0xFF13);
	Pulse1.NRx4 = ReadByte(0xFF14);
	Pulse1.currentLengthTimer = 64 - Pulse1.initialLengthTimer;
	Pulse1.volume = Pulse1.initialVolume;
	Pulse1.volumeTimer = Pulse1.volumeSweepPace;

	Pulse1.wavelength = (Pulse1.wavelengthHigh << 8) | Pulse1.NRx3;
	Pulse1.sweepTimer = (Pulse1.wavelengthSweepPace == 0) ? 8 : Pulse1.wavelengthSweepPace;
	Pulse1.sweepEnabled = (Pulse1.wavelengthSweepPace || Pulse1.wavelengthSweepSlope) ? 1 : 0;
}

void ResetPulse2Channel (void)
{
	Pulse2.NRx0 = 0x00;
	Pulse2.NRx1 = ReadByte(0xFF16);
	Pulse2.NRx2 = ReadByte(0xFF17);
	Pulse2.NRx3 = ReadByte(0xFF18);
	Pulse2.NRx4 = ReadByte(0xFF19);
	Pulse2.currentLengthTimer = 64 - Pulse2.initialLengthTimer;
	Pulse2.volume = Pulse2.initialVolume;
	Pulse2.wavelength = (Pulse2.wavelengthHigh << 8) | Pulse2.NRx3;
	Pulse2.volumeTimer = Pulse2.volumeSweepPace;
}

void ResetWaveChannel (void)
{
	Wave.NRx0 = ReadByte(0xFF1A);
	Wave.NRx1 = ReadByte(0xFF1B);
	Wave.NRx2 = ReadByte(0xFF1C);
	Wave.NRx3 = ReadByte(0xFF1D);
	Wave.NRx4 = ReadByte(0xFF1E);

	Wave.channelActive = (Wave.DAC) ? 1 : 0;
	Wave.currentLengthTimer = 256 - Wave.initialLengthTimer;

}

void SetLSFRCounter (void)
{
	Noise.NRx3 = ReadByte(0xFF22);
	// printf("%02X\n", Noise.NRx3);
	double Divider = (Noise.clockDivider == 0) ? .5f : Noise.clockDivider;
	// printf("clockdivider: %lf\n", Divider);
	LSFRCounter = (4194304 / ((262144 >> Noise.clockShift) / Divider));
}

void ResetNoiseChannel (void)
{
	Noise.NRx1 = ReadByte(0xFF20);
	Noise.NRx2 = ReadByte(0xFF21);
	// Noise.NRx3 = ReadByte(0xFF22);
	Noise.NRx4 = ReadByte(0xFF23);

	Noise.currentLengthTimer = 64 - Noise.initialLengthTimer;
	Noise.volume = Noise.initialVolume;
	Noise.volumeTimer = Noise.volumeSweepPace;
	LFSR = 0;
	SetLSFRCounter();

}

int GetWaveVolumeShift (void)
{
	Wave.NRx2 = ReadByte(0xFF1C);
	int output;
	switch(Wave.volume)
	{
		case 0:
			output = 8;
			break;
		case 1:
			output = 0;
			break;
		case 2:
			output = 1;
			break;
		case 3:
			output = 2;
			break;
	}
	return output;
}

void ResetWaveDAC (void)
{
	Wave.NRx0 = ReadByte(0xFF1A);
	Wave.channelActive = (Wave.DAC) ? 1 : 0;
}

void OpenAudio (void)
{

	ResetPulse1Channel();
	ResetPulse2Channel();
	ResetWaveChannel();
	ResetNoiseChannel();

	ID = SDL_OpenAudioDevice(NULL, 0, &Spec, NULL, 0);
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
	Pulse1.frequency = 131072/(2048 - Pulse1.wavelength);
	//default waveform 
	//22 cycles is how much time in a waveform

	return sin(Pulse1.frequency * totalSamples * 2 * M_PI / Spec.freq) > 0 ? 0xFF : 0x00;
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
	return sin(Pulse2.frequency * totalSamples * 2 * M_PI / Spec.freq) > 0 ? 0xFF : 0x00;
}

unsigned char GetWaveSample (void)
{
	unsigned short waveFormAddress = 0xFF30; 
	Wave.NRx3 = ReadByte(0xFF1D);
	Wave.NRx4 = ReadByte(0xFF1E);
	Wave.wavelength = (Wave.wavelengthHigh << 8) | Wave.NRx3;

	if(Wave.wavelength < 2000) Wave.frequency = 65536/(2048 - Wave.wavelength);
	
	double offset = (totalSamples / (Spec.freq / (16.0F * Wave.frequency))); /// (16 * Wave.frequency))) & 0xF;
	int roundedOffset = (int)round(offset) % 16;

	unsigned char output = 0;
	if(roundedOffset/2 % 2)
	{
		output = ReadByte(waveFormAddress + roundedOffset) & 0xF << 4;
	}
	else
	{
		output = ReadByte(waveFormAddress + roundedOffset) & 0xF0;
	}

	return output;
}

unsigned char GetNoiseSample (void)
{
	return (LFSR & 0x1) ? 0x0 : 0xFF;
}


void LoadSamples (void)
{
	unsigned char sample = 0x00;

	if(Pulse1.channelActive)
	{
		sample += (unsigned char)round(GetChannel1Sample() * (Pulse1.volume / 15.0f) / 4);
	}
	
	if(Pulse2.channelActive)
	{
		sample += (unsigned char)round(GetChannel2Sample() * (Pulse2.volume / 15.0f) / 4);
	}
		
	ResetWaveDAC();
	if(Wave.channelActive)
	{
		sample += (GetWaveSample() >> GetWaveVolumeShift()) / 4;
	}


	if(Noise.channelActive)
	{
		sample += (unsigned char)round(GetNoiseSample() * (Noise.volume / 15.0f) / 4);
	}

	sample = (unsigned char)round(sample * MasterVolume / 100.0f); 
	buffer[bufferPointer] = sample;

	bufferPointer++;
	totalSamples++;

	if(totalSamples == Spec.freq)
	{
		totalSamples = 0;
	}
	
	if(bufferPointer == CHUNKSIZE)
	{
		// FILE *ptr;
		// ptr = fopen("audiolog.file","ab"); 
		// fwrite(buffer, 400, 1, ptr);
		// fclose(ptr);

		while(SDL_GetQueuedAudioSize(ID) > 0) {SDL_Delay(1);}
		SDL_QueueAudio(ID, buffer, CHUNKSIZE);
		bufferPointer = 0;
	}
}

void UpdatePulseChannel (struct Pulse_Channel *Channel)
{	

	//2048 Machine Cycles per Frame sequencer = 1 tick
	//2	ticks per Lengthtimer
	//8 ticks per volume env
	//4 ticks per wavelength sweep

	if(timerFrame % 2 == 0)
	{
		if(Channel->lengthEnable) 
		{
			// printf("length enabled\n");
			Channel->currentLengthTimer--;
			if(Channel->currentLengthTimer == 0) {CloseAudio(Channel); return;}
		} 
	}

	if(timerFrame % 4 == 0 && Channel->sweepEnabled && (Channel == &Pulse1))
	{
		
		if(Channel->sweepTimer > 0)
		{
			Channel->sweepTimer -= 1;
		}

		if(Channel->sweepTimer == 0)
		{
			int newWavelength = 0;

			if(Channel->wavelengthSweepPace)
			{
				Channel->sweepTimer = Channel->wavelengthSweepPace;
			} 
			else 
			{
				Channel->sweepTimer = 8;				
			}

			if(Channel->wavelengthSweepPace && Channel->sweepEnabled)
			{
				int sign = (Channel->wavelengthSweepSubtraction) ? -1 : 1;
				newWavelength = Channel->wavelength + sign * (Channel->wavelength >> Channel->wavelengthSweepSlope);
				
				if(newWavelength <= 2047 && Channel->wavelengthSweepSlope) 
				{
					Channel->wavelength = newWavelength;
					SetPulseWavelength(newWavelength);
				} 
				else
				{
					CloseAudio(Channel); 
					return;
				}
			}
		}
	}	
	
	if(timerFrame % 8 == 0)
	{
		if(Channel->volumeSweepPace) //only refreshes on trigger
		{
			Channel->volumeTimer--;
			if(Channel->volumeTimer != 0) return;
			Channel->volumeTimer = Channel->volumeTimer;

			// if(Channel->volume == 0 && Channel->volumeEnvelopeInc == 0) {CloseAudio(Channel); return;}
			if(Channel->volume == 0x0F && Channel->volumeEnvelopeInc || (Channel->volume == 0x00 && !Channel->volumeEnvelopeInc)) return;
			Channel->volume += (Channel->volumeEnvelopeInc) ? 1 : -1;
			// printf("volume: %i\n", Channel->volume);
			//Disable DAC
		}			
	}

}

void UpdateWaveChannel (void)
{
	if(timerFrame % 2 == 0)
	{
		if(Wave.lengthEnable) 
		{
			Wave.currentLengthTimer--;
			if(Wave.currentLengthTimer == 0) {Wave.channelActive = 0; return;}
		} 
	}
}

void UpdateNoiseChannel (void)
{
	if(timerFrame % 2 == 0)
	{
		if(Noise.lengthEnable) 
		{
			Noise.currentLengthTimer--;
			if(Noise.currentLengthTimer == 0) {Noise.channelActive = 0; return;}
		} 
	}

	if(timerFrame % 8 == 0)
	{
		if(Noise.volumeSweepPace) //only refreshes on trigger
		{
			Noise.volumeTimer--;
			if(Noise.volumeTimer != 0) return;
			Noise.volumeTimer = Noise.volumeTimer;

			// if(Noise.volume == 0 && Noise.volumeEnvelopeInc == 0) {CloseAudio(Noise. return;}
			if(Noise.volume == 0x0F && Noise.volumeEnvelopeInc || (Noise.volume == 0x00 && !Noise.volumeEnvelopeInc)) return;
			Noise.volume += (Noise.volumeEnvelopeInc) ? 1 : -1;
			// printf("volume: %i\n", Channel->volume);
			//Disable DAC
		}			
	}
}

void TriggerChannel(int ChannelNum)
{

	switch(ChannelNum) {	
		case 1:
			ResetPulse1Channel();
			// printf("Pulse 1: %02X %02X %02X %02X %02X \n", Pulse1.NRx0, Pulse1.NRx1,
			// Pulse1.NRx2, Pulse1.NRx3, Pulse1.NRx4);
			Pulse1.channelActive = 1;
			break;
		case 2:
			ResetPulse2Channel();
			// printf("Pulse 2: %02X %02X %02X %02X %02X \n", Pulse2.NRx0, Pulse2.NRx1,
			// Pulse2.NRx2, Pulse2.NRx3, Pulse2.NRx4);
			Pulse2.channelActive = 1;
			break;
		case 3:
			ResetWaveChannel();
			Wave.channelActive = 1;
			break;
		case 4:
			ResetNoiseChannel();
			// printf("triggered Noise\n");
			Noise.channelActive = 1;
			break;
	}

}

void CycleLFSR (void)
{
    unsigned char xnorResult = 0;
    xnorResult = ~(LFSR & 0x01 ^ ((LFSR & 0x02) >> 1)) & 0x1;

	LFSR &= ~(0x1 << 15);
	LFSR |= (xnorResult << 15);

	if(Noise.LFSRWidth) 
	{
		LFSR &= ~(0x1 << 7);
		LFSR |= (xnorResult << 7);
	}

	LFSR >>= 1;
}

void UpdateSoundChannels (int cycles)
{	
	unsigned char MasterRegister = ReadByte(0xFF26);

	if(!(MasterRegister & 0x80)) return; //APU switch

	frameCounter -= cycles;
	if(frameCounter <= 0 ) //wont work with 0
	{
		frameCounter += (8196 * FREQUENCY / (double)Spec.freq);
		timerFrame++;

		if(Pulse1.channelActive) UpdatePulseChannel(&Pulse1);
		if(Pulse2.channelActive) UpdatePulseChannel(&Pulse2);
		if(Wave.channelActive) UpdateWaveChannel();
		if(Noise.channelActive) UpdateNoiseChannel();
	}

	LSFRCounter -= cycles;
	if(LSFRCounter <= 0)
	{
		SetLSFRCounter();
		CycleLFSR();
	}

	sampleCounter -= cycles;
	if(sampleCounter <= 0)
	{
		sampleCounter += (128.5f * FREQUENCY / Spec.freq);
		LoadSamples();
	}
}


