#include <stdint.h>
#include <iostream>
//Specifications and naming come from http://www-mmsp.ece.mcgill.ca/Documents/AudioFormats/WAVE/WAVE.html
//Another good reference: http://soundfile.sapp.org/doc/WaveFormat/
//Only works for PCM data
struct Wav {
    private:
        // Wav Header
        char ckID[5]; //"RIFF"
        uint32_t cksize; //Size of rest of WAV file
        char WAVEID[5]; //"WAVE"
        char fmt_ckID[5]; //"fmt "
        uint32_t fmt_cksize; //Size of subchunk header (Value is 16 for PCM)
        uint16_t wFormatTag; //Wave format (Value is 1 for PCM)
        uint16_t nChannels;  //Number of channels used
        uint32_t nSamplesPerSec; //Number of times per second audio is sampled
        uint32_t nAvgBytesPerSec; //Number of bytes played per second
        uint16_t nBlockAlign; //Number of bytes for a single sample across all channels
        uint16_t wBitsPerSample; //We use 16 bit samples
        char data_ckID[5]; // "data"
        uint32_t data_cksize; //Number of bytes in data

        uint num_samples; //Not part of WAV file
    public:
        Wav(const char* filename);
        Wav(uint16_t num_channels, uint32_t sample_rate, uint16_t bits_per_sample, uint32_t num_samples, int16_t* data);
        ~Wav();
        int sample_rate();
        int number_of_samples();
        int bits_per_sample();
        void write_to_file(const char* filename);
        int16_t* data; //This is the data for the audio

};