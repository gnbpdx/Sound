#include "Wav.h"
#include <iostream>
#include <alsa/asoundlib.h>

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cout << "usage: "  << argv[0] << " wavfile.wav\n";
        return -1;
    }
    Wav original_wav = Wav(argv[1]);

    int channels_per_frame = 1;
    int sample_size = 16;
    int original_sample_rate = original_wav.sample_rate();
    int new_sample_rate = original_sample_rate / 2;
    int num_samples = original_wav.number_of_samples();
    int time_duration = num_samples / original_sample_rate;
    int bits_per_sample = original_wav.bits_per_sample();

    double coefficients[] = {
        0.0007402732643054117,
        -0.0007116178196449282,
        -0.0012364542225753877,
        0.0004878382037768346,
        0.001762355408397323,
        -4.42067370785638e-18,
        -0.002215182400043849,
        -0.0007717219779807554,
        0.002468118275890691,
        0.0017997462427444813,
        -0.002385864864076639,
        -0.0029997753194695915,
        0.0018453409331850108,
        0.004228422134073137,
        -0.0007592690890261643,
        -0.005289349745070121,
        -0.0009003571537489589,
        0.005948391131440519,
        0.0030822569848409777,
        -0.005956801424111659,
        -0.00564241768842927,
        0.005080467069416566,
        0.008339545358107981,
        -0.003131762701341885,
        -0.01084034550601907,
        1.3106206228765793e-17,
        0.012733594561465547,
        0.004323800421297494,
        -0.013549309312476557,
        -0.009731690323690397,
        0.012775497060241557,
        0.015996022350404576,
        -0.009858263926207278,
        -0.022780555925105705,
        0.004155841182759176,
        0.029665112243643956,
        0.005226581766628221,
        -0.036181637361030625,
        -0.019949557006721064,
        0.04185792449918842,
        0.044243745670944155,
        -0.04626408217804201,
        -0.09396783582341778,
        0.04905629457673092,
        0.3142499866912212,
        0.45011339147505103,
        0.3142499866912212,
        0.04905629457673092,
        -0.09396783582341778,
        -0.04626408217804201,
        0.044243745670944155,
        0.04185792449918842,
        -0.019949557006721064,
        -0.036181637361030625,
        0.005226581766628221,
        0.029665112243643956,
        0.004155841182759176,
        -0.022780555925105705,
        -0.009858263926207278,
        0.015996022350404576,
        0.012775497060241557,
        -0.009731690323690397,
        -0.013549309312476557,
        0.004323800421297494,
        0.012733594561465547,
        1.3106206228765793e-17,
        -0.01084034550601907,
        -0.003131762701341885,
        0.008339545358107981,
        0.005080467069416566,
        -0.00564241768842927,
        -0.005956801424111659,
        0.0030822569848409777,
        0.005948391131440519,
        -0.0009003571537489589,
        -0.005289349745070121,
        -0.0007592690890261643,
        0.004228422134073137,
        0.0018453409331850108,
        -0.0029997753194695915,
        -0.002385864864076639,
        0.0017997462427444813,
        0.002468118275890691,
        -0.0007717219779807554,
        -0.002215182400043849,
        -4.42067370785638e-18,
        0.001762355408397323,
        0.0004878382037768346,
        -0.0012364542225753877,
        -0.0007116178196449282,
        0.0007402732643054117
    };

    int new_num_samples = num_samples / 2;
    int16_t* sampled_data = new int16_t[new_num_samples];
    //Apply Low-Pass Filter
    int num_coefficients = 91;
    for (int i = 0; i < new_num_samples; ++i) {
        double val = 0;
        for (int j = 0; j < num_coefficients; ++j) {
            if ((i - j) >= 0)
                val += coefficients[j] * ((double) original_wav.data[2*i-j]);
            else
                break;
        }
        sampled_data[i] = val;
    }

    Wav sampled_wav = Wav(channels_per_frame, new_sample_rate, bits_per_sample, new_num_samples, sampled_data);
    std::string filename = argv[1];
    filename = "r" + filename;
    sampled_wav.write_to_file(filename.c_str());


        const char* playback_device = "default";
    if (argc == 3 && strcmp(argv[2], "play") == 0) {
        snd_pcm_t* pcm_handle;
        snd_pcm_open(&pcm_handle, playback_device, SND_PCM_STREAM_PLAYBACK, 0);

        //If no sound is played the latency may not be correctly configured
        snd_pcm_set_params(pcm_handle, SND_PCM_FORMAT_S16_LE, SND_PCM_ACCESS_RW_INTERLEAVED, channels_per_frame, new_sample_rate, 0, 500000);

        snd_pcm_writei(pcm_handle, sampled_data, new_sample_rate * time_duration);
        snd_pcm_start(pcm_handle);
        snd_pcm_drain(pcm_handle);
        snd_pcm_close(pcm_handle);
    }
    delete [] sampled_data;
    return 0;
}