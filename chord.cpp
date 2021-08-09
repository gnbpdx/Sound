#include <iostream>
#include <unordered_map>
#include <string>
#include <cstring>
#include <cmath>
#include "Wav.h"
#include <alsa/asoundlib.h>

int main (int argc, char** argv) {
    if (argc < 3) {
        std::cout << "usage: "  << argv[0] << "chord" << "just/equal" << std::endl;
        return -1;
    }
    std::unordered_map<std::string, int> to_midi = {
        {"C", 72},
        {"C#", 73},
        {"Db", 73},
        {"D", 74},
        {"D#", 75},
        {"Eb", 75},
        {"E", 76},
        {"Fb", 76},
        {"E#", 77},
        {"F", 77},
        {"F#", 78},
        {"Gb", 78},
        {"G", 79},
        {"G#", 80},
        {"Ab", 80},
        {"A", 81},
        {"A#", 82},
        {"Bb", 82},
        {"B", 83},
        {"Cb", 83},
        {"B#", 72} 
    };
    std::unordered_map<int,double> just_step = {
        {0, 1.0},
        {1, 16.0/15.0},
        {2, 9.0/8.0},
        {3, 6.0/5.0},
        {4, 5.0/4.0},
        {5, 4.0/3.0},
        {6, 45.0/32.0},
        {7, 3.0/2.0},
        {8, 8.0/5.0},
        {9, 5.0/3.0},
        {10, 9.0/5.0},
        {11, 15.0/8.0}
    };
    char* chord = argv[1];
    std::string chord_name(chord);
    bool major = true;
    if (chord[strlen(chord) - 1] == 'm') {
        major = false;
        chord[strlen(chord) - 1] = '\0';
    }
    double C = 440 * pow(2,3.0/12.0);
    double frequencies[3];
    std::unordered_map<std::string, int>::iterator it = to_midi.find(chord);
    if (it == to_midi.end()) {
        std::cout << "Invalid Chord Name" << std::endl;
    }
    
    if (!strcmp(argv[2], "just")) {
        frequencies[0] = C * just_step.find((it->second - 72) % 12)->second;
        frequencies[1] = C * just_step.find(((it->second - 72) + (major ? 4 : 3)) % 12)->second;
        frequencies[2] = C * just_step.find((it->second - 72 + 7) % 12)->second;
    }
    else if (!strcmp(argv[2], "equal")) {
        frequencies[0] = C * pow(2.0, ((it->second - 72) % 12)/12.0);
        frequencies[1] = C * pow(2.0, (((it->second - 72) + (major ? 4 : 3)) % 12)/12.0);
        frequencies[2] = C * pow(2.0, ((it->second + 7 - 72) % 12) /12.0);
    }
    else {
        std::cout << "Temperament must be just or equal";
        return -1;
    }

    const double amplitude = 32767.0 / 6.0;
    int channels_per_frame = 1;
    int sample_size = 16;
    int time_duration = 1;
    int sample_rate = 48000;

    int16_t* data = new int16_t[sample_rate * time_duration];
    for (int i = 0; i < sample_rate * time_duration; ++i) {
        float t = i * time_duration / ((float) sample_rate);
        data[i] =  amplitude * sin(2 * M_PI * frequencies[0] * t)
        +  amplitude * sin(2 * M_PI * frequencies[1] * t)
        + amplitude * sin(2 * M_PI * frequencies[2] * t);
    }

    Wav sine_wav(channels_per_frame, sample_rate, sample_size, sample_rate * time_duration, data);
    sine_wav.write_to_file((chord_name + "-" + argv[2] + ".wav").c_str());

    if (!strcmp(argv[argc-1], "play")) {
        const char* playback_device = "default";
        snd_pcm_t* pcm_handle;
        snd_pcm_open(&pcm_handle, playback_device, SND_PCM_STREAM_PLAYBACK, 0);

        //If no sound is played the latency may not be correctly configured
        snd_pcm_set_params(pcm_handle, SND_PCM_FORMAT_S16_LE, SND_PCM_ACCESS_RW_INTERLEAVED, channels_per_frame, sample_rate, 0, 500000);

        snd_pcm_writei(pcm_handle, data, sample_rate * time_duration);
        snd_pcm_start(pcm_handle);
        snd_pcm_drain(pcm_handle);
        snd_pcm_close(pcm_handle);
    }

    delete [] data;

}
