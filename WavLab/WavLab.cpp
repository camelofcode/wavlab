#include <iostream>
#include "WavFile.h"
#include "Generators.h"
#include "Tools.h"

int main()
{
    //WavFile wav(1, 44100, 32);
    //Generators::sineWave(wav, 1000, 50);

    WavFile wav("C:\\Temp\\test_normalize_mono.wav", true);

    std::cout << Tools::GetPeakDecibels(wav) << std::endl;

    wav.save("C:\\Temp\\test_normalize_mono_out.wav");

    return 0;
}
