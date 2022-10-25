#define _USE_MATH_DEFINES
#include "FX.h"
#include <cmath>

void FX::saturate(WavFile& wave, double drive)
{
	//At one point (2009) this algorithm existed at http://tap-plugins.sourceforge.net/ladspa/sigmoid.html

    double gainFactor = (drive + 0.7) * 1.2;

    std::vector<double> newSamples;

    for (auto& sample : wave.getSamples())
    {
        double preSaturation = sample * gainFactor;
        double saturated = 2 / (1 + (pow(M_E, (-5.0 * preSaturation)))) - 1;
        newSamples.push_back(saturated);
    }
    wave.setSamples(newSamples);
}