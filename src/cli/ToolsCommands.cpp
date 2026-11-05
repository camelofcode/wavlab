#include "Commands.h"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <wavlab/Tools.h>

using namespace wavlab;

/* stat command */

void Commands::statHelp()
{
    std::cout << "stat <input>" << std::endl;
    std::cout << "\tdescription: show stats of a wave file" << std::endl;
    std::cout << "\tparameters:" << std::endl;
    std::cout << "\t\tinput: path to wave file" << std::endl;
}

int Commands::statCommand(const std::string &command, int argc, char *argv[])
{
    if (argc != 3)
    {
        Commands::printIncorrectArgumentsWarning(command);
        return EXIT_FAILURE;
    }
    std::string path = static_cast<std::string>(argv[2]);
    auto wav = WavFile::openMetadata(path);
    std::cout << wav.details() << std::endl;
    return EXIT_SUCCESS;
}

/* normalise command */

void Commands::normaliseHelp()
{
    std::cout << "normalise <input> [output]" << std::endl;
    std::cout << "\tdescription: normalises a wave file to 0 dBFS" << std::endl;
    std::cout << "\tparameters:" << std::endl;
    std::cout << "\t\tinput: path to wave file" << std::endl;
    std::cout
        << "\t\toutput: path to output file. Optional, if not specified input will be overwritten"
        << std::endl;
}

int Commands::normaliseCommand(const std::string &command, int argc, char *argv[])
{
    if (argc != 3 && argc != 4)
    {
        Commands::printIncorrectArgumentsWarning(command);
        return EXIT_FAILURE;
    }
    const std::string input = argv[2];
    const std::string output = argc == 4 ? argv[3] : input;

    auto wav = WavFile::open(input);
    Tools::normalise(wav);
    wav.save(output);
    return EXIT_SUCCESS;
}

void Commands::normaliseDbHelp()
{
    std::cout << "normalise-db <input> <dBFS> [output]" << std::endl;
    std::cout << "\tdescription: normalises a wave file to a target peak level in dBFS"
              << std::endl;
    std::cout << "\tparameters:" << std::endl;
    std::cout << "\t\tinput: path to wave file" << std::endl;
    std::cout << "\t\tdBFS: target peak level in decibels relative to full scale" << std::endl;
    std::cout
        << "\t\toutput: path to output file. Optional, if not specified input will be overwritten"
        << std::endl;
}

int Commands::normaliseDbCommand(const std::string &command, int argc, char *argv[])
{
    if (argc != 4 && argc != 5)
    {
        Commands::printIncorrectArgumentsWarning(command);
        return EXIT_FAILURE;
    }

    double targetDecibels = 0.0;
    if (!tryParse(argv[3], targetDecibels))
    {
        std::cerr << "Parameter 'dBFS' could not be parsed. Please ensure it is a numeric value. "
                     "Provided value: "
                  << argv[3] << std::endl;
        return EXIT_FAILURE;
    }

    const std::string input = argv[2];
    const std::string output = argc == 5 ? argv[4] : input;
    auto wav = WavFile::open(input);
    Tools::normaliseToDecibels(wav, targetDecibels);
    wav.save(output);
    return EXIT_SUCCESS;
}

/* Rust-compatible US-English aliases */

void Commands::normalizeHelp()
{
    std::cout << "normalize <input> [output]" << std::endl;
    std::cout << "\tdescription: US-English alias for normalise" << std::endl;
    std::cout << "\tparameters:" << std::endl;
    std::cout << "\t\tinput: path to wave file" << std::endl;
    std::cout
        << "\t\toutput: path to output file. Optional, if not specified input will be overwritten"
        << std::endl;
}

int Commands::normalizeCommand(const std::string &command, int argc, char *argv[])
{
    return normaliseCommand(command, argc, argv);
}

void Commands::normalizeDbHelp()
{
    std::cout << "normalize-db <input> <dBFS> [output]" << std::endl;
    std::cout << "\tdescription: US-English alias for normalise-db" << std::endl;
    std::cout << "\tparameters:" << std::endl;
    std::cout << "\t\tinput: path to wave file" << std::endl;
    std::cout << "\t\tdBFS: target peak level in decibels relative to full scale" << std::endl;
    std::cout
        << "\t\toutput: path to output file. Optional, if not specified input will be overwritten"
        << std::endl;
}

