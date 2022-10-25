#include <iostream>
#include "WavFile.h"
#include "Generators.h"
#include "Tools.h"
#include "FX.h"

int main()
{
    //WavFile wav(1, 44100, 32);
    //Generators::sineWave(wav, 1000, 50);

    WavFile wav("C:\\Temp\\test_stereo_16.wav", true);
    FX::saturate(wav, 4);

    wav.save("C:\\Temp\\test_stereo_16_out.wav");

    return 0;
}
