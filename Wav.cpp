#include "Wav.h"
#include <string.h>
#include <fstream>
Wav::Wav(uint16_t num_channels, uint32_t sample_rate, uint16_t bits_per_sample, uint32_t num_samples, int16_t* data) 
    : nChannels(num_channels), nSamplesPerSec(sample_rate), wBitsPerSample(bits_per_sample), ckID("RIFF"), 
    fmt_ckID("fmt "), data_ckID("data"), WAVEID("WAVE"), fmt_cksize(16), wFormatTag(1), num_samples(num_samples)
{
    nAvgBytesPerSec = sample_rate * num_channels * bits_per_sample / 8;
    nBlockAlign = num_channels * bits_per_sample / 8;
    data_cksize = num_samples * num_channels * bits_per_sample / 8;
    cksize = 36 + data_cksize;
    this->data = new int16_t[num_samples * num_channels];
    memcpy(this->data, data, num_samples * bits_per_sample / 8);
}
Wav::Wav(const char* filename) {
    std::fstream in;
    in.open(filename, std::ios::in | std::ios::binary);
    in.read(ckID, 4);
    in.read((char*) &cksize, 4);
    in.read(WAVEID, 4);
    in.read(fmt_ckID, 4);
    in.read((char*) &fmt_cksize, 4);
    in.read((char*) &wFormatTag, 2);
    in.read((char*) &nChannels, 2);
    in.read((char*) &nSamplesPerSec, 4);
    in.read((char*) &nAvgBytesPerSec, 4);
    in.read((char*) &nBlockAlign, 2);
    in.read((char*) &wBitsPerSample, 2);
    in.read(data_ckID, 4);
    in.read((char*) &data_cksize, 4);
    num_samples = 8 * data_cksize / (nChannels * wBitsPerSample);
    data = new int16_t[num_samples];
    in.read((char*) data, 2 * num_samples);
    in.close();
}
Wav::~Wav()
{
    delete [] data;
}
void Wav::write_to_file(const char* filename)
{
    std::fstream out;
    out.open(filename, std::ios::out | std::ios::binary);
    //This isn't going to work on big-endian systems
    out.write(ckID, 4);
    out.write((char*) &cksize, 4);
    out.write(WAVEID, 4);
    out.write(fmt_ckID, 4);
    out.write((char*) &fmt_cksize, 4);
    out.write((char*) &wFormatTag, 2);
    out.write((char*) &nChannels, 2);
    out.write((char*) &nSamplesPerSec, 4);
    out.write((char*) &nAvgBytesPerSec, 4);
    out.write((char*) &nBlockAlign, 2);
    out.write((char*) &wBitsPerSample, 2);
    out.write(data_ckID, 4);
    out.write((char*) &data_cksize, 4);
    out.write((char*) data, 2 * num_samples);

    out.close();

}
int Wav::sample_rate() {
    return nSamplesPerSec;
}

int Wav::number_of_samples() {
    return num_samples;
}
int Wav::bits_per_sample() {
    return wBitsPerSample;
}