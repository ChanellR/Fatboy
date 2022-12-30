#pragma once

struct Pulse_Channel
{
	union {
		unsigned char NRx0; //sweep control
		struct
		{
			int wavelengthSweepSlope : 3;
			int wavelengthSweepSubtraction : 1;
			int wavelengthSweepPace : 3;
		};
	};	

	union {
		unsigned char NRx1; //length timer and Duty Cycle
		struct
		{
			int initialLengthTimer : 6;
			int waveDuty : 2;
		};
	};	

	union {
		unsigned char NRx2; //volume and envelope
		struct
		{
			int volumeSweepPace : 3;
			int volumeEnvelopeInc : 1;
			int initialVolume : 4;
		};
	};	

	unsigned char NRx3;//wavelength low

	union {
		unsigned char NRx4; //wavelength high and contol
		struct
		{
			int wavelengthHigh : 3;
			int : 3;
			int lengthEnable : 1;
			int Trigger : 1;
		};
	};	

	int currentLengthTimer : 6;
	unsigned short wavelength;
    int frequency;
    unsigned char channelActive : 1;
    int volume : 4;

};


void UpdateSoundChannels (int cycles);
void UpdatePulse1 (int cycles);
void TriggerChannel(int Channel);
