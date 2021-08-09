import matplotlib.pyplot as plt
import librosa
import librosa.display
import scipy
import numpy as np
import os
import cv2
class Spectogram():
    #Window length is the size of the window
    #Window padding is number of zeros to pad audio after window is applied
    #Hop length is the number of audio samples between each STFT output
    def __init__(self, window_length= 2048, window_padding=0, hop_length=None, window=scipy.signal.windows.hann):
        self.window_length = window_length
        self.window_padding = window_padding
        self.window = window
        if hop_length != None:
            self.hop_length = hop_length
        else:
            self.hop_length = window_length // 4

    #Code based off examples from https://librosa.org/doc/latest/index.html
    def signal_to_spectogram(self, wav_file):
        signal, sampling_rate = librosa.load(wav_file)
        spectogram_matrix = librosa.stft(signal, n_fft = self.window_length + self.window_padding, hop_length=self.hop_length)
        return spectogram_matrix

    def signal_to_spectogram_file(self, wav_file, compressed_file, dims):
        spec = self.signal_to_spectogram(wav_file)
        spectogram_matrix  =cv2.resize(np.abs(spec), dims, interpolation=cv2.INTER_AREA)
        np.savetxt(compressed_file, spectogram_matrix, fmt = '%.2f')

    @staticmethod
    def float_to_string(real_num):
        if abs(real_num) < 0.01:
            return "0"
        return "{:.2f}".format(real_num)

    def npy_to_spectogram_file(self, spectogram_file, compressed_file, dims):
        spec = Spectogram.load_spectogram_from_npy_file(spectogram_file)
        spectogram_matrix  =cv2.resize(spec, dims, interpolation=cv2.INTER_AREA)
        np.savetxt(compressed_file, spectogram_matrix, fmt = '%.2f')

    @staticmethod
    def compress_spectogram(spectogram, dims):
        return cv2.resize(spectogram, dims, interpolation=cv2.INTER_AREA)

    @staticmethod
    def load_spectogram_from_file(spectogram_file):
        return np.loadtxt(spectogram_file)

    @staticmethod
    def load_spectogram_from_npy_file(npy_file):
        return np.load(npy_file)

    @staticmethod
    def real_spectogram_to_db_spectogram(spectogram):
        return librosa.amplitude_to_db(spectogram, ref=np.max)

    @staticmethod
    def display_complex_spectogram(spectogram):
        real_spectogram_db = librosa.amplitude_to_db(np.abs(spectogram), ref=np.max)
        plt.figure()
        librosa.display.specshow(real_spectogram_db)
        plt.colorbar()
        plt.show()

    @staticmethod
    def display_real_spectogram(spectogram):
        real_spectogram_db = Spectogram.real_spectogram_to_db_spectogram(spectogram)
        plt.figure()
        librosa.display.specshow(real_spectogram_db)
        plt.colorbar()
        plt.show()

def npy_file_test(test_file):
    spectogram = Spectogram()
    spec=Spectogram.load_spectogram_from_npy_file(test_file)
    Spectogram.display_real_spectogram(spec)
    spectogram.npy_to_spectogram_file(test_file, 'spec', (500,150))
    spec=Spectogram.load_spectogram_from_file('spec')
    print(spec.shape[0], spec.shape[1])
    Spectogram.display_real_spectogram(spec)

def wav_file_test(wav_file):
    spectogram = Spectogram()
    spec=spectogram.signal_to_spectogram(wav_file)
    print(spec.shape[0], spec.shape[1])
    Spectogram.display_complex_spectogram(spec)
    spectogram.signal_to_spectogram_file(wav_file, 'spec', (150,150))
    spec=Spectogram.load_spectogram_from_file('spec')
    print(spec.shape[0], spec.shape[1])
    Spectogram.display_real_spectogram(spec)



def convert_signals_to_spectograms():
    spectogram = Spectogram()
    for data_dir in ['test_data/', 'training_data/', 'validation_data/']:
        print('Converting' + ' ' + data_dir)
        for label_dir in os.listdir(data_dir):
            os.mkdir(data_dir + 'spec' + label_dir)
            path = data_dir + label_dir + '/'
            for wav_file in os.listdir(path):
                print('Converting ' + wav_file)
                spec = wav_file + '.spec'
                spec_path = data_dir + 'spec' + label_dir + '/'
                spectogram.signal_to_spectogram_file(path + wav_file, spec_path + spec)
    print('Done Converting!')
        
if __name__ == '__main__':
    wav_file_test('test_data/1/Medley-solos-DB_test-1_81fb3f6e-980f-5bfe-f1c0-b9f1a7cca824.wav')
    npy_file_test('/media/grant/Home/mtg-jamendo-dataset/mtg-dataset/01/1101.npy')
