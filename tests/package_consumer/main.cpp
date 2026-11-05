#include <wavlab/Tools.h>
#include <wavlab/WavFile.h>

#include <cmath>

int main()
{
    auto wave = wavlab::WavFile::create(1, 48'000, 16);
    wave.setSamples({0.25, -0.5});
    wavlab::Tools::decibelGain(wave, 6.020599913279624);

    return std::abs(wave.samples()[0] - 0.5) < 1.0e-12 ? 0 : 1;
}
