#include "Tools.h"
#include <algorithm>
#include "Commands.h"
#include "Constants.h"
#include <cmath>

/* stat command */

void Commands::statHelp()
{
    std::cout << "stat <input>" << std::endl;
    std::cout << "\tdescription: show stats of a wave file" << std::endl;
    std::cout << "\tparameters:" << std::endl;
    std::cout << "\t\tinput: path to wave file" << std::endl;
}

int Commands::statCommand(std::string command, int argc, char* argv[])
{
    if (argc != 3)
    {
        std::cout << "Incorrect number of arguments for command 'stat'. To see available commands run 'wavlab help'" << std::endl;
        return 1;
    }
    std::string path = static_cast<std::string>(argv[2]);
    WavFile wav(path, false);
    wav.printDetails();
    return 0;
}

/* normalise command */

void Commands::normaliseHelp()
{
    std::cout << "normalise <input> [output]" << std::endl;
    std::cout << "\tdescription: normalises a wave file" << std::endl;
    std::cout << "\tparameters:" << std::endl;
    std::cout << "\t\tinput: path to wave file" << std::endl;
    std::cout << "\t\toutput: path to output file. Optional, if not specified input will be overwritten" << std::endl;
}

int Commands::normaliseCommand(std::string command, int argc, char* argv[])
{
    if (argc != 3 && argc != 4)
    {
        std::cout << "Incorrect number of arguments for command 'normalise'. To see available commands run 'wavlab help'" << std::endl;
        return 1;
    }
    std::string input = static_cast<std::string>(argv[2]);
    std::string output = argc == 4 ? static_cast<std::string>(argv[3]) : input;

    WavFile wav(input, true);
    Tools::normalise(wav);
    wav.save(output);
    return 0;
}

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

/* reverse command */

void Commands::reverseHelp()
{
    std::cout << "reverse <input> [output]" << std::endl;
    std::cout << "\tdescription: reverses a wave file" << std::endl;
    std::cout << "\tparameters:" << std::endl;
    std::cout << "\t\tinput: path to wave file" << std::endl;
    std::cout << "\t\toutput: path to output file. Optional, if not specified input will be overwritten" << std::endl;
}

