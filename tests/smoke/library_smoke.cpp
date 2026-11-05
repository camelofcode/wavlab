#include <wavlab/WavFile.h>

#include <cstdint>

int main()
{
    constexpr std::uint16_t channels = 1;
    constexpr std::uint32_t sampleRate = 8'000;
    constexpr std::uint16_t bitsPerSample = 16;

    auto wav = wavlab::WavFile::create(channels, sampleRate, bitsPerSample);
    if (wav.numberOfChannels() != channels || wav.sampleRate() != sampleRate ||
        wav.bitsPerSample() != bitsPerSample || !wav.samples().empty())
    {
        return 1;
    }

    return 0;
}
