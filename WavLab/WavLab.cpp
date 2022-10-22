#include <iostream>
#include "WavFile.h"

int main()
{
    WavFile wav("C:\\Temp\\test_mono_24.wav");

    wav.printDetails();

    wav.loadSamplesFromFile();

    wav.save("C:\\Temp\\test_mono_24_out.wav");

    /*
    for (auto& sample : wav.getSamples())
    {
        std::cout << sample << std::endl;
    }
    */

    return 0;
}
