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

	unsigned char currentLengthTimer : 6;
	unsigned short wavelength;
    int frequency;
    unsigned char channelActive : 1;
	unsigned char volume : 4;
	unsigned char ticksTillSweep : 3;

};

void OpenAudio (void);
void UpdateSoundChannels (int cycles);
void UpdatePulseChannel (int cycles, struct Pulse_Channel *Channel);
void TriggerChannel(int ChannelNum);