int Commands::normalizeDbCommand(const std::string &command, int argc, char *argv[])
{
    return normaliseDbCommand(command, argc, argv);
}

void Commands::reverseHelp()
{
    std::cout << "reverse <input> [output]" << std::endl;
    std::cout << "\tdescription: reverses a wave file" << std::endl;
    std::cout << "\tparameters:" << std::endl;
    std::cout << "\t\tinput: path to wave file" << std::endl;
    std::cout
        << "\t\toutput: path to output file. Optional, if not specified input will be overwritten"
        << std::endl;
}

int Commands::reverseCommand(const std::string &command, int argc, char *argv[])
{
    if (argc != 3 && argc != 4)
    {
        Commands::printIncorrectArgumentsWarning(command);
        return EXIT_FAILURE;
    }
    std::string input = static_cast<std::string>(argv[2]);
    std::string output = argc == 4 ? static_cast<std::string>(argv[3]) : input;

    auto wav = WavFile::open(input);
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
    std::cout
        << "\t\toutput: path to output file. Optional, if not specified input will be overwritten"
        << std::endl;
}

int Commands::flipPhaseCommand(const std::string &command, int argc, char *argv[])
{
    if (argc != 3 && argc != 4)
    {
        Commands::printIncorrectArgumentsWarning(command);
        return EXIT_FAILURE;
    }
    std::string input = static_cast<std::string>(argv[2]);
    std::string output = argc == 4 ? static_cast<std::string>(argv[3]) : input;

    auto wav = WavFile::open(input);
    Tools::flipPhase(wav);
    wav.save(output);
    return EXIT_SUCCESS;
}

void Commands::swapChannelsHelp()
{
    std::cout << "swap-channels <input> [output]" << std::endl;
    std::cout << "\tdescription: swaps the left and right channels of a stereo wave file"
              << std::endl;
    std::cout << "\tparameters:" << std::endl;
    std::cout << "\t\tinput: path to wave file" << std::endl;
    std::cout
        << "\t\toutput: path to output file. Optional, if not specified input will be overwritten"
        << std::endl;
}

int Commands::swapChannelsCommand(const std::string &command, int argc, char *argv[])
{
    if (argc != 3 && argc != 4)
    {
        Commands::printIncorrectArgumentsWarning(command);
        return EXIT_FAILURE;
    }
    std::string input = static_cast<std::string>(argv[2]);
    std::string output = argc == 4 ? static_cast<std::string>(argv[3]) : input;

    auto wav = WavFile::open(input);
    Tools::swapChannels(wav);
    wav.save(output);
    return EXIT_SUCCESS;
}

void Commands::splitStereoHelp()
{
    std::cout << "split-stereo <input> [leftOutput rightOutput]" << std::endl;
    std::cout << "\tdescription: splits a stereo wave file to two mono files, {original}_l.wav and "
                 "{original}_r.wav"
              << std::endl;
    std::cout << "\tparameters:" << std::endl;
    std::cout << "\t\tinput: path to wave file" << std::endl;
    std::cout
        << "\t\tleftOutput/rightOutput: optional explicit destinations; provide both or neither"
        << std::endl;
}

int Commands::splitStereoCommand(const std::string &command, int argc, char *argv[])
{
    if (argc != 3 && argc != 5)
    {
        Commands::printIncorrectArgumentsWarning(command);
        return EXIT_FAILURE;
    }
    std::string input = static_cast<std::string>(argv[2]);

    auto stereo = WavFile::open(input);

    auto [left, right] = Tools::splitStereo(stereo);
    const auto leftOutput =
        argc == 5 ? std::filesystem::path(argv[3]) : StringHelpers::pathWithStemSuffix(input, "_l");
    const auto rightOutput =
        argc == 5 ? std::filesystem::path(argv[4]) : StringHelpers::pathWithStemSuffix(input, "_r");
    left.save(leftOutput);
    right.save(rightOutput);

    return EXIT_SUCCESS;
}

