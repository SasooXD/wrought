#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>
#include "portaudio.h"

#define SAMPLE_RATE 44100
#define FRAMES_PER_BUFFER 64
#define FREQ 440.0

typedef struct {
	float phase;
} paTestData;

// Silences stderr
void suppress_stderr(void) {
	int devnull = open("/dev/null", O_WRONLY);
	if (devnull != -1) {
		dup2(devnull, STDERR_FILENO);
		close(devnull);
	}
}

// Callback PortAudio
static int paCallback(const void *inputBuffer, void *outputBuffer,
		unsigned long framesPerBuffer,
		const PaStreamCallbackTimeInfo* timeInfo,
		PaStreamCallbackFlags statusFlags,
		void *userData) {
	paTestData *data = (paTestData*)userData;
	float *out = (float*)outputBuffer;
	unsigned long i;

	(void)inputBuffer;
	(void)timeInfo;
	(void)statusFlags;

	for (i = 0; i < framesPerBuffer; i++) {
		*out++ = (float)sin(2.0 * M_PI * FREQ * data->phase / SAMPLE_RATE);
		data->phase += 1.0;

		if (data->phase >= SAMPLE_RATE) data->phase -= SAMPLE_RATE;
	}

	return paContinue;
}

int main(void) {
	suppress_stderr();	// To shut up ALSA

	PaStream *stream;
	PaError err;
	paTestData data = {0};

	err = Pa_Initialize();
	if (err != paNoError) goto error;

	err = Pa_OpenDefaultStream(&stream,
		0, // no input
		1, // mono output
		paFloat32, // 32 bit floating point
		SAMPLE_RATE,
		FRAMES_PER_BUFFER,
		paCallback,
		&data);
	if (err != paNoError) goto error;

	err = Pa_StartStream(stream);
	if (err != paNoError) goto error;

	printf("...\n");
	Pa_Sleep(3000);

	err = Pa_StopStream(stream);
	if (err != paNoError) goto error;

	err = Pa_CloseStream(stream);
	if (err != paNoError) goto error;

	Pa_Terminate();
	return 0;

error:
	fprintf(stderr, "PortAudio error: %s\n", Pa_GetErrorText(err));
	Pa_Terminate();
	return 1;
}
