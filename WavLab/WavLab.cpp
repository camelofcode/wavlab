#include <iostream>
#include "WavFile.h"

int main()
{
    WavFile wav("C:\\Temp\\test.wav");

    wav.printDetails();

    wav.loadSamplesFromFile();

    for (auto& sample : wav.getSamples())
    {
        std::cout << sample << std::endl;
    }

    return 0;
}
