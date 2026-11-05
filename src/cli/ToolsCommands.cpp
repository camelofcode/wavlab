#include <wavlab/Tools.h>
#include <algorithm>
#include "Commands.h"
#include <cmath>

using namespace wavlab;

/* stat command */

void Commands::statHelp()
{
    std::cout << "stat <input>" << std::endl;
    std::cout << "\tdescription: show stats of a wave file" << std::endl;
    std::cout << "\tparameters:" << std::endl;
    std::cout << "\t\tinput: path to wave file" << std::endl;
}

int Commands::statCommand(const std::string& command, int argc, char* argv[])
{
    if (argc != 3)
    {
        Commands::printIncorrectArgumentsWarning(command);
        return EXIT_FAILURE;
    }
    std::string path = static_cast<std::string>(argv[2]);
    WavFile wav(path, false);
    wav.printDetails();
    return EXIT_SUCCESS;
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

int Commands::normaliseCommand(const std::string& command, int argc, char* argv[])
{
    if (argc != 3 && argc != 4)
    {
        Commands::printIncorrectArgumentsWarning(command);
        return EXIT_FAILURE;
    }
    std::string input = static_cast<std::string>(argv[2]);
    std::string output = argc == 4 ? static_cast<std::string>(argv[3]) : input;

    WavFile wav(input, true);
    Tools::normalise(wav);
    wav.save(output);
    return EXIT_SUCCESS;
}

void Commands::reverseHelp()
{
    std::cout << "reverse <input> [output]" << std::endl;
    std::cout << "\tdescription: reverses a wave file" << std::endl;
    std::cout << "\tparameters:" << std::endl;
    std::cout << "\t\tinput: path to wave file" << std::endl;
    std::cout << "\t\toutput: path to output file. Optional, if not specified input will be overwritten" << std::endl;
}

int Commands::reverseCommand(const std::string& command, int argc, char* argv[])
{
    if (argc != 3 && argc != 4)
    {
        Commands::printIncorrectArgumentsWarning(command);
        return EXIT_FAILURE;
    }
    std::string input = static_cast<std::string>(argv[2]);
    std::string output = argc == 4 ? static_cast<std::string>(argv[3]) : input;

    WavFile wav(input, true);
    Tools::reverse(wav);
    wav.save(output);
    return EXIT_SUCCESS;
}

void Commands::flipPhaseHelp()
{
    std::cout << "flip-phase <input> [output]" << std::endl;
    std::cout << "\tdescription: reverses the phase of a wave file" << std::endl;
    std::cout << "\tparameters:" << std::endl;
    std::cout << "\t\tinput: path to wave file" << std::endl;
    std::cout << "\t\toutput: path to output file. Optional, if not specified input will be overwritten" << std::endl;
}

int Commands::flipPhaseCommand(const std::string& command, int argc, char* argv[])
{
    if (argc != 3 && argc != 4)
    {
        Commands::printIncorrectArgumentsWarning(command);
        return EXIT_FAILURE;
    }
    std::string input = static_cast<std::string>(argv[2]);
    std::string output = argc == 4 ? static_cast<std::string>(argv[3]) : input;

    WavFile wav(input, true);
    Tools::flipPhase(wav);
    wav.save(output);
    return EXIT_SUCCESS;
}

void Commands::swapChannelsHelp() 
{
    std::cout << "swap-channels <input> [output]" << std::endl;
    std::cout << "\tdescription: swaps the left and right channels of a stereo wave file" << std::endl;
    std::cout << "\tparameters:" << std::endl;
    std::cout << "\t\tinput: path to wave file" << std::endl;
    std::cout << "\t\toutput: path to output file. Optional, if not specified input will be overwritten" << std::endl;
}

int Commands::swapChannelsCommand(const std::string& command, int argc, char* argv[])
{
    if (argc != 3 && argc != 4)
    {
        Commands::printIncorrectArgumentsWarning(command);
        return EXIT_FAILURE;
    }
    std::string input = static_cast<std::string>(argv[2]);
    std::string output = argc == 4 ? static_cast<std::string>(argv[3]) : input;

    WavFile wav(input, true);
    Tools::swapChannels(wav);
    wav.save(output);
    return EXIT_SUCCESS;
}

void Commands::splitStereoHelp()
{
    std::cout << "split-stereo <input>" << std::endl;
    std::cout << "\tdescription: splits a stereo wave file to two mono files, {original}_l.wav and {original}_r.wav" << std::endl;
    std::cout << "\tparameters:" << std::endl;
    std::cout << "\t\tinput: path to wave file" << std::endl;
}

int Commands::splitStereoCommand(const std::string& command, int argc, char* argv[])
{
    if (argc != 3)
    {
        Commands::printIncorrectArgumentsWarning(command);
        return EXIT_FAILURE;
    }
    std::string input = static_cast<std::string>(argv[2]);

    WavFile stereo(input, true);

    if (stereo.getNumberOfChannels() != 2)
    {
        std::cerr << "Input file is not stereo." << std::endl;
        return EXIT_FAILURE;
    }

    WavFile left(1, stereo.getSampleRate(), stereo.getBitsPerSample());
    WavFile right(1, stereo.getSampleRate(), stereo.getBitsPerSample());

    Tools::splitStereo(stereo, left, right);

    left.save(StringHelpers::stringReplace(input, ".wav", "_l.wav"));
    right.save(StringHelpers::stringReplace(input, ".wav", "_r.wav"));

    return EXIT_SUCCESS;
}

void Commands::combineLeftRightHelp()
{
    std::cout << "left-right-to-stereo <leftInput> <rightInput> <output>" << std::endl;
    std::cout << "\tdescription: converts two left/right mono wave files to a combined stereo wave file" << std::endl;
    std::cout << "\tparameters:" << std::endl;
    std::cout << "\t\tleftInput: path to left wave file" << std::endl;
    std::cout << "\t\trightInput: path to right wave file" << std::endl;
    std::cout << "\t\toutput: path to combined output file" << std::endl;
}

int Commands::combineLeftRightCommand(const std::string& command, int argc, char* argv[])
{
    if (argc != 5)
    {
        Commands::printIncorrectArgumentsWarning(command);
        return EXIT_FAILURE;
    }
    std::string leftInput = static_cast<std::string>(argv[2]);
    std::string rightInput = static_cast<std::string>(argv[3]);
    std::string output = static_cast<std::string>(argv[4]);

    WavFile left(leftInput, true);
    WavFile right(rightInput, true);

    if (left.getNumberOfChannels() != 1)
    {
        std::cerr << "Left input file is not mono." << std::endl;
        return EXIT_FAILURE;
    }

    if (right.getNumberOfChannels() != 1)
    {
        std::cerr << "Right input file is not mono." << std::endl;
        return EXIT_FAILURE;
    }

    if (left.getSampleRate() != right.getSampleRate())
    {
        std::cerr << "Left and Right input files are not the same sample rate." << std::endl;
        return EXIT_FAILURE;
    }

    auto allSampleRates = std::vector<unsigned short>({ left.getBitsPerSample(), right.getBitsPerSample() });
    unsigned short maxSampleRate = *std::max_element(allSampleRates.begin(), allSampleRates.end());

    WavFile stereo(2, left.getSampleRate(), maxSampleRate);

    Tools::combineLeftRight(left, right, stereo);

    stereo.save(output);

    return EXIT_SUCCESS;
}

void Commands::splitStereoMidSideHelp()
{
    std::cout << "split-mid-side <input>" << std::endl;
    std::cout << "\tdescription: splits a stereo wave file to mid and side components, {original}_mid.wav and {original}_side.wav" << std::endl;
    std::cout << "\tparameters:" << std::endl;
    std::cout << "\t\tinput: path to wave file" << std::endl;
}

int Commands::splitStereoMidSideCommand(const std::string& command, int argc, char* argv[])
{
    if (argc != 3)
    {
        Commands::printIncorrectArgumentsWarning(command);
        return EXIT_FAILURE;
    }
    std::string input = static_cast<std::string>(argv[2]);

    WavFile stereo(input, true);

    if (stereo.getNumberOfChannels() != 2)
    {
        std::cerr << "Input file is not stereo." << std::endl;
        return EXIT_FAILURE;
    }

    WavFile mid(1, stereo.getSampleRate(), stereo.getBitsPerSample());
    WavFile side(1, stereo.getSampleRate(), stereo.getBitsPerSample());

    Tools::splitStereoMidSide(stereo, mid, side);

    mid.save(StringHelpers::stringReplace(input, ".wav", "_mid.wav"));
    side.save(StringHelpers::stringReplace(input, ".wav", "_side.wav"));

    return EXIT_SUCCESS;
}

void Commands::combineMidSideHelp()
{
    std::cout << "mid-side-to-stereo <midInput> <sideInput> <output>" << std::endl;
    std::cout << "\tdescription: converts two mid/side mono wave files to a combined stereo wave file" << std::endl;
    std::cout << "\tparameters:" << std::endl;
    std::cout << "\t\tmidInput: path to mid wave file" << std::endl;
    std::cout << "\t\tsideInput: path to side wave file" << std::endl;
    std::cout << "\t\toutput: path to combined output file" << std::endl;
}

int Commands::combineMidSideCommand(const std::string& command, int argc, char* argv[])
{
    if (argc != 5)
    {
        Commands::printIncorrectArgumentsWarning(command);
        return EXIT_FAILURE;
    }
    std::string midInput = static_cast<std::string>(argv[2]);
    std::string sideInput = static_cast<std::string>(argv[3]);
    std::string output = static_cast<std::string>(argv[4]);

    WavFile mid(midInput, true);
    WavFile side(sideInput, true);

    if (mid.getNumberOfChannels() != 1)
    {
        std::cerr << "Mid input file is not mono." << std::endl;
        return EXIT_FAILURE;
    }

    if (side.getNumberOfChannels() != 1)
    {
        std::cerr << "Side input file is not mono." << std::endl;
        return EXIT_FAILURE;
    }

    if (side.getSampleRate() != mid.getSampleRate())
    {
        std::cerr << "Mid and Side input files are not the same sample rate." << std::endl;
        return EXIT_FAILURE;
    }

    auto allSampleRates = std::vector<unsigned short>({ mid.getBitsPerSample(), side.getBitsPerSample() });
    unsigned short maxSampleRate = *std::max_element(allSampleRates.begin(), allSampleRates.end());

    WavFile stereo(2, side.getSampleRate(), maxSampleRate);

    Tools::combineMidSide(mid, side, stereo);

    stereo.save(output);

    return EXIT_SUCCESS;
}

void Commands::gainHelp()
{
    std::cout << "gain <input> <gain> [output]" << std::endl;
    std::cout << "\tdescription: applies gain to a wave file" << std::endl;
    std::cout << "\tparameters:" << std::endl;
    std::cout << "\t\tinput: path to wave file" << std::endl;
    std::cout << "\t\tgain: gain multiplier" << std::endl;
    std::cout << "\t\toutput: path to output file. Optional, if not specified input will be overwritten" << std::endl;
}

int Commands::gainCommand(const std::string& command, int argc, char* argv[])
{
    if (argc != 4 && argc != 5)
    {
        Commands::printIncorrectArgumentsWarning(command);
        return EXIT_FAILURE;
    }
    std::string input = static_cast<std::string>(argv[2]);

    double gain = 0.0;
    if (!tryParse(argv[3], gain))
    {
        std::cerr << "Parameter 'gain' could not be parsed. Please ensure it is a numeric value. Provided value: " << argv[3] << std::endl;
        return EXIT_FAILURE;
    }

    std::string output = argc == 5 ? static_cast<std::string>(argv[4]) : input;

    WavFile wav(input, true);
    Tools::gain(wav, gain);
    wav.save(output);
    return EXIT_SUCCESS;
}

void Commands::gainDbHelp()
{
    std::cout << "gain-db <input> <dB> [output]" << std::endl;
    std::cout << "\tdescription: applies gain to a wave file, with input in decibels" << std::endl;
    std::cout << "\tparameters:" << std::endl;
    std::cout << "\t\tinput: path to wave file" << std::endl;
    std::cout << "\t\tdB: gain to apply in decibels" << std::endl;
    std::cout << "\t\toutput: path to output file. Optional, if not specified input will be overwritten" << std::endl;
}

int Commands::gainDbCommand(const std::string& command, int argc, char* argv[])
{
    if (argc != 4 && argc != 5)
    {
        Commands::printIncorrectArgumentsWarning(command);
        return EXIT_FAILURE;
    }
    std::string input = static_cast<std::string>(argv[2]);

    double dB = 0.0;
    if (!tryParse(argv[3], dB))
    {
        std::cerr << "Parameter 'db' could not be parsed. Please ensure it is a numeric value. Provided value: " << argv[3] << std::endl;
        return EXIT_FAILURE;
    }

    std::string output = argc == 5 ? static_cast<std::string>(argv[4]) : input;

    WavFile wav(input, true);
    Tools::decibelGain(wav, dB);
    wav.save(output);
    return EXIT_SUCCESS;
}

void Commands::getPeakDbHelp()
{
    std::cout << "get-peak-db <input>" << std::endl;
    std::cout << "\tdescription: shows the peak dB level of a wave file" << std::endl;
    std::cout << "\tparameters:" << std::endl;
    std::cout << "\t\tinput: path to wave file" << std::endl;
}

int Commands::getPeakDbCommand(const std::string& command, int argc, char* argv[])
{
    if (argc != 3)
    {
        Commands::printIncorrectArgumentsWarning(command);
        return EXIT_FAILURE;
    }
    std::string input = static_cast<std::string>(argv[2]);

    WavFile wav(input, true);
    std::cout << Tools::getPeakDecibels(wav) << "dB";
    return EXIT_SUCCESS;
}

void Commands::getRMSDbHelp()
{
    std::cout << "get-rms-db <input>" << std::endl;
    std::cout << "\tdescription: shows the RMS dB level of a wave file" << std::endl;
    std::cout << "\tparameters:" << std::endl;
    std::cout << "\t\tinput: path to wave file" << std::endl;
}

int Commands::getRMSDbCommand(const std::string& command, int argc, char* argv[])
{
    if (argc != 3)
    {
        Commands::printIncorrectArgumentsWarning(command);
        return EXIT_FAILURE;
    }
    std::string input = static_cast<std::string>(argv[2]);

    WavFile wav(input, true);
    std::cout << Tools::getRMSDecibels(wav) << "dB";
    return EXIT_SUCCESS;
}

void Commands::mixHelp()
{
    std::cout << "mix <output> <input...>" << std::endl;
    std::cout << "\tdescription: mixes multiple wave files into one. Will attenuate and re-normalise to ensure lack of clipping. Currently will start all files at zero." << std::endl;
    std::cout << "\tparameters:" << std::endl;
    std::cout << "\t\toutput: path to mixed wave file" << std::endl;
    std::cout << "\t\tinput: arbitrary number of input files (minimum 2)" << std::endl;
}

int Commands::mixCommand(const std::string& command, int argc, char* argv[])
{
    if (argc < 5)
    {
        Commands::printIncorrectArgumentsWarning(command);
        return EXIT_FAILURE;
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
                return EXIT_FAILURE;
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

    return EXIT_SUCCESS;
}

void Commands::splitHelp()
{
    std::cout << "split <input> <splitTimeMs...>" << std::endl;
    std::cout << "\tdescription: splits a wave file at the specified times in milliseconds. Output files will be named {original}_1.wav, {original}_2.wav etc" << std::endl;
    std::cout << "\tparameters:" << std::endl;
    std::cout << "\t\tinput: path to wave file to split" << std::endl;
    std::cout << "\t\tsplitTimeMs: arbitrary number of split times in milliseconds" << std::endl;
}

int Commands::splitCommand(const std::string& command, int argc, char* argv[])
{
    if (argc < 4)
    {
        Commands::printIncorrectArgumentsWarning(command);
        return EXIT_FAILURE;
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
            std::cerr << "Parameter 'splitTimeMs' could not be parsed. Please ensure it is a numeric value. Provided value: " << argv[i] << std::endl;
            return EXIT_FAILURE;
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

    return EXIT_SUCCESS;
}

void Commands::joinHelp()
{
    std::cout << "join <output> <input...>" << std::endl;
    std::cout << "\tdescription: joins multiple wave files into one." << std::endl;
    std::cout << "\tparameters:" << std::endl;
    std::cout << "\t\toutput: path to mixed wave file" << std::endl;
    std::cout << "\t\tinput: arbitrary number of input files (minimum 2)" << std::endl;
}

int Commands::joinCommand(const std::string&command, int argc, char* argv[])
{
    if (argc < 5)
    {
        Commands::printIncorrectArgumentsWarning(command);
        return EXIT_FAILURE;
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
                std::cerr << "Not all input files are the same sample rate." << std::endl;
                return EXIT_FAILURE;
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

    return EXIT_SUCCESS;
}

