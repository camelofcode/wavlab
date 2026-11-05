#include "WavFile.h"

#include <cstdint>

int main()
{
    constexpr std::uint16_t channels = 1;
    constexpr std::uint32_t sampleRate = 8'000;
    constexpr std::uint16_t bitsPerSample = 16;

    WavFile wav(channels, sampleRate, bitsPerSample);
    if (wav.getNumberOfChannels() != channels || wav.getSampleRate() != sampleRate ||
        wav.getBitsPerSample() != bitsPerSample || !wav.getSamples().empty())
    {
        return 1;
    }

    return 0;
}
