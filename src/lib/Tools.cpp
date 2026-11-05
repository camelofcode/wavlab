#include <wavlab/Tools.h>

#include "Constants.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace wavlab
{

ProcessingError::ProcessingError(ProcessingErrorCode code, std::string message)
    : std::invalid_argument(std::move(message)), code_(code)
{
}

ProcessingErrorCode ProcessingError::code() const noexcept { return code_; }

namespace
{

[[nodiscard]] double amplitudeToDecibels(double amplitude)
{
    if (amplitude == 0.0)
    {
        return -std::numeric_limits<double>::infinity();
    }
    return 20.0 * std::log10(amplitude);
}

void requireMono(const WavFile &input, std::string_view role)
{
    if (input.numberOfChannels() != 1)
    {
        throw ProcessingError(ProcessingErrorCode::expectedMono,
                              std::string(role) + " input must be mono, but has " +
                                  std::to_string(input.numberOfChannels()) + " channels");
    }
}

void requireSameSampleRate(const WavFile &first, const WavFile &second)
{
    if (first.sampleRate() != second.sampleRate())
    {
        throw ProcessingError(ProcessingErrorCode::sampleRateMismatch,
                              "sample rates do not match: " + std::to_string(first.sampleRate()) +
                                  " and " + std::to_string(second.sampleRate()));
    }
}

template <typename Combine>
[[nodiscard]] WavFile combineMonoPair(const WavFile &first, const WavFile &second, Combine combine)
{
    requireMono(first, "first");
    requireMono(second, "second");
    requireSameSampleRate(first, second);

    const auto firstSamples = first.samples();
    const auto secondSamples = second.samples();
    const auto frameCount = std::max(firstSamples.size(), secondSamples.size());
    std::vector<double> outputSamples;
    if (frameCount > outputSamples.max_size() / 2)
    {
        throw ProcessingError(ProcessingErrorCode::sizeOverflow,
                              "combined sample data is too large");
    }
    outputSamples.reserve(frameCount * 2);

    for (std::size_t frame = 0; frame < frameCount; ++frame)
    {
        const auto firstSample = frame < firstSamples.size() ? firstSamples[frame] : 0.0;
        const auto secondSample = frame < secondSamples.size() ? secondSamples[frame] : 0.0;
        const auto outputFrame = combine(firstSample, secondSample);
        outputSamples.insert(outputSamples.end(), outputFrame.begin(), outputFrame.end());
    }

    auto output = WavFile::create(2, first.sampleRate(),
                                  std::max(first.bitsPerSample(), second.bitsPerSample()));
    output.setSamples(std::move(outputSamples));
    return output;
}

void requireMinimumInputs(std::span<const WavFile> inputs)
{
    constexpr std::size_t minimumInputs = 2;
    if (inputs.size() < minimumInputs)
    {
        throw ProcessingError(ProcessingErrorCode::tooFewInputs,
                              "at least two input files are required; received " +
                                  std::to_string(inputs.size()));
    }
}

[[nodiscard]] std::uint32_t commonSampleRate(std::span<const WavFile> inputs)
{
    const auto expected = inputs.front().sampleRate();
    for (const auto &input : inputs.subspan(1))
    {
        if (input.sampleRate() != expected)
        {
            throw ProcessingError(ProcessingErrorCode::sampleRateMismatch,
                                  "sample rates do not match: expected " +
                                      std::to_string(expected) + ", received " +
                                      std::to_string(input.sampleRate()));
        }
    }
    return expected;
}

[[nodiscard]] std::uint16_t commonChannelCount(std::span<const WavFile> inputs)
{
    const auto expected = inputs.front().numberOfChannels();
    for (const auto &input : inputs.subspan(1))
    {
        if (input.numberOfChannels() != expected)
        {
            throw ProcessingError(ProcessingErrorCode::channelCountMismatch,
                                  "channel counts do not match: expected " +
                                      std::to_string(expected) + ", received " +
                                      std::to_string(input.numberOfChannels()));
        }
    }
    return expected;
}

[[nodiscard]] std::uint16_t maximumBitDepth(std::span<const WavFile> inputs)
{
    return std::max_element(inputs.begin(), inputs.end(), [](const auto &left, const auto &right)
                            { return left.bitsPerSample() < right.bitsPerSample(); })
        ->bitsPerSample();
}

[[nodiscard]] WavFile fileWithSamplesLike(const WavFile &source, std::uint16_t channels,
                                          std::vector<double> samples)
{
    auto output = WavFile::create(channels, source.sampleRate(), source.bitsPerSample());
    output.setSamples(std::move(samples));
    return output;
}

} // namespace

namespace Tools
{

void normalise(WavFile &wave)
{
    const auto samples = wave.samples();
    double peak = 0.0;
    for (const auto sample : samples)
    {
        peak = std::max(peak, std::abs(sample));
    }

    if (peak == 0.0)
    {
        return;
    }

    gain(wave, 1.0 / peak);
}

void normalize(WavFile &wave) { normalise(wave); }

void normaliseToAmplitude(WavFile &wave, double targetAmplitude)
{
    normalise(wave);
    gain(wave, targetAmplitude);
}

void normalise(WavFile &wave, double targetAmplitude)
{
    normaliseToAmplitude(wave, targetAmplitude);
}

void normalizeToAmplitude(WavFile &wave, double targetAmplitude)
{
    normaliseToAmplitude(wave, targetAmplitude);
}

void normaliseToDecibels(WavFile &wave, double targetDecibels)
{
    normalise(wave);
    decibelGain(wave, targetDecibels);
}

void normalizeToDecibels(WavFile &wave, double targetDecibels)
{
    normaliseToDecibels(wave, targetDecibels);
}

void reverse(WavFile &wave)
{
    const auto channels = static_cast<std::size_t>(wave.numberOfChannels());
    auto samples = wave.samples();
    const auto frames = samples.size() / channels;

    for (std::size_t first = 0; first < frames / 2; ++first)
    {
        const auto last = frames - first - 1;
        for (std::size_t channel = 0; channel < channels; ++channel)
        {
            std::swap(samples[first * channels + channel], samples[last * channels + channel]);
        }
    }
}

void flipPhase(WavFile &wave)
{
    for (auto &sample : wave.samples())
    {
        sample = -sample;
    }
}

void swapChannels(WavFile &wave)
{
    if (wave.numberOfChannels() == 1)
    {
        return;
    }

    auto samples = wave.samples();
    for (std::size_t frame = 0; frame < samples.size(); frame += 2)
    {
        std::swap(samples[frame], samples[frame + 1]);
    }
}

std::pair<WavFile, WavFile> splitStereo(const WavFile &stereo)
{
    if (stereo.numberOfChannels() != 2)
    {
        throw ProcessingError(ProcessingErrorCode::expectedStereo,
                              "input must be stereo, but has " +
                                  std::to_string(stereo.numberOfChannels()) + " channels");
    }

    const auto samples = stereo.samples();
    std::vector<double> leftSamples;
    std::vector<double> rightSamples;
    leftSamples.reserve(samples.size() / 2);
    rightSamples.reserve(samples.size() / 2);

    for (std::size_t frame = 0; frame < samples.size(); frame += 2)
    {
        leftSamples.push_back(samples[frame]);
        rightSamples.push_back(samples[frame + 1]);
    }

    return {fileWithSamplesLike(stereo, 1, std::move(leftSamples)),
            fileWithSamplesLike(stereo, 1, std::move(rightSamples))};
}

WavFile combineLeftRight(const WavFile &left, const WavFile &right)
{
    return combineMonoPair(left, right, [](double leftSample, double rightSample)
                           { return std::array{leftSample, rightSample}; });
}

std::pair<WavFile, WavFile> splitStereoMidSide(const WavFile &stereo)
{
    if (stereo.numberOfChannels() != 2)
    {
        throw ProcessingError(ProcessingErrorCode::expectedStereo,
                              "input must be stereo, but has " +
                                  std::to_string(stereo.numberOfChannels()) + " channels");
    }

    const auto samples = stereo.samples();
    std::vector<double> midSamples;
    std::vector<double> sideSamples;
    midSamples.reserve(samples.size() / 2);
    sideSamples.reserve(samples.size() / 2);

    for (std::size_t frame = 0; frame < samples.size(); frame += 2)
    {
        const auto left = samples[frame];
        const auto right = samples[frame + 1];
        midSamples.push_back((left + right) * Constants::MINUS_THREE_DB);
        sideSamples.push_back((left - right) * Constants::MINUS_THREE_DB);
    }

    return {fileWithSamplesLike(stereo, 1, std::move(midSamples)),
            fileWithSamplesLike(stereo, 1, std::move(sideSamples))};
}

WavFile combineMidSide(const WavFile &mid, const WavFile &side)
{
    const auto decodeScale = 0.5 / Constants::MINUS_THREE_DB;
    return combineMonoPair(mid, side,
                           [decodeScale](double midSample, double sideSample)
                           {
                               return std::array{(midSample + sideSample) * decodeScale,
                                                 (midSample - sideSample) * decodeScale};
                           });
}

void gain(WavFile &wave, double gainFactor)
{
    for (auto &sample : wave.samples())
    {
        sample *= gainFactor;
    }
}

void decibelGain(WavFile &wave, double decibels) { gain(wave, std::pow(10.0, decibels / 20.0)); }

double getPeakDecibels(const WavFile &wave)
{
    double peak = 0.0;
    for (const auto sample : wave.samples())
    {
        peak = std::max(peak, std::abs(sample));
    }
    return amplitudeToDecibels(peak);
}

double getRMSDecibels(const WavFile &wave)
{
    const auto samples = wave.samples();
    if (samples.empty())
    {
        return -std::numeric_limits<double>::infinity();
    }

    double sumOfSquares = 0.0;
    for (const auto sample : samples)
    {
        sumOfSquares += sample * sample;
    }
    return amplitudeToDecibels(std::sqrt(sumOfSquares / static_cast<double>(samples.size())));
}

WavFile mix(std::span<const WavFile> inputs)
{
    requireMinimumInputs(inputs);
    const auto sampleRate = commonSampleRate(inputs);
    const auto outputChannels =
        std::max_element(inputs.begin(), inputs.end(), [](const auto &left, const auto &right)
                         { return left.numberOfChannels() < right.numberOfChannels(); })
            ->numberOfChannels();
    std::size_t frameCount = 0;
    for (const auto &input : inputs)
    {
        static_cast<void>(input.samples());
        frameCount = std::max(frameCount, input.frameCount());
    }

    std::vector<double> outputSamples;
    if (frameCount > outputSamples.max_size() / outputChannels)
    {
        throw ProcessingError(ProcessingErrorCode::sizeOverflow, "mixed sample data is too large");
    }
    outputSamples.reserve(frameCount * outputChannels);
    const auto inputCount = static_cast<double>(inputs.size());

    for (std::size_t frame = 0; frame < frameCount; ++frame)
    {
        if (outputChannels == 1)
        {
            double sum = 0.0;
            for (const auto &input : inputs)
            {
                const auto samples = input.samples();
                sum += frame < samples.size() ? samples[frame] : 0.0;
            }
            outputSamples.push_back(sum / inputCount);
            continue;
        }

        double leftSum = 0.0;
        double rightSum = 0.0;
        for (const auto &input : inputs)
        {
            const auto samples = input.samples();
            if (input.numberOfChannels() == 1)
            {
                const auto sample = frame < samples.size() ? samples[frame] : 0.0;
                leftSum += sample;
                rightSum += sample;
            }
            else
            {
                const auto samplePosition = frame * 2;
                if (samplePosition < samples.size())
                {
                    leftSum += samples[samplePosition];
                    rightSum += samples[samplePosition + 1];
                }
            }
        }
        outputSamples.push_back(leftSum / inputCount);
        outputSamples.push_back(rightSum / inputCount);
    }

    auto output = WavFile::create(outputChannels, sampleRate, maximumBitDepth(inputs));
    output.setSamples(std::move(outputSamples));
    normalise(output);
    return output;
}

std::vector<WavFile> split(const WavFile &input, std::span<const std::uint64_t> splitTimesMs)
{
    const auto samples = input.samples();
    const auto channels = static_cast<std::size_t>(input.numberOfChannels());
    const auto frameCount = samples.size() / channels;
    std::vector<std::size_t> splitSamplePositions;
    splitSamplePositions.reserve(splitTimesMs.size());
    std::uint64_t previousFrame = 0;

    for (const auto milliseconds : splitTimesMs)
    {
        if (milliseconds > std::numeric_limits<std::uint64_t>::max() / input.sampleRate())
        {
            throw ProcessingError(ProcessingErrorCode::invalidSplitTime,
                                  "invalid split time " + std::to_string(milliseconds) + " ms");
        }
        const auto frame = milliseconds * input.sampleRate() / 1'000;
        if (frame <= previousFrame || frame >= frameCount)
        {
            throw ProcessingError(ProcessingErrorCode::invalidSplitTime,
                                  "invalid split time " + std::to_string(milliseconds) + " ms");
        }
        previousFrame = frame;
        splitSamplePositions.push_back(static_cast<std::size_t>(frame) * channels);
    }

    std::vector<WavFile> outputs;
    outputs.reserve(splitSamplePositions.size() + 1);
    std::size_t start = 0;
    for (const auto end : splitSamplePositions)
    {
        outputs.push_back(fileWithSamplesLike(
            input, input.numberOfChannels(),
            std::vector<double>(samples.begin() + static_cast<std::ptrdiff_t>(start),
                                samples.begin() + static_cast<std::ptrdiff_t>(end))));
        start = end;
    }
    outputs.push_back(fileWithSamplesLike(
        input, input.numberOfChannels(),
        std::vector<double>(samples.begin() + static_cast<std::ptrdiff_t>(start), samples.end())));
    return outputs;
}

WavFile join(std::span<const WavFile> inputs)
{
    requireMinimumInputs(inputs);
    const auto sampleRate = commonSampleRate(inputs);
    const auto channels = commonChannelCount(inputs);
    std::vector<double> joinedSamples;
    std::size_t totalSamples = 0;
    for (const auto &input : inputs)
    {
        const auto samples = input.samples();
        if (samples.size() > joinedSamples.max_size() - totalSamples)
        {
            throw ProcessingError(ProcessingErrorCode::sizeOverflow,
                                  "joined sample data is too large");
        }
        totalSamples += samples.size();
    }
    joinedSamples.reserve(totalSamples);
    for (const auto &input : inputs)
    {
        const auto samples = input.samples();
        joinedSamples.insert(joinedSamples.end(), samples.begin(), samples.end());
    }

    auto output = WavFile::create(channels, sampleRate, maximumBitDepth(inputs));
    output.setSamples(std::move(joinedSamples));
    return output;
}

} // namespace Tools

} // namespace wavlab
