#pragma once

#include <Portaudio/portaudio.h>

typedef struct
{
	float left_phase;
	float right_phase;
}
paTestData;

typedef int PaStreamCallback(const void* input,
	void* output,
	unsigned long frameCount,
	const PaStreamCallbackTimeInfo* timeInfo,
	PaStreamCallbackFlags statusFlags,
	void* userData);

class AudioRecorder
{
public:
	AudioRecorder() = default;
	~AudioRecorder()
	{
		close();
		terminate();
	}

	static int patestCallback(const void* inputBuffer, void* outputBuffer,
		unsigned long framesPerBuffer,
		const PaStreamCallbackTimeInfo* timeInfo,
		PaStreamCallbackFlags statusFlags,
		void* userData)
	{
		paTestData* data = (paTestData*)userData;
		const float* in = (const float*)inputBuffer;
		float* out = (float*)outputBuffer;

		for (int i = 0; i < framesPerBuffer; ++i)
		{
			std::cout << out[i] << std::endl;
		}

		return 0;
	}

	bool initialize()
	{
		if ((error = Pa_Initialize()) != paNoError)
			return false;

		num_devices = Pa_GetDeviceCount();
		if (num_devices < 0)
		{
			num_devices = error;
			return false;
		}

		memset(&inputParameters, 0, sizeof(inputParameters));
		memset(&outputParameters, 0, sizeof(outputParameters));

		

		inputParameters.device = Pa_GetDefaultInputDevice();
		inputParameters.channelCount = 2;
		inputParameters.sampleFormat = paFloat32;
		inputParameters.suggestedLatency = Pa_GetDeviceInfo(inputParameters.device)->defaultLowInputLatency;
		inputParameters.hostApiSpecificStreamInfo = NULL;

		std::cout << Pa_GetDeviceInfo(Pa_GetDefaultOutputDevice())->name << std::endl;

		outputParameters.device = Pa_GetDefaultOutputDevice();
		outputParameters.channelCount = 2;
		outputParameters.sampleFormat = paFloat32;
		outputParameters.suggestedLatency = Pa_GetDeviceInfo(outputParameters.device)->defaultLowInputLatency;
		outputParameters.hostApiSpecificStreamInfo = NULL;

		error = Pa_OpenStream(&stream,
			&inputParameters,
			&outputParameters,
			44100,
			56,
			paNoFlag,
			patestCallback,
			&data);

		if (error != paNoError)
			return false;

		return true;
	}

	bool start()
	{
		return (error = Pa_StartStream(stream)) == paNoError;
	}

	bool stop()
	{
		return (error = Pa_StopStream(stream)) == paNoError;
	}

	bool abort()
	{
		return (error = Pa_AbortStream(stream)) == paNoError;
	}

	bool close()
	{
		return (error = Pa_CloseStream(stream)) == paNoError;
	}

	bool terminate()
	{
		return (error = Pa_Terminate()) == paNoError;
	}

private:
	PaError error;
	PaStream* stream;
	paTestData data;

	PaStreamParameters inputParameters;
	PaStreamParameters outputParameters;

	int num_devices;
};
