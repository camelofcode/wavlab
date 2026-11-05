#include <wavlab/WavFile.h>

#include <chrono>
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <fstream>
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

std::vector<char> fileBytes(const std::filesystem::path &path)
{
    std::ifstream file(path, std::ios::binary);
    return {std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()};
}

void writeFileBytes(const std::filesystem::path &path, const std::vector<char> &bytes)
{
    std::ofstream file(path, std::ios::binary | std::ios::trunc);
    file.write(bytes.data(), static_cast<std::streamsize>(bytes.size()));
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

    auto metadata = WavFile::openMetadata(path);
    check(!metadata.hasLoadedSamples(), "openMetadata does not load sample values");
    check(metadata.sampleCount() == 4 && metadata.frameCount() == 2,
          "metadata exposes sample and frame counts");
    check(metadata.details().find("Number Of Channels: 2") != std::string::npos,
          "details returns a printable metadata report");
    checkError([&metadata] { static_cast<void>(metadata.samples()); },
               WavErrorCode::samplesNotLoaded, "metadata-only files reject sample access");
    const auto bytesBeforeFailedOverwrite = fileBytes(path);
    checkError([&metadata, &path] { metadata.save(path); }, WavErrorCode::samplesNotLoaded,
               "saving metadata-only input reports an error");
    check(fileBytes(path) == bytesBeforeFailedOverwrite,
          "a failed overwrite leaves the destination unchanged");

    const auto reopened = WavFile::open(path);
    check(reopened.numberOfChannels() == 2, "saved stereo layout can be reopened");
    check(reopened.samples().size() == 4, "round trip preserves the sample count");
    check(std::abs(reopened.samples()[0] - 0.25) < 0.0001, "round trip preserves sample values");

    auto overwrite = WavFile::open(path);
    overwrite.samples()[0] = -0.25;
    overwrite.save(path);
    check(std::abs(WavFile::open(path).samples()[0] + 0.25) < 0.0001,
          "saving over an input file uses a successful replacement");

    std::error_code removeError;
    std::filesystem::remove(path, removeError);
    check(!removeError, "temporary WAV file can be removed");

    checkError([&path] { static_cast<void>(WavFile::open(path)); }, WavErrorCode::io,
               "open reports missing files as typed I/O errors");

    const std::filesystem::path fixtureDirectory = WAVLAB_FIXTURE_DIR;
    const auto unknownChunk = WavFile::open(fixtureDirectory / "unknown_odd_chunk.wav");
    check(unknownChunk.sampleCount() == 2,
          "unknown odd-sized chunks and their padding are skipped");

    for (const std::string_view fixtureName :
         {"pcm_u8_mono.wav", "pcm_s16_mono.wav", "pcm_s24_mono.wav", "pcm_s32_mono.wav",
          "pcm_s16_stereo.wav", "empty_pcm_s16_mono.wav"})
    {
        const auto fixture = WavFile::open(fixtureDirectory / fixtureName);
        check(fixture.sampleCount() == fixture.samples().size(),
              "PCM fixtures load their declared sample counts");
    }

    for (const std::string_view fixtureName : {"bext_before_data.wav", "bext_after_data.wav"})
    {
        const auto inputPath = fixtureDirectory / fixtureName;
        auto fixture = WavFile::open(inputPath);
        check(fixture.details().find("wavlab tests") != std::string::npos,
              "BEXT text metadata is loaded");
        const auto outputPath = temporaryWavPath();
        fixture.save(outputPath);
        check(fileBytes(inputPath) == fileBytes(outputPath),
              "BEXT position and unknown extra data survive a round trip");
        std::filesystem::remove(outputPath, removeError);
    }

    const auto oddInputPath = fixtureDirectory / "pcm_u8_mono.wav";
    auto oddFixture = WavFile::open(oddInputPath);
    const auto oddOutputPath = temporaryWavPath();
    oddFixture.save(oddOutputPath);
    check(fileBytes(oddInputPath) == fileBytes(oddOutputPath),
          "odd-sized sample data is serialized with RIFF padding");
    std::filesystem::remove(oddOutputPath, removeError);

    const auto validBytes = fileBytes(fixtureDirectory / "pcm_s16_mono.wav");
    auto corruptBytes = validBytes;
    corruptBytes[0] = 'N';
    const auto corruptPath = temporaryWavPath();
    writeFileBytes(corruptPath, corruptBytes);
    checkError([&corruptPath] { static_cast<void>(WavFile::open(corruptPath)); },
               WavErrorCode::invalidFile, "invalid RIFF identifiers are rejected");

    auto incompleteFrameBytes = validBytes;
    incompleteFrameBytes[40] = 9;
    incompleteFrameBytes[41] = 0;
    incompleteFrameBytes[42] = 0;
    incompleteFrameBytes[43] = 0;
    const auto incompleteFramePath = temporaryWavPath();
    writeFileBytes(incompleteFramePath, incompleteFrameBytes);
    checkError([&incompleteFramePath] { static_cast<void>(WavFile::open(incompleteFramePath)); },
               WavErrorCode::invalidFile, "partial PCM sample frames are rejected");

    auto truncatedBytes = validBytes;
    truncatedBytes.pop_back();
    const auto truncatedPath = temporaryWavPath();
    writeFileBytes(truncatedPath, truncatedBytes);
    checkError([&truncatedPath] { static_cast<void>(WavFile::open(truncatedPath)); },
               WavErrorCode::invalidFile, "truncated WAV files are rejected");
    std::filesystem::remove(corruptPath, removeError);
    std::filesystem::remove(incompleteFramePath, removeError);
    std::filesystem::remove(truncatedPath, removeError);

    return failures == 0 ? 0 : 1;
}
