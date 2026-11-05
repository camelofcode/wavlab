#include <wavlab/WavFile.h>

#include <chrono>
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <iostream>
#include <string_view>
#include <vector>

namespace
{

int failures = 0;

void check(bool condition, std::string_view message)
{
    if (!condition)
    {
        std::cerr << "FAILED: " << message << '\n';
        ++failures;
    }
}

template <typename Action>
void checkError(Action action, wavlab::WavErrorCode expectedCode, std::string_view message)
{
    try
    {
        action();
        check(false, message);
    }
    catch (const wavlab::WavError &error)
    {
        check(error.code() == expectedCode, message);
    }
}

std::filesystem::path temporaryWavPath()
{
    const auto suffix = std::chrono::steady_clock::now().time_since_epoch().count();
    return std::filesystem::temp_directory_path() /
           ("wavlab-library-test-" + std::to_string(suffix) + ".wav");
}

} // namespace

int main()
{
    using wavlab::WavErrorCode;
    using wavlab::WavFile;

    auto wav = WavFile::create(2, 48'000, 16);
    check(wav.numberOfChannels() == 2, "create stores the channel count");
    check(wav.sampleRate() == 48'000, "create stores the sample rate");
    check(wav.bitsPerSample() == 16, "create stores the bit depth");
    check(wav.sampleCount() == 0 && wav.frameCount() == 0, "new files are empty");
    check(wav.hasLoadedSamples(), "new files have an editable sample buffer");

    checkError([] { static_cast<void>(WavFile::create(3, 48'000, 16)); },
               WavErrorCode::unsupportedChannelCount, "create rejects unsupported channel counts");
    checkError([] { static_cast<void>(WavFile::create(1, 0, 16)); },
               WavErrorCode::invalidSampleRate, "create rejects a zero sample rate");
    checkError([] { static_cast<void>(WavFile::create(1, 48'000, 12)); },
               WavErrorCode::unsupportedBitsPerSample, "create rejects unsupported PCM bit depths");
    checkError([&wav] { wav.setSamples({0.25}); }, WavErrorCode::incompleteSampleFrame,
               "setSamples rejects an incomplete stereo frame");

    wav.setSamples({0.0, 0.5, -0.5, 1.0});
    check(wav.sampleCount() == 4, "sampleCount counts interleaved samples");
    check(wav.frameCount() == 2, "frameCount counts complete frames");
    wav.samples()[0] = 0.25;
    check(wav.samples()[0] == 0.25, "mutable sample spans update samples in place");

    const auto path = temporaryWavPath();
    wav.save(path);

    const auto metadata = WavFile::openMetadata(path);
    check(!metadata.hasLoadedSamples(), "openMetadata does not load sample values");
    check(metadata.sampleCount() == 4 && metadata.frameCount() == 2,
          "metadata exposes sample and frame counts");
    check(metadata.details().find("Number Of Channels: 2") != std::string::npos,
          "details returns a printable metadata report");
    checkError([&metadata] { static_cast<void>(metadata.samples()); },
               WavErrorCode::samplesNotLoaded, "metadata-only files reject sample access");

    const auto reopened = WavFile::open(path);
    check(reopened.numberOfChannels() == 2, "saved stereo layout can be reopened");
    check(reopened.samples().size() == 4, "round trip preserves the sample count");
    check(std::abs(reopened.samples()[0] - 0.25) < 0.0001, "round trip preserves sample values");

    std::error_code removeError;
    std::filesystem::remove(path, removeError);
    check(!removeError, "temporary WAV file can be removed");

    checkError([&path] { static_cast<void>(WavFile::open(path)); }, WavErrorCode::io,
               "open reports missing files as typed I/O errors");

    return failures == 0 ? 0 : 1;
}
