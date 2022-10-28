#include "Tools.h"
#include <algorithm>
#include "Commands.h"
#include "Constants.h"

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
        double left = original[i];
        double right = original[i + 1];

        leftSamples.push_back(left);
        rightSamples.push_back(right);
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

    unsigned short maxSampleRate = left.getBitsPerSample() >= right.getBitsPerSample() ? left.getBitsPerSample() : right.getBitsPerSample();

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

    size_t maxNumberOfSamples = leftSamples.size() >= rightSamples.size() ? leftSamples.size() : rightSamples.size();

    for (size_t i = 0; i < maxNumberOfSamples - 1; i = i++)
    {
        double left = i < leftSamples.size() - 1 ? leftSamples[i] : 0.0;
        double right = i < rightSamples.size() - 1 ? rightSamples[i] : 0.0;

        stereoSamples.push_back(left);
        stereoSamples.push_back(right);
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

    unsigned short maxSampleRate = side.getBitsPerSample() >= mid.getBitsPerSample() ? side.getBitsPerSample() : mid.getBitsPerSample();

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

    size_t maxNumberOfSamples = midSamples.size() >= sideSamples.size() ? midSamples.size() : sideSamples.size();

    for (size_t i = 0; i < maxNumberOfSamples - 1; i = i++)
    {
        double mid = i < midSamples.size() - 1 ? midSamples[i] : 0.0;
        double side = i < sideSamples.size() - 1 ? sideSamples[i] : 0.0;

        stereoSamples.push_back((mid + side) * Constants::MINUS_THREE_DB);
        stereoSamples.push_back((mid - side) * Constants::MINUS_THREE_DB);
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
