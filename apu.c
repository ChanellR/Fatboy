#define SDL_MAIN_HANDELD
#include <SDL2/SDL.h>
#include <math.h>

#include "header.h"
#include "memory.h"

#include <stdio.h> // printf

#define BUFFER_DURATION 1 // Length of the buffer in seconds
#define FREQUENCY 48000 // Samples per second
#define BUFFER_LEN (BUFFER_DURATION*FREQUENCY) // Samples in the buffer

void play_buffer(void*, unsigned char*, int);

// Global audio format variable
// If the sound card doesn't support this format, SDL handles the
// conversions seemlessly for us
SDL_AudioSpec spec = {
	.freq = FREQUENCY, 
	.format = AUDIO_S16SYS, // Signed 16 bit integer format
	.channels = 1,
	.samples = 4096, // The size of each "chunk"
	.callback = play_buffer, // user-defined function that provides the audio data
	.userdata = NULL // an argument to the callback function (we dont need any)
};

// Buffer that gets filled with the audio samples
Sint16 buffer[BUFFER_LEN];
// The position in buffer :
// We cannot provide the whole sound to the sound card at once.
// instead, it asks us repeatedly to provide the samples IN CHUNKS
// we store the last chunk (also called a block or frame) position so we know
// where to continue when we need to provide new samples
// It is also atomic because it can be accessed from different threads at the same time
int buffer_pos = 0;

// This maps our [0, 1] value to the sound card format we chose (signed 16 bit integer)
// Amplitude is the height of the wave, hence the volume
Sint16 format(double sample, double amplitude) {
	// 32567 is the maximum value of a 16 bit signed integer (2^15-1)
	return (Sint16)(sample*32567*amplitude);
}

// Generate a sine wave
double tone(double hz, unsigned long time) {
	return sin(time * hz * M_PI * 2 / FREQUENCY);
}

// Generate a sawtooth wave
double saw(double hz, unsigned long time) {
	return fmod(time*hz/FREQUENCY, 1)*2-1;
}

// Generate a square wave
double square(double hz, unsigned long time) {
	double sine = tone(hz, time);
	return sine > 0.0 ? 1.0 : -1.0;
}

int main(int argc, char** argv) {

	// Initialize the SDL audio system
	if (SDL_Init(SDL_INIT_AUDIO) < 0) {
		printf("SDL failed to initialize : %s\n", SDL_GetError());
		return -1;
	}

	// Arguments (in order) : 
	// default audio device, playback mode (not recording), the desired audio format, the obtained audio format (not needed),
	// If the sound card doesn't support our desired format, SDL handles the conversion for us
	SDL_AudioDeviceID dev = SDL_OpenAudioDevice(NULL, 0, &spec, NULL, 0);

	// Precompute the sound buffer so the audio doesn't hiccup
	// We could also compute these values on the fly (in the callback function), but that would
	// result in short pauses (hiccups)
	for (int i = 0; i < BUFFER_LEN; i++) {
		buffer[i] = format(tone(440, i), 0.5);
	}

	// Unpause the device, this starts playback (the callback function starts getting called)
	SDL_PauseAudioDevice(dev, 0);
	
	// Wait until the sound plays (SDL_Delay(1000 * BUFFER_DURATION) is also possible, but this is more precise)
	while (buffer_pos < BUFFER_LEN) {}

	SDL_CloseAudioDevice(dev);
	SDL_Quit();

	return 0;
}

// This is the function that gets automatically called every time the audio device needs more data
void play_buffer(void* userdata, unsigned char* stream, int len) {
	// Silence the whole stream in case we don't touch some parts of it
	// This fills the stream with the silence value (almost always just 0)
	// SDL implements the standard library (SDL_memset, SDL_memcpy etc.) to support more platforms
	SDL_memset(stream, spec.silence, len);
	
	// Dividing the stream size by 2 gives us the real length of the buffer (our format is 2 bytes per sample)
	len /= 2;
	// Prevent overflows (if we get to the end of the sound buffer, we don't want to read beyond it)
	len = (buffer_pos+len < BUFFER_LEN ? len : BUFFER_LEN-buffer_pos);

	// If we are at the end of the buffer, keep the silence and return
	if (len == 0) return;

	// Copy the samples from the current position in the buffer to the stream
	// Notice that the length gets multiplied back by 2 because we need to specify the length IN BYTES
	SDL_memcpy(stream, buffer+buffer_pos, len*2);

	// Move the buffer position
	buffer_pos += len;
}