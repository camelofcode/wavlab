#include "Tools.h"

void Tools::Normalise(WavFile &wave)
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

void Tools::Gain(WavFile &wave, double gain)
{
    std::vector<double> newSamples;

    for (auto& sample : wave.getSamples())
    {
        newSamples.push_back(sample * gain);
    }

    wave.setSamples(newSamples);
}

void Tools::DecibelGain(WavFile &wave, double dB)
{
    Gain(wave, pow(10.0, dB / 20.0));
}

double Tools::GetPeakDecibels(WavFile& wave) 
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