int Commands::reverseCommand(std::string command, int argc, char* argv[])
{
    if (argc != 3 && argc != 4)
    {
        std::cout << "Incorrect number of arguments for command 'reverse'. To see available commands run 'wavlab help'" << std::endl;
        return 1;
    }
    std::string input = static_cast<std::string>(argv[2]);
    std::string output = argc == 4 ? static_cast<std::string>(argv[3]) : input;

    WavFile wav(input, true);
    Tools::reverse(wav);
    wav.save(output);
    return 0;
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

/* flip-phase command */

void Commands::flipPhaseHelp()
{
    std::cout << "flip-phase <input> [output]" << std::endl;
    std::cout << "\tdescription: reverses the phase of a wave file" << std::endl;
    std::cout << "\tparameters:" << std::endl;
    std::cout << "\t\tinput: path to wave file" << std::endl;
    std::cout << "\t\toutput: path to output file. Optional, if not specified input will be overwritten" << std::endl;
}

int Commands::flipPhaseCommand(std::string command, int argc, char* argv[])
{
    if (argc != 3 && argc != 4)
    {
        std::cout << "Incorrect number of arguments for command 'flip-phase'. To see available commands run 'wavlab help'" << std::endl;
        return 1;
    }
    std::string input = static_cast<std::string>(argv[2]);
    std::string output = argc == 4 ? static_cast<std::string>(argv[3]) : input;

    WavFile wav(input, true);
    Tools::flipPhase(wav);
    wav.save(output);
    return 0;
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

/* swap-channels command */

void Commands::swapChannelsHelp() 
{
    std::cout << "swap-channels <input> [output]" << std::endl;
    std::cout << "\tdescription: swaps the left and right channels of a stereo wave file" << std::endl;
    std::cout << "\tparameters:" << std::endl;
    std::cout << "\t\tinput: path to wave file" << std::endl;
    std::cout << "\t\toutput: path to output file. Optional, if not specified input will be overwritten" << std::endl;
}

int Commands::swapChannelsCommand(std::string command, int argc, char* argv[])
{
    if (argc != 3 && argc != 4)
    {
        std::cout << "Incorrect number of arguments for command 'swap-channels'. To see available commands run 'wavlab help'" << std::endl;
        return 1;
    }
    std::string input = static_cast<std::string>(argv[2]);
    std::string output = argc == 4 ? static_cast<std::string>(argv[3]) : input;

    WavFile wav(input, true);
    Tools::swapChannels(wav);
    wav.save(output);
    return 0;
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

/* split-stereo command */

void Commands::splitStereoHelp()
{
    std::cout << "split-stereo <input>" << std::endl;
    std::cout << "\tdescription: splits a stereo wave file to two mono files, {original}_l.wav and {original}_r.wav" << std::endl;
    std::cout << "\tparameters:" << std::endl;
    std::cout << "\t\tinput: path to wave file" << std::endl;
}

int Commands::splitStereoCommand(std::string command, int argc, char* argv[])
{
    if (argc != 3)
    {
        std::cout << "Incorrect number of arguments for command 'split-stereo'. To see available commands run 'wavlab help'" << std::endl;
        return 1;
    }
    std::string input = static_cast<std::string>(argv[2]);

    WavFile stereo(input, true);

    if (stereo.getNumberOfChannels() != 2)
    {
        std::cout << "Input file is not stereo." << std::endl;
        return 1;
    }

    WavFile left(1, stereo.getSampleRate(), stereo.getBitsPerSample());
    WavFile right(1, stereo.getSampleRate(), stereo.getBitsPerSample());

    Tools::splitStereo(stereo, left, right);

    left.save(StringHelpers::stringReplace(input, ".wav", "_l.wav"));
    right.save(StringHelpers::stringReplace(input, ".wav", "_r.wav"));

    return 0;
}

void Tools::splitStereo(WavFile& stereo, WavFile& left, WavFile& right)
{
    std::vector<double> original = stereo.getSamples();
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

void Commands::combineLeftRightHelp()
{
    std::cout << "left-right-to-stereo <leftInput> <rightInput> <output>" << std::endl;
    std::cout << "\tdescription: converts two left/right mono wave files to a combined stereo wave file" << std::endl;
    std::cout << "\tparameters:" << std::endl;
    std::cout << "\t\tleftInput: path to left wave file" << std::endl;
    std::cout << "\t\trightInput: path to right wave file" << std::endl;
    std::cout << "\t\toutput: path to combined output file" << std::endl;
}

int Commands::combineLeftRightCommand(std::string command, int argc, char* argv[])
{
    if (argc != 5)
    {
        std::cout << "Incorrect number of arguments for command 'left-right-to-stereo'. To see available commands run 'wavlab help'" << std::endl;
        return 1;
    }
    std::string leftInput = static_cast<std::string>(argv[2]);
    std::string rightInput = static_cast<std::string>(argv[3]);
    std::string output = static_cast<std::string>(argv[4]);

    WavFile left(leftInput, true);
    WavFile right(rightInput, true);

    if (left.getNumberOfChannels() != 1)
    {
        std::cout << "Left input file is not mono." << std::endl;
        return 1;
    }

    if (right.getNumberOfChannels() != 1)
    {
        std::cout << "Right input file is not mono." << std::endl;
        return 1;
    }

    if (left.getSampleRate() != right.getSampleRate())
    {
        std::cout << "Left and Right input files are not the same sample rate." << std::endl;
        return 1;
    }

    auto allSampleRates = std::vector<unsigned short>({ left.getBitsPerSample(), right.getBitsPerSample() });
    unsigned short maxSampleRate = *std::max_element(allSampleRates.begin(), allSampleRates.end());

    WavFile stereo(2, left.getSampleRate(), maxSampleRate);

    Tools::combineLeftRight(left, right, stereo);

    stereo.save(output);

    return 0;
}

void Tools::combineLeftRight(WavFile& left, WavFile& right, WavFile& out)
{
    std::vector<double> leftSamples = left.getSamples();
    std::vector<double> rightSamples = right.getSamples();
    std::vector<double> stereoSamples;

    auto allNumberOfSamples = std::vector<size_t>({ leftSamples.size(), rightSamples.size()});
    size_t maxNumberOfSamples = *std::max_element(allNumberOfSamples.begin(), allNumberOfSamples.end());

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

void Commands::splitStereoMidSideHelp()
{
    std::cout << "split-mid-side <input>" << std::endl;
    std::cout << "\tdescription: splits a stereo wave file to mid and side components, {original}_mid.wav and {original}_side.wav" << std::endl;
    std::cout << "\tparameters:" << std::endl;
    std::cout << "\t\tinput: path to wave file" << std::endl;
}

int Commands::splitStereoMidSideCommand(std::string command, int argc, char* argv[])
{
    if (argc != 3)
    {
        std::cout << "Incorrect number of arguments for command 'split-mid-side'. To see available commands run 'wavlab help'" << std::endl;
        return 1;
    }
    std::string input = static_cast<std::string>(argv[2]);

    WavFile stereo(input, true);

    if (stereo.getNumberOfChannels() != 2)
    {
        std::cout << "Input file is not stereo." << std::endl;
        return 1;
    }

    WavFile mid(1, stereo.getSampleRate(), stereo.getBitsPerSample());
    WavFile side(1, stereo.getSampleRate(), stereo.getBitsPerSample());

    Tools::splitStereoMidSide(stereo, mid, side);

    mid.save(StringHelpers::stringReplace(input, ".wav", "_mid.wav"));
    side.save(StringHelpers::stringReplace(input, ".wav", "_side.wav"));

    return 0;
}

void Tools::splitStereoMidSide(WavFile& stereo, WavFile& mid, WavFile& side)
{
    //See https://www.soundonsound.com/sound-advice/q-how-does-mid-sides-recording-actually-work

    std::vector<double> original = stereo.getSamples();
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

void Commands::combineMidSideHelp()
{
    std::cout << "mid-side-to-stereo <midInput> <sideInput> <output>" << std::endl;
    std::cout << "\tdescription: converts two mid/side mono wave files to a combined stereo wave file" << std::endl;
    std::cout << "\tparameters:" << std::endl;
    std::cout << "\t\tmidInput: path to mid wave file" << std::endl;
    std::cout << "\t\tsideInput: path to side wave file" << std::endl;
    std::cout << "\t\toutput: path to combined output file" << std::endl;
}

int Commands::combineMidSideCommand(std::string command, int argc, char* argv[])
{
    if (argc != 5)
    {
        std::cout << "Incorrect number of arguments for command 'mid-side-to-stereo'. To see available commands run 'wavlab help'" << std::endl;
        return 1;
    }
    std::string midInput = static_cast<std::string>(argv[2]);
    std::string sideInput = static_cast<std::string>(argv[3]);
    std::string output = static_cast<std::string>(argv[4]);

    WavFile mid(midInput, true);
    WavFile side(sideInput, true);

    if (mid.getNumberOfChannels() != 1)
    {
        std::cout << "Mid input file is not mono." << std::endl;
        return 1;
    }

    if (side.getNumberOfChannels() != 1)
    {
        std::cout << "Side input file is not mono." << std::endl;
        return 1;
    }

    if (side.getSampleRate() != mid.getSampleRate())
    {
        std::cout << "Mid and Side input files are not the same sample rate." << std::endl;
        return 1;
    }

    auto allSampleRates = std::vector<unsigned short>({ mid.getBitsPerSample(), side.getBitsPerSample() });
    unsigned short maxSampleRate = *std::max_element(allSampleRates.begin(), allSampleRates.end());

    WavFile stereo(2, side.getSampleRate(), maxSampleRate);

    Tools::combineMidSide(mid, side, stereo);

    stereo.save(output);

    return 0;
}

void Tools::combineMidSide(WavFile& mid, WavFile& side, WavFile& out)
{
    //See https://www.soundonsound.com/sound-advice/q-how-does-mid-sides-recording-actually-work

    std::vector<double> midSamples = mid.getSamples();
    std::vector<double> sideSamples = side.getSamples();
    std::vector<double> stereoSamples;

    auto allNumberOfSamples = std::vector<size_t>({ midSamples.size(), sideSamples.size() });
    size_t maxNumberOfSamples = *std::max_element(allNumberOfSamples.begin(), allNumberOfSamples.end());

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

void Commands::gainHelp()
{
    std::cout << "gain <input> <gain> [output]" << std::endl;
    std::cout << "\tdescription: applies gain to a wave file" << std::endl;
    std::cout << "\tparameters:" << std::endl;
    std::cout << "\t\tinput: path to wave file" << std::endl;
    std::cout << "\t\tgain: gain multiplier" << std::endl;
    std::cout << "\t\toutput: path to output file. Optional, if not specified input will be overwritten" << std::endl;
}

int Commands::gainCommand(std::string command, int argc, char* argv[])
{
    if (argc != 4 && argc != 5)
    {
        std::cout << "Incorrect number of arguments for command 'gain'. To see available commands run 'wavlab help'" << std::endl;
        return 1;
    }
    std::string input = static_cast<std::string>(argv[2]);

    double gain = 0.0;
    if (!tryParse(argv[3], gain))
    {
        std::cout << "Parameter 'gain' could not be parsed. Please ensure it is a numeric value. Provided value: " << argv[3] << std::endl;
        return 1;
    }

    std::string output = argc == 5 ? static_cast<std::string>(argv[4]) : input;

    WavFile wav(input, true);
    Tools::gain(wav, gain);
    wav.save(output);
    return 0;
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

/* gain-db command */

void Commands::gainDbHelp()
{
    std::cout << "gain-db <input> <dB> [output]" << std::endl;
    std::cout << "\tdescription: applies gain to a wave file, with input in decibels" << std::endl;
    std::cout << "\tparameters:" << std::endl;
    std::cout << "\t\tinput: path to wave file" << std::endl;
    std::cout << "\t\tdB: gain to apply in decibels" << std::endl;
    std::cout << "\t\toutput: path to output file. Optional, if not specified input will be overwritten" << std::endl;
}

int Commands::gainDbCommand(std::string command, int argc, char* argv[])
{
    if (argc != 4 && argc != 5)
    {
        std::cout << "Incorrect number of arguments for command 'gain-db'. To see available commands run 'wavlab help'" << std::endl;
        return 1;
    }
    std::string input = static_cast<std::string>(argv[2]);

    double dB = 0.0;
    if (!tryParse(argv[3], dB))
    {
        std::cout << "Parameter 'db' could not be parsed. Please ensure it is a numeric value. Provided value: " << argv[3] << std::endl;
        return 1;
    }

    std::string output = argc == 5 ? static_cast<std::string>(argv[4]) : input;

    WavFile wav(input, true);
    Tools::decibelGain(wav, dB);
    wav.save(output);
    return 0;
}

void Tools::decibelGain(WavFile &wave, double dB)
{
    gain(wave, pow(10.0, dB / 20.0));
}

/* get-peak-db command */

void Commands::getPeakDbHelp()
{
    std::cout << "get-peak-db <input>" << std::endl;
    std::cout << "\tdescription: shows the peak dB level of a wave file" << std::endl;
    std::cout << "\tparameters:" << std::endl;
    std::cout << "\t\tinput: path to wave file" << std::endl;
}

int Commands::getPeakDbCommand(std::string command, int argc, char* argv[])
{
    if (argc != 3)
    {
        std::cout << "Incorrect number of arguments for command 'get-peak-db'. To see available commands run 'wavlab help'" << std::endl;
        return 1;
    }
    std::string input = static_cast<std::string>(argv[2]);

    WavFile wav(input, true);
    std::cout << Tools::getPeakDecibels(wav) << "dB";
    return 0;
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

/* get-rms-db command */

void Commands::getRMSDbHelp()
{
    std::cout << "get-rms-db <input>" << std::endl;
    std::cout << "\tdescription: shows the RMS dB level of a wave file" << std::endl;
    std::cout << "\tparameters:" << std::endl;
    std::cout << "\t\tinput: path to wave file" << std::endl;
}

int Commands::getRMSDbCommand(std::string command, int argc, char* argv[])
{
    if (argc != 3)
    {
        std::cout << "Incorrect number of arguments for command 'get-rms-db'. To see available commands run 'wavlab help'" << std::endl;
        return 1;
    }
    std::string input = static_cast<std::string>(argv[2]);

    WavFile wav(input, true);
    std::cout << Tools::getRMSDecibels(wav) << "dB";
    return 0;
}

double Tools::getRMSDecibels(WavFile& wave)
{
    std::vector<double> samples = wave.getSamples();
    double sumOfSquares = 0;

    for (auto& sample : samples)
    {
        sumOfSquares += pow(sample, 2.0);
    }

    double mean = sumOfSquares / samples.size();
    double rms = sqrt(mean);

    return 20.0 * log10(rms);
}

/* mix command */
void Commands::mixHelp()
{
    std::cout << "mix <output> <input...>" << std::endl;
    std::cout << "\tdescription: mixes multiple wave files into one. Will attenuate and renormalise to ensure lack of clipping. Currently will start all files at zero." << std::endl;
    std::cout << "\tparameters:" << std::endl;
    std::cout << "\t\toutput: path to mixed wave file" << std::endl;
    std::cout << "\t\tinput: arbitrary number of input files (minimum 2)" << std::endl;
}

int Commands::mixCommand(std::string command, int argc, char* argv[])
{
    if (argc < 5)
    {
        std::cout << "Incorrect number of arguments for command 'mix'. To see available commands run 'wavlab help'" << std::endl;
        return 1;
    }
    std::string output = static_cast<std::string>(argv[2]);

    std::vector<WavFile> inputWavs;
    std::vector<unsigned short> allBitDepths;
    std::vector<unsigned short> allNumberOfChannels;
    uint32_t sampleRate = 0;

    for (int i = 3; i < argc; i++)
    {
        std::string input = static_cast<std::string>(argv[i]);
        WavFile inputWav(input, true);

        if (sampleRate != 0)
        {
            if (sampleRate != inputWav.getSampleRate())
            {
                std::cout << "Not all input files are the same sample rate." << std::endl;
                return 1;
            }
        }
        sampleRate = inputWav.getSampleRate();

        allBitDepths.push_back(inputWav.getBitsPerSample());
        allNumberOfChannels.push_back(inputWav.getNumberOfChannels());
        inputWavs.push_back(inputWav);

    }

    unsigned short maxBitDepth = *std::max_element(allBitDepths.begin(), allBitDepths.end());
    unsigned short maxNumberOfChannels = *std::max_element(allNumberOfChannels.begin(), allNumberOfChannels.end());

    WavFile outputWav(maxNumberOfChannels, sampleRate, maxBitDepth);

    Tools::mix(inputWavs, outputWav);
    outputWav.save(output);

    return 0;
}

void Tools::mix(std::vector<WavFile>& inputs, WavFile& output)
{  
    std::vector<size_t> allNumberOfSamples;
    std::vector<std::vector<double>> allSamples;
    for (auto& input : inputs)
    {
        std::vector<double> samples;
        if (input.getNumberOfChannels() == 2)
        {
            samples = input.getSamples();
        }
        if (input.getNumberOfChannels() == 1)
        {
            //Convert mono samples into stereo.
            for (auto& sample : input.getSamples())
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

    if (output.getNumberOfChannels() == 1)
    {
        for (size_t i = 0; i < maxSamples; i++)
        {
            double addedSamples = 0.0;
            for (auto& samples : allSamples)
            {
                double sample = samples.size() <= maxSamples ? samples[i] : 0.0;
                addedSamples += sample / static_cast<double>(numberOfInputs);
            }
            newSamples.push_back(addedSamples);
        }
    }
    else if (output.getNumberOfChannels() == 2)
    {
        for (size_t i = 0; i < maxSamples - 1; i = i + 2)
        {
            double addedSamplesLeft = 0.0;
            double addedSamplesRight = 0.0;
            for (auto& samples : allSamples)
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

void Commands::splitHelp()
{
    std::cout << "split <input> <splitTimeMs...>" << std::endl;
    std::cout << "\tdescription: splits a wave file at the specified times in milliseconds. Output files will be named {original}_1.wav, {original}_2.wav etc" << std::endl;
    std::cout << "\tparameters:" << std::endl;
    std::cout << "\t\tinput: path to wave file to split" << std::endl;
    std::cout << "\t\tsplitTimeMs: arbitrary number of split times in milliseconds" << std::endl;
}

int Commands::splitCommand(std::string command, int argc, char* argv[])
{
    if (argc < 4)
    {
        std::cout << "Incorrect number of arguments for command 'split'. To see available commands run 'wavlab help'" << std::endl;
        return 1;
    }
    std::string input = static_cast<std::string>(argv[2]);

    WavFile inputWav(input, true);
    std::vector<WavFile> outputWavs;
    std::vector<uint32_t> splitTimesMs;

    WavFile firstOutputWav(inputWav.getNumberOfChannels(), inputWav.getSampleRate(), inputWav.getBitsPerSample());
    outputWavs.push_back(firstOutputWav);

    for (int i = 3; i < argc; i++)
    {
        uint32_t timeInMs = 0;
        if (!tryParse(argv[i], timeInMs))
        {
            std::cout << "Parameter 'splitTimeMs' could not be parsed. Please ensure it is a numeric value. Provided value: " << argv[i] << std::endl;
            return 1;
        }
        splitTimesMs.push_back(timeInMs);

        WavFile subsequentOutputWav(inputWav.getNumberOfChannels(), inputWav.getSampleRate(), inputWav.getBitsPerSample());
        outputWavs.push_back(subsequentOutputWav);
    }

    Tools::split(inputWav, outputWavs, splitTimesMs);
    int i = 1;
    for (WavFile output : outputWavs)
    {
        std::string outFileSuffix = "_";
        outFileSuffix += std::to_string(i);
        outFileSuffix += ".wav";
        output.save(StringHelpers::stringReplace(input, ".wav", outFileSuffix));
        i++;
    }

    return 0;
}

void Tools::split(WavFile& input, std::vector<WavFile>& outputs, std::vector<uint32_t> splitTimesMs)
{
    std::vector<int32_t> splitSamplePositions;
    uint32_t sampleRateMultiplier = input.getSampleRate() / 1000;

    for (uint32_t& ms : splitTimesMs)
    {
        splitSamplePositions.push_back(ms * sampleRateMultiplier * input.getNumberOfChannels());
    }

    unsigned int splitPosition = 0;
    uint32_t nextSplit = splitSamplePositions[splitPosition];
    std::vector<double>* currentSamples = new std::vector<double>();

    auto originalSamples = input.getSamples();

    if (input.getNumberOfChannels() == 1)
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
    else if (input.getNumberOfChannels() == 2)
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

void Commands::joinHelp()
{
    std::cout << "join <output> <input...>" << std::endl;
    std::cout << "\tdescription: joins multiple wave files into one." << std::endl;
    std::cout << "\tparameters:" << std::endl;
    std::cout << "\t\toutput: path to mixed wave file" << std::endl;
    std::cout << "\t\tinput: arbitrary number of input files (minimum 2)" << std::endl;
}

int Commands::joinCommand(std::string command, int argc, char* argv[])
{
    if (argc < 5)
    {
        std::cout << "Incorrect number of arguments for command 'join'. To see available commands run 'wavlab help'" << std::endl;
        return 1;
    }
    std::string output = static_cast<std::string>(argv[2]);

    std::vector<WavFile> inputWavs;
    std::vector<unsigned short> allBitDepths;
    std::vector<unsigned short> allNumberOfChannels;
    uint32_t sampleRate = 0;

    for (int i = 3; i < argc; i++)
    {
        std::string input = static_cast<std::string>(argv[i]);
        WavFile inputWav(input, true);

        if (sampleRate != 0)
        {
            if (sampleRate != inputWav.getSampleRate())
            {
                std::cout << "Not all input files are the same sample rate." << std::endl;
                return 1;
            }
        }
        sampleRate = inputWav.getSampleRate();

        allBitDepths.push_back(inputWav.getBitsPerSample());
        allNumberOfChannels.push_back(inputWav.getNumberOfChannels());
        inputWavs.push_back(inputWav);

    }

    unsigned short maxBitDepth = *std::max_element(allBitDepths.begin(), allBitDepths.end());
    unsigned short maxNumberOfChannels = *std::max_element(allNumberOfChannels.begin(), allNumberOfChannels.end());

    WavFile outputWav(maxNumberOfChannels, sampleRate, maxBitDepth);

    Tools::join(inputWavs, outputWav);
    outputWav.save(output);

    return 0;
}

void Tools::join(std::vector<WavFile>& inputs, WavFile& output)
{
    std::vector<double> joinedSamples;
    for (WavFile wav : inputs)
    {
        auto wavSamples = wav.getSamples();
        joinedSamples.insert(joinedSamples.end(), wavSamples.begin(), wavSamples.end());
    }
    output.setSamples(joinedSamples);
}