void Commands::combineLeftRightHelp()
{
    std::cout << "left-right-to-stereo <leftInput> <rightInput> <output>" << std::endl;
    std::cout
        << "\tdescription: converts two left/right mono wave files to a combined stereo wave file"
        << std::endl;
    std::cout << "\tparameters:" << std::endl;
    std::cout << "\t\tleftInput: path to left wave file" << std::endl;
    std::cout << "\t\trightInput: path to right wave file" << std::endl;
    std::cout << "\t\toutput: path to combined output file" << std::endl;
}

int Commands::combineLeftRightCommand(const std::string &command, int argc, char *argv[])
{
    if (argc != 5)
    {
        Commands::printIncorrectArgumentsWarning(command);
        return EXIT_FAILURE;
    }
    std::string leftInput = static_cast<std::string>(argv[2]);
    std::string rightInput = static_cast<std::string>(argv[3]);
    std::string output = static_cast<std::string>(argv[4]);

    auto left = WavFile::open(leftInput);
    auto right = WavFile::open(rightInput);

    auto stereo = Tools::combineLeftRight(left, right);
    stereo.save(output);

    return EXIT_SUCCESS;
}

void Commands::splitStereoMidSideHelp()
{
    std::cout << "split-mid-side <input> [midOutput sideOutput]" << std::endl;
    std::cout << "\tdescription: splits a stereo wave file to mid and side components, "
                 "{original}_mid.wav and {original}_side.wav"
              << std::endl;
    std::cout << "\tparameters:" << std::endl;
    std::cout << "\t\tinput: path to wave file" << std::endl;
    std::cout << "\t\tmidOutput/sideOutput: optional explicit destinations; provide both or neither"
              << std::endl;
}

int Commands::splitStereoMidSideCommand(const std::string &command, int argc, char *argv[])
{
    if (argc != 3 && argc != 5)
    {
        Commands::printIncorrectArgumentsWarning(command);
        return EXIT_FAILURE;
    }
    std::string input = static_cast<std::string>(argv[2]);

    auto stereo = WavFile::open(input);

    auto [mid, side] = Tools::splitStereoMidSide(stereo);
    const auto midOutput = argc == 5 ? std::filesystem::path(argv[3])
                                     : StringHelpers::pathWithStemSuffix(input, "_mid");
    const auto sideOutput = argc == 5 ? std::filesystem::path(argv[4])
                                      : StringHelpers::pathWithStemSuffix(input, "_side");
    mid.save(midOutput);
    side.save(sideOutput);

    return EXIT_SUCCESS;
}

void Commands::combineMidSideHelp()
{
    std::cout << "mid-side-to-stereo <midInput> <sideInput> <output>" << std::endl;
    std::cout
        << "\tdescription: converts two mid/side mono wave files to a combined stereo wave file"
        << std::endl;
    std::cout << "\tparameters:" << std::endl;
    std::cout << "\t\tmidInput: path to mid wave file" << std::endl;
    std::cout << "\t\tsideInput: path to side wave file" << std::endl;
    std::cout << "\t\toutput: path to combined output file" << std::endl;
}

int Commands::combineMidSideCommand(const std::string &command, int argc, char *argv[])
{
    if (argc != 5)
    {
        Commands::printIncorrectArgumentsWarning(command);
        return EXIT_FAILURE;
    }
    std::string midInput = static_cast<std::string>(argv[2]);
    std::string sideInput = static_cast<std::string>(argv[3]);
    std::string output = static_cast<std::string>(argv[4]);

    auto mid = WavFile::open(midInput);
    auto side = WavFile::open(sideInput);

    auto stereo = Tools::combineMidSide(mid, side);
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
    std::cout
        << "\t\toutput: path to output file. Optional, if not specified input will be overwritten"
        << std::endl;
}

int Commands::gainCommand(const std::string &command, int argc, char *argv[])
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
        std::cerr << "Parameter 'gain' could not be parsed. Please ensure it is a numeric value. "
                     "Provided value: "
                  << argv[3] << std::endl;
        return EXIT_FAILURE;
    }

    std::string output = argc == 5 ? static_cast<std::string>(argv[4]) : input;

    auto wav = WavFile::open(input);
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
    std::cout
        << "\t\toutput: path to output file. Optional, if not specified input will be overwritten"
        << std::endl;
}

int Commands::gainDbCommand(const std::string &command, int argc, char *argv[])
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
        std::cerr << "Parameter 'db' could not be parsed. Please ensure it is a numeric value. "
                     "Provided value: "
                  << argv[3] << std::endl;
        return EXIT_FAILURE;
    }

    std::string output = argc == 5 ? static_cast<std::string>(argv[4]) : input;

    auto wav = WavFile::open(input);
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

