#pragma once

struct Pulse_Channel
{
	union {
		unsigned char NRx0; //sweep control
		struct
		{
			unsigned char wavelengthSweepSlope : 3;
			unsigned char wavelengthSweepSubtraction : 1;
			unsigned char wavelengthSweepPace : 3;
		};
	};	

	union {
		unsigned char NRx1; //length timer and Duty Cycle
		struct
		{
			unsigned char initialLengthTimer : 6;
			unsigned char waveDuty : 2;
		};
	};	

	union {
		unsigned char NRx2; //volume and envelope
		struct
		{
			unsigned char volumeSweepPace : 3;
			unsigned char volumeEnvelopeInc : 1;
			unsigned char initialVolume : 4;
		};
	};	

	unsigned char NRx3;//wavelength low

	union {
		unsigned char NRx4; //wavelength high and contol
		struct
		{
			unsigned char wavelengthHigh : 3;
			unsigned char : 3;
			unsigned char lengthEnable : 1;
			unsigned char Trigger : 1;
		};
	};	

    unsigned char channelActive : 1;

	unsigned char currentLengthTimer : 6;
	unsigned char volumeTimer : 3;
	unsigned char sweepTimer : 4;
	unsigned char sweepEnabled : 1;

	unsigned short wavelength;
    int frequency;

	unsigned char volume : 4;

	

};

struct Wave_Channel 
{
	union {
		unsigned char NRx0; //DAC control
		struct
		{
			unsigned char : 7;
			unsigned char DAC : 1;
		};
	};	

	union {
		unsigned char NRx1; //length timer
		struct
		{
			unsigned char initialLengthTimer : 8;
		};
	};	

	union {
		unsigned char NRx2; //volume 
		struct
		{
			unsigned char : 5;
			unsigned char volume : 2;
			unsigned char : 1;
		};
	};	

	unsigned char NRx3;//wavelength low

	union {
		unsigned char NRx4; //wavelength high and contol
		struct
		{
			unsigned char wavelengthHigh : 3;
			unsigned char : 3;
			unsigned char lengthEnable : 1;
			unsigned char Trigger : 1;
		};
	};	

	unsigned char channelActive : 1;
	unsigned char currentLengthTimer : 6;
	unsigned short wavelength : 11;
	int frequency;


};

struct Noise_Channel 
{

	union {
		unsigned char NRx1; //length timer
		struct
		{
			unsigned char initialLengthTimer : 6;
		};
	};	

	union {
		unsigned char NRx2; //volume and envelope
		struct
		{
			unsigned char volumeSweepPace : 3;
			unsigned char volumeEnvelopeInc : 1;
			unsigned char initialVolume : 4;
		};
	};	

	union {
		unsigned char NRx3; //frequency and randomness
		struct
		{
			unsigned char clockDivider : 3;
			unsigned char LFSRWidth : 1;
			unsigned char clockShift : 4;
		};
	};	

	union {
		unsigned char NRx4; //wavelength high and contol
		struct
		{
			unsigned char : 6;
			unsigned char lengthEnable : 1;
			unsigned char Trigger : 1;
		};
	};	

	unsigned char channelActive : 1;

	unsigned char currentLengthTimer : 6;

	unsigned char volume : 4;
	unsigned char volumeTimer : 3;

	int frequency;
	unsigned short LFSR;


};



void OpenAudio (void);
void UpdateSoundChannels (int cycles);
void UpdatePulseChannel (struct Pulse_Channel *Channel);
void TriggerChannel(int ChannelNum);
void LoadSamples();
void UpdateSampleRate(double fps);
void EnagagePulseLength (int ChannelNum);
void ResetWaveDAC (void);