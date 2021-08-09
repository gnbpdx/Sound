#include "Wav.h"
#include <math.h>
#include <alsa/asoundlib.h>
int main()
{
    int channels_per_frame = 1;
    int sample_size = 16;
    int amplitude = 8192;
    int time_duration = 1;
    int frequency = 440;
    int sample_rate = 48000;

    int16_t* data = new int16_t[sample_rate * time_duration];
    for (int i = 0; i < sample_rate * time_duration; ++i) {
        float t = i * time_duration / ((float) sample_rate);
        data[i] = (int16_t) amplitude * sin(2 * M_PI * frequency * t);
    }

    Wav sine_wav(channels_per_frame, sample_rate, sample_size, sample_rate * time_duration, data);
    sine_wav.write_to_file("sine.wav");

    amplitude = 16384;

    for (int i = 0; i < sample_rate * time_duration; ++i) {
        float t = i * time_duration / ((float) sample_rate);
        data[i] = (int16_t) amplitude * sin(2 * M_PI * frequency * t);
        if (data[i] > 0.5 * amplitude)
            data[i] = (int16_t) (0.5 * amplitude);
        if (data[i] < -0.5 * amplitude)
            data[i] = (int16_t) (-0.5 * amplitude);
    }

    Wav clipped_wav(channels_per_frame, sample_rate, sample_size, sample_rate * time_duration, data);
    clipped_wav.write_to_file("clipped.wav");

    //Based off of ALSA docs: https://www.alsa-project.org/alsa-doc/alsa-lib/_2test_2pcm_min_8c-example.html
    const char* playback_device = "default";
    snd_pcm_t* pcm_handle;
    snd_pcm_open(&pcm_handle, playback_device, SND_PCM_STREAM_PLAYBACK, 0);

    //If no sound is played the latency may not be correctly configured
    snd_pcm_set_params(pcm_handle, SND_PCM_FORMAT_S16_LE, SND_PCM_ACCESS_RW_INTERLEAVED, channels_per_frame, sample_rate, 0, 500000);

    snd_pcm_writei(pcm_handle, data, sample_rate * time_duration);
    snd_pcm_start(pcm_handle);
    snd_pcm_drain(pcm_handle);
    snd_pcm_close(pcm_handle);

    delete [] data;
}