int Commands::getPeakDbCommand(const std::string &command, int argc, char *argv[])
{
    if (argc != 3)
    {
        Commands::printIncorrectArgumentsWarning(command);
        return EXIT_FAILURE;
    }
    std::string input = static_cast<std::string>(argv[2]);

    auto wav = WavFile::open(input);
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

int Commands::getRMSDbCommand(const std::string &command, int argc, char *argv[])
{
    if (argc != 3)
    {
        Commands::printIncorrectArgumentsWarning(command);
        return EXIT_FAILURE;
    }
    std::string input = static_cast<std::string>(argv[2]);

    auto wav = WavFile::open(input);
    std::cout << Tools::getRMSDecibels(wav) << "dB";
    return EXIT_SUCCESS;
}

void Commands::mixHelp()
{
    std::cout << "mix <output> <input...>" << std::endl;
    std::cout << "\tdescription: mixes multiple wave files into one. Will attenuate and "
                 "re-normalise to ensure lack of clipping. Currently will start all files at zero."
              << std::endl;
    std::cout << "\tparameters:" << std::endl;
    std::cout << "\t\toutput: path to mixed wave file" << std::endl;
    std::cout << "\t\tinput: arbitrary number of input files (minimum 2)" << std::endl;
}

int Commands::mixCommand(const std::string &command, int argc, char *argv[])
{
    if (argc < 5)
    {
        Commands::printIncorrectArgumentsWarning(command);
        return EXIT_FAILURE;
    }
    std::string output = static_cast<std::string>(argv[2]);

    std::vector<WavFile> inputWavs;
    for (int i = 3; i < argc; i++)
    {
        std::string input = static_cast<std::string>(argv[i]);
        inputWavs.push_back(WavFile::open(input));
    }
    auto outputWav = Tools::mix(inputWavs);
    outputWav.save(output);

    return EXIT_SUCCESS;
}

void Commands::splitHelp()
{
    std::cout << "split <input> <splitTimeMs...>" << std::endl;
    std::cout << "\tdescription: splits a wave file at the specified times in milliseconds. Output "
                 "files will be named {original}_1.wav, {original}_2.wav etc"
              << std::endl;
    std::cout << "\tparameters:" << std::endl;
    std::cout << "\t\tinput: path to wave file to split" << std::endl;
    std::cout << "\t\tsplitTimeMs: arbitrary number of split times in milliseconds" << std::endl;
}

int Commands::splitCommand(const std::string &command, int argc, char *argv[])
{
    if (argc < 4)
    {
        Commands::printIncorrectArgumentsWarning(command);
        return EXIT_FAILURE;
    }
    std::string input = static_cast<std::string>(argv[2]);

    auto inputWav = WavFile::open(input);
    std::vector<std::uint64_t> splitTimesMs;

    for (int i = 3; i < argc; i++)
    {
        std::uint64_t timeInMs = 0;
        if (!tryParse(argv[i], timeInMs))
        {
            std::cerr << "Parameter 'splitTimeMs' could not be parsed. Please ensure it is a "
                         "numeric value. Provided value: "
                      << argv[i] << std::endl;
            return EXIT_FAILURE;
        }
        splitTimesMs.push_back(timeInMs);
    }

    auto outputWavs = Tools::split(inputWav, splitTimesMs);
    std::size_t index = 1;
    for (auto &output : outputWavs)
    {
        output.save(StringHelpers::pathWithStemSuffix(input, "_" + std::to_string(index)));
        ++index;
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

int Commands::joinCommand(const std::string &command, int argc, char *argv[])
{
    if (argc < 5)
    {
        Commands::printIncorrectArgumentsWarning(command);
        return EXIT_FAILURE;
    }
    std::string output = static_cast<std::string>(argv[2]);

    std::vector<WavFile> inputWavs;
    for (int i = 3; i < argc; i++)
    {
        std::string input = static_cast<std::string>(argv[i]);
        inputWavs.push_back(WavFile::open(input));
    }
    auto outputWav = Tools::join(inputWavs);
    outputWav.save(output);

    return EXIT_SUCCESS;
}
