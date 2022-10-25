#include "Tools.h"
#include <algorithm>

void Tools::normalise(WavFile &wave)
{
	double highValue = 0;
 
    //Find loudest sample
    for (auto& sample : wave.getSamples())
    {
        if (fabs(sample) > fabs(highValue)) 
        {
            highValue = sample;
        }
    }

    //Calculate gain
    double peak = fabs(highValue);
    double gain = 1 / peak;

    std::vector<double> newSamples;

    //Write new samples with gain applied
    for (auto& sample : wave.getSamples())
    {
        newSamples.push_back(sample * gain);
    }

    wave.setSamples(newSamples);
}

void Tools::reverse(WavFile& wave)
{
    if (wave.getNumberOfChannels() == 1)
    {
        std::vector<double> original = wave.getSamples();
        std::reverse(original.begin(), original.end());
        wave.setSamples(original);
    }
    else if (wave.getNumberOfChannels() == 2)
    {
        std::vector<double> original = wave.getSamples();
        std::vector<double> reversed;
        for (size_t i = original.size() - 1; i > 1; i = i - 2)
        {
            double right = original[i];
            double left = original[i - 1];

            reversed.push_back(left);
            reversed.push_back(right);
        }
        wave.setSamples(reversed);
    }
}

void Tools::flipPhase(WavFile& wave)
{
    std::vector<double> newSamples;

    for (auto& sample : wave.getSamples())
    {
        newSamples.push_back(-sample);
    }
    wave.setSamples(newSamples);
}

void Tools::swapChannels(WavFile& wave)
{
    if (wave.getNumberOfChannels() == 1)
    {
        //Can't do anything to a mono file, but why error, no harm done!
        return;
    }
    else if (wave.getNumberOfChannels() == 2)
    {
        std::vector<double> original = wave.getSamples();
        std::vector<double> swapped;
        for (size_t i = 0; i < original.size() - 1; i = i + 2)
        {
            double left = original[i];
            double right = original[i + 1];

            swapped.push_back(right);
            swapped.push_back(left);
        }
        wave.setSamples(swapped);
    }
}

void Tools::gain(WavFile &wave, double gain)
{
    std::vector<double> newSamples;

    for (auto& sample : wave.getSamples())
    {
        newSamples.push_back(sample * gain);
    }

    wave.setSamples(newSamples);
}

void Tools::decibelGain(WavFile &wave, double dB)
{
    gain(wave, pow(10.0, dB / 20.0));
}

double Tools::getPeakDecibels(WavFile& wave) 
{
    double highValue = 0;

    //Find loudest sample
    for (auto& sample : wave.getSamples())
    {
        if (fabs(sample) > fabs(highValue))
        {
            highValue = sample;
        }
    }

    return 20.0 * log10(fabs(highValue));
}
