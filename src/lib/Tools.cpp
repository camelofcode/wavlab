#include "Constants.h"
#include <algorithm>
#include <cmath>
#include <wavlab/Tools.h>

namespace wavlab
{

/* stat command */

void Tools::normalise(WavFile &wave)
{
    double highValue = 0;

    // Find loudest sample
    for (auto &sample : wave.samples())
    {
        if (fabs(sample) > fabs(highValue))
        {
            highValue = sample;
        }
    }

    // Calculate gain
    double peak = fabs(highValue);
    double gain = 1 / peak;

    std::vector<double> newSamples;

    // Write new samples with gain applied
    for (auto &sample : wave.samples())
    {
        newSamples.push_back(sample * gain);
    }

    wave.setSamples(newSamples);
}

void Tools::normalise(WavFile &wave, double normaliseTo)
{
    Tools::normalise(wave);

    std::vector<double> newSamples;
    for (auto &sample : wave.samples())
    {
        newSamples.push_back(sample * normaliseTo);
    }
    wave.setSamples(newSamples);
}

/* reverse command */

void Tools::reverse(WavFile &wave)
{
    if (wave.numberOfChannels() == 1)
    {
        const auto sourceSamples = wave.samples();
        std::vector<double> original(sourceSamples.begin(), sourceSamples.end());
        std::reverse(original.begin(), original.end());
        wave.setSamples(original);
    }
    else if (wave.numberOfChannels() == 2)
    {
        const auto sourceSamples = wave.samples();
        std::vector<double> original(sourceSamples.begin(), sourceSamples.end());
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

/* flip-phase command */

void Tools::flipPhase(WavFile &wave)
{
    std::vector<double> newSamples;

    for (auto &sample : wave.samples())
    {
        newSamples.push_back(-sample);
    }
    wave.setSamples(newSamples);
}

/* swap-channels command */

void Tools::swapChannels(WavFile &wave)
{
    if (wave.numberOfChannels() == 1)
    {
        // Can't do anything to a mono file, but why error, no harm done!
        return;
    }
    else if (wave.numberOfChannels() == 2)
    {
        const auto original = wave.samples();
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

/* split-stereo command */

void Tools::splitStereo(const WavFile &stereo, WavFile &left, WavFile &right)
{
    const auto original = stereo.samples();
    std::vector<double> leftSamples;
    std::vector<double> rightSamples;

    for (size_t i = 0; i < original.size() - 1; i = i + 2)
    {
        double leftSample = original[i];
        double rightSample = original[i + 1];

        leftSamples.push_back(leftSample);
        rightSamples.push_back(rightSample);
    }

    left.setSamples(leftSamples);
    right.setSamples(rightSamples);
}

/* left-right-to-stereo command */

void Tools::combineLeftRight(const WavFile &left, const WavFile &right, WavFile &out)
{
    const auto leftSamples = left.samples();
    const auto rightSamples = right.samples();
    std::vector<double> stereoSamples;

    auto allNumberOfSamples = std::vector<size_t>({leftSamples.size(), rightSamples.size()});
    size_t maxNumberOfSamples =
        *std::max_element(allNumberOfSamples.begin(), allNumberOfSamples.end());

    for (size_t i = 0; i < maxNumberOfSamples - 1; i++)
    {
        double leftSample = i < leftSamples.size() - 1 ? leftSamples[i] : 0.0;
        double rightSample = i < rightSamples.size() - 1 ? rightSamples[i] : 0.0;

        stereoSamples.push_back(leftSample);
        stereoSamples.push_back(rightSample);
    }

    out.setSamples(stereoSamples);
}

/* split-mid-side command */

void Tools::splitStereoMidSide(const WavFile &stereo, WavFile &mid, WavFile &side)
{
    // See https://www.soundonsound.com/sound-advice/q-how-does-mid-sides-recording-actually-work

    const auto original = stereo.samples();
    std::vector<double> midSamples;
    std::vector<double> sideSamples;

    for (size_t i = 0; i < original.size() - 1; i = i + 2)
    {
        double left = original[i];
        double right = original[i + 1];

        midSamples.push_back((left + right) * Constants::MINUS_THREE_DB);
        sideSamples.push_back((left - right) * Constants::MINUS_THREE_DB);
    }

    mid.setSamples(midSamples);
    side.setSamples(sideSamples);
}

/* mid-side-to-stereo */

void Tools::combineMidSide(const WavFile &mid, const WavFile &side, WavFile &out)
{
    // See https://www.soundonsound.com/sound-advice/q-how-does-mid-sides-recording-actually-work

    const auto midSamples = mid.samples();
    const auto sideSamples = side.samples();
    std::vector<double> stereoSamples;

    auto allNumberOfSamples = std::vector<size_t>({midSamples.size(), sideSamples.size()});
    size_t maxNumberOfSamples =
        *std::max_element(allNumberOfSamples.begin(), allNumberOfSamples.end());

    for (size_t i = 0; i < maxNumberOfSamples - 1; i++)
    {
        double midSample = i < midSamples.size() - 1 ? midSamples[i] : 0.0;
        double sideSample = i < sideSamples.size() - 1 ? sideSamples[i] : 0.0;

        stereoSamples.push_back((midSample + sideSample) * Constants::MINUS_THREE_DB);
        stereoSamples.push_back((midSample - sideSample) * Constants::MINUS_THREE_DB);
    }

    out.setSamples(stereoSamples);
}

/* gain command */

void Tools::gain(WavFile &wave, double gain)
{
    std::vector<double> newSamples;

    for (auto &sample : wave.samples())
    {
        newSamples.push_back(sample * gain);
    }

    wave.setSamples(newSamples);
}

/* gain-db command */

void Tools::decibelGain(WavFile &wave, double dB) { gain(wave, pow(10.0, dB / 20.0)); }

/* get-peak-db command */

double Tools::getPeakDecibels(const WavFile &wave)
{
    double highValue = 0;

    // Find loudest sample
    for (auto &sample : wave.samples())
    {
        if (fabs(sample) > fabs(highValue))
        {
            highValue = sample;
        }
    }

    return 20.0 * log10(fabs(highValue));
}

/* get-rms-db command */

double Tools::getRMSDecibels(const WavFile &wave)
{
    const auto samples = wave.samples();
    double sumOfSquares = 0;

    for (auto &sample : samples)
    {
        sumOfSquares += pow(sample, 2.0);
    }

    double mean = sumOfSquares / static_cast<double>(samples.size());
    double rms = sqrt(mean);

    return 20.0 * log10(rms);
}

/* mix command */
void Tools::mix(std::span<const WavFile> inputs, WavFile &output)
{
    std::vector<size_t> allNumberOfSamples;
    std::vector<std::vector<double>> allSamples;
    for (auto &input : inputs)
    {
        std::vector<double> samples;
        if (input.numberOfChannels() == 2)
        {
            const auto inputSamples = input.samples();
            samples.assign(inputSamples.begin(), inputSamples.end());
        }
        if (input.numberOfChannels() == 1)
        {
            // Convert mono samples into stereo.
            for (auto &sample : input.samples())
            {
                samples.push_back(sample);
                samples.push_back(sample);
            }
        }
        allNumberOfSamples.push_back(samples.size());
        allSamples.push_back(samples);
    }

    size_t numberOfInputs = inputs.size();
    size_t maxSamples = *std::max_element(allNumberOfSamples.begin(), allNumberOfSamples.end());

    std::vector<double> newSamples;

    if (output.numberOfChannels() == 1)
    {
        for (size_t i = 0; i < maxSamples; i++)
        {
            double addedSamples = 0.0;
            for (auto &samples : allSamples)
            {
                double sample = samples.size() <= maxSamples ? samples[i] : 0.0;
                addedSamples += sample / static_cast<double>(numberOfInputs);
            }
            newSamples.push_back(addedSamples);
        }
    }
    else if (output.numberOfChannels() == 2)
    {
        for (size_t i = 0; i < maxSamples - 1; i = i + 2)
        {
            double addedSamplesLeft = 0.0;
            double addedSamplesRight = 0.0;
            for (auto &samples : allSamples)
            {
                double sampleLeft = samples.size() <= maxSamples ? samples[i] : 0.0;
                double sampleRight = samples.size() <= maxSamples ? samples[i + 1] : 0.0;
                addedSamplesLeft += sampleLeft / static_cast<double>(numberOfInputs);
                addedSamplesRight += sampleRight / static_cast<double>(numberOfInputs);
            }
            newSamples.push_back(addedSamplesLeft);
            newSamples.push_back(addedSamplesRight);
        }
    }

    output.setSamples(newSamples);
    Tools::normalise(output);
}

/* split command */

void Tools::split(const WavFile &input, std::vector<WavFile> &outputs,
                  const std::vector<uint32_t> &splitTimesMs)
{
    std::vector<uint32_t> splitSamplePositions;
    uint32_t sampleRateMultiplier = input.sampleRate() / 1000;

    for (const uint32_t &ms : splitTimesMs)
    {
        splitSamplePositions.push_back(ms * sampleRateMultiplier * input.numberOfChannels());
    }

    unsigned int splitPosition = 0;
    uint32_t nextSplit = splitSamplePositions[splitPosition];
    auto currentSamples = new std::vector<double>();

    auto originalSamples = input.samples();

    if (input.numberOfChannels() == 1)
    {
        for (size_t i = 0; i < originalSamples.size(); i++)
        {
            currentSamples->push_back(originalSamples[i]);
            if (i == nextSplit)
            {
                outputs[splitPosition].setSamples(*currentSamples);
                splitPosition++;
                if (splitPosition < splitSamplePositions.size())
                {
                    nextSplit = splitSamplePositions[splitPosition];
                }
                currentSamples = new std::vector<double>();
            }
        }
    }
    else if (input.numberOfChannels() == 2)
    {
        for (size_t i = 0; i < originalSamples.size() - 1; i = i + 2)
        {
            currentSamples->push_back(originalSamples[i]);
            currentSamples->push_back(originalSamples[i + 1]);
            if (i == nextSplit)
            {
                outputs[splitPosition].setSamples(*currentSamples);
                splitPosition++;
                if (splitPosition < splitSamplePositions.size())
                {
                    nextSplit = splitSamplePositions[splitPosition];
                }
                currentSamples = new std::vector<double>();
            }
        }
    }
    outputs[splitPosition].setSamples(*currentSamples);
}

/* join command */

void Tools::join(std::span<const WavFile> inputs, WavFile &output)
{
    std::vector<double> joinedSamples;
    for (const WavFile &wav : inputs)
    {
        auto wavSamples = wav.samples();
        joinedSamples.insert(joinedSamples.end(), wavSamples.begin(), wavSamples.end());
    }
    output.setSamples(joinedSamples);
}

} // namespace wavlab
