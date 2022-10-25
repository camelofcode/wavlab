#include <iostream>
#include "WavFile.h"
#include "Generators.h"

int main()
{
    WavFile wav(1, 44100, 32);
    Generators::sineWave(wav, 1000, 50);

    wav.save("C:\\Temp\\test_sine_wave_out.wav");

    return 0;
}
