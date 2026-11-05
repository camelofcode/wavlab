#include <wavlab/FX.h>
#include <wavlab/WavFile.h>

#include "Commands.h"

#include <cstddef>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <span>
#include <string>
#include <string_view>

namespace
{

bool parseNumbers(char *argv[], std::size_t first, std::span<double> values)
{
    for (std::size_t index = 0; index < values.size(); ++index)
    {
        if (!tryParse(argv[first + index], values[index]))
        {
            std::cerr << "Parameter " << first + index - 2
                      << " must be a numeric value. Provided value: " << argv[first + index]
                      << '\n';
            return false;
        }
    }
    return true;
}

bool parseWaveform(std::string_view value, wavlab::LfoWaveform &waveform)
{
    if (value == "sine")
    {
        waveform = wavlab::LfoWaveform::sine;
    }
    else if (value == "triangle")
    {
        waveform = wavlab::LfoWaveform::triangle;
    }
    else if (value == "square")
    {
        waveform = wavlab::LfoWaveform::square;
    }
    else
    {
        std::cerr << "Waveform must be sine, triangle, or square. Provided value: " << value
                  << '\n';
        return false;
    }
    return true;
}

bool parseFilterType(std::string_view value, wavlab::ModulatedFilterType &filterType)
{
    if (value == "lowpass" || value == "low-pass")
    {
        filterType = wavlab::ModulatedFilterType::lowPass;
    }
    else if (value == "highpass" || value == "high-pass")
    {
        filterType = wavlab::ModulatedFilterType::highPass;
    }
    else
    {
        std::cerr << "Filter type must be low-pass or high-pass. Provided value: " << value << '\n';
        return false;
    }
    return true;
}

template <typename Effect>
int apply(const std::filesystem::path &input, const std::filesystem::path &output, Effect effect)
{
    auto wave = wavlab::WavFile::open(input);
    effect(wave);
    wave.save(output);
    return EXIT_SUCCESS;
}

void printHelp(std::string_view usage, std::string_view description)
{
    std::cout << usage << '\n';
    std::cout << "\tdescription: " << description << '\n';
    std::cout << "\tdepth and mix are normalised values from 0 to 1; frequencies are in Hz\n";
    std::cout << "\toutput is optional and defaults to overwriting the input\n";
}

bool requireArguments(const std::string &command, int argc, int minimum, int maximum)
{
    if (argc >= minimum && argc <= maximum)
    {
        return true;
    }
    Commands::printIncorrectArgumentsWarning(command);
    return false;
}

} // namespace

void Commands::chorusMonoHelp()
{
    printHelp("chorus-mono <input> <depth> <frequencyHz> <mix> [output]",
              "applies common-setting chorus to every input channel");
}

int Commands::chorusMonoCommand(const std::string &command, int argc, char *argv[])
{
    if (!requireArguments(command, argc, 6, 7))
        return EXIT_FAILURE;
    double values[3]{};
    if (!parseNumbers(argv, 3, values))
        return EXIT_FAILURE;
    const std::filesystem::path input(argv[2]);
    const std::filesystem::path output = argc == 7 ? argv[6] : argv[2];
    return apply(
        input, output, [&](wavlab::WavFile &wave)
        { wavlab::FX::chorus(wave, wavlab::ChorusSettings{{values[0], values[1]}, values[2]}); });
}

void Commands::chorusStereoHelp()
{
    printHelp("chorus-stereo <input> <leftDepth> <leftFrequencyHz> <rightDepth> "
              "<rightFrequencyHz> <mix> [output]",
              "applies independent left/right chorus and promotes mono input");
}

int Commands::chorusStereoCommand(const std::string &command, int argc, char *argv[])
{
    if (!requireArguments(command, argc, 8, 9))
        return EXIT_FAILURE;
    double values[5]{};
    if (!parseNumbers(argv, 3, values))
        return EXIT_FAILURE;
    const std::filesystem::path input(argv[2]);
    const std::filesystem::path output = argc == 9 ? argv[8] : argv[2];
    return apply(input, output,
                 [&](wavlab::WavFile &wave)
                 {
                     wavlab::FX::chorus(wave, wavlab::StereoChorusSettings{{values[0], values[1]},
                                                                           {values[2], values[3]},
                                                                           values[4]});
                 });
}

void Commands::vibratoMonoHelp()
{
    printHelp("vibrato-mono <input> <depth> <frequencyHz> [output]",
              "applies fully wet chorus pitch modulation to every input channel");
}

int Commands::vibratoMonoCommand(const std::string &command, int argc, char *argv[])
{
    if (!requireArguments(command, argc, 5, 6))
        return EXIT_FAILURE;
    double values[2]{};
    if (!parseNumbers(argv, 3, values))
        return EXIT_FAILURE;
    const std::filesystem::path input(argv[2]);
    const std::filesystem::path output = argc == 6 ? argv[5] : argv[2];
    return apply(input, output, [&](wavlab::WavFile &wave)
                 { wavlab::FX::vibrato(wave, wavlab::VibratoSettings{{values[0], values[1]}}); });
}

void Commands::vibratoStereoHelp()
{
    printHelp("vibrato-stereo <input> <leftDepth> <leftFrequencyHz> <rightDepth> "
              "<rightFrequencyHz> [output]",
              "applies independent stereo pitch modulation and promotes mono input");
}

int Commands::vibratoStereoCommand(const std::string &command, int argc, char *argv[])
{
    if (!requireArguments(command, argc, 7, 8))
        return EXIT_FAILURE;
    double values[4]{};
    if (!parseNumbers(argv, 3, values))
        return EXIT_FAILURE;
    const std::filesystem::path input(argv[2]);
    const std::filesystem::path output = argc == 8 ? argv[7] : argv[2];
    return apply(input, output,
                 [&](wavlab::WavFile &wave)
                 {
                     wavlab::FX::vibrato(wave, wavlab::StereoVibratoSettings{
                                                   {values[0], values[1]}, {values[2], values[3]}});
                 });
}

void Commands::flangerMonoHelp()
{
    printHelp("flanger-mono <input> <depth> <frequencyHz> <feedback> <mix> [output]",
              "applies triangle-modulated flanging with signed feedback");
}

int Commands::flangerMonoCommand(const std::string &command, int argc, char *argv[])
{
    if (!requireArguments(command, argc, 7, 8))
        return EXIT_FAILURE;
    double values[4]{};
    if (!parseNumbers(argv, 3, values))
        return EXIT_FAILURE;
    const std::filesystem::path input(argv[2]);
    const std::filesystem::path output = argc == 8 ? argv[7] : argv[2];
    return apply(input, output,
                 [&](wavlab::WavFile &wave)
                 {
                     wavlab::FX::flanger(wave, wavlab::FlangerSettings{
                                                   {values[0], values[1], values[2]}, values[3]});
                 });
}

void Commands::flangerStereoHelp()
{
    printHelp("flanger-stereo <input> <leftDepth> <leftFrequencyHz> <leftFeedback> "
              "<rightDepth> <rightFrequencyHz> <rightFeedback> <mix> [output]",
              "applies independent stereo flanging and promotes mono input");
}

int Commands::flangerStereoCommand(const std::string &command, int argc, char *argv[])
{
    if (!requireArguments(command, argc, 10, 11))
        return EXIT_FAILURE;
    double values[7]{};
    if (!parseNumbers(argv, 3, values))
        return EXIT_FAILURE;
    const std::filesystem::path input(argv[2]);
    const std::filesystem::path output = argc == 11 ? argv[10] : argv[2];
    return apply(input, output,
                 [&](wavlab::WavFile &wave)
                 {
                     wavlab::FX::flanger(
                         wave, wavlab::StereoFlangerSettings{{values[0], values[1], values[2]},
                                                             {values[3], values[4], values[5]},
                                                             values[6]});
                 });
}

void Commands::tremoloHelp()
{
    printHelp("tremolo <input> <depth> <frequencyHz> <sine|triangle|square> [output]",
              "applies shared gain modulation while preserving the stereo image");
}

int Commands::tremoloCommand(const std::string &command, int argc, char *argv[])
{
    if (!requireArguments(command, argc, 6, 7))
        return EXIT_FAILURE;
    double values[2]{};
    wavlab::LfoWaveform waveform{};
    if (!parseNumbers(argv, 3, values) || !parseWaveform(argv[5], waveform))
        return EXIT_FAILURE;
    const std::filesystem::path input(argv[2]);
    const std::filesystem::path output = argc == 7 ? argv[6] : argv[2];
    return apply(
        input, output, [&](wavlab::WavFile &wave)
        { wavlab::FX::tremolo(wave, wavlab::TremoloSettings{values[0], values[1], waveform}); });
}

void Commands::autoPanHelp()
{
    printHelp("autopan <input> <depth> <frequencyHz> <sine|triangle|square> [output]",
              "pans mono or stereo input across an always-stereo output");
}

int Commands::autoPanCommand(const std::string &command, int argc, char *argv[])
{
    if (!requireArguments(command, argc, 6, 7))
        return EXIT_FAILURE;
    double values[2]{};
    wavlab::LfoWaveform waveform{};
    if (!parseNumbers(argv, 3, values) || !parseWaveform(argv[5], waveform))
        return EXIT_FAILURE;
    const std::filesystem::path input(argv[2]);
    const std::filesystem::path output = argc == 7 ? argv[6] : argv[2];
    return apply(
        input, output, [&](wavlab::WavFile &wave)
        { wavlab::FX::autoPan(wave, wavlab::AutoPanSettings{values[0], values[1], waveform}); });
}

void Commands::phaserMonoHelp()
{
    printHelp("phaser-mono <input> <depth> <frequencyHz> <centreFrequencyHz> "
              "<waveform> <mix> [output] [--stages N]",
              "applies a configurable cascade of modulated all-pass filters");
}

int Commands::phaserMonoCommand(const std::string &command, int argc, char *argv[])
{
    std::size_t stages = wavlab::FX::defaultPhaserStages;
    int positionalArgc = argc;
    if (argc >= 10 && std::string_view(argv[argc - 2]) == "--stages")
    {
        if (!tryParse(argv[argc - 1], stages))
        {
            std::cerr << "Parameter 'stages' must be a positive integer\n";
            return EXIT_FAILURE;
        }
        positionalArgc -= 2;
    }
    if (!requireArguments(command, positionalArgc, 8, 9))
        return EXIT_FAILURE;
    double values[3]{};
    double mix{};
    wavlab::LfoWaveform waveform{};
    if (!parseNumbers(argv, 3, values) || !parseWaveform(argv[6], waveform) ||
        !tryParse(argv[7], mix))
        return EXIT_FAILURE;
    const std::filesystem::path input(argv[2]);
    const std::filesystem::path output = positionalArgc == 9 ? argv[8] : argv[2];
    return apply(input, output,
                 [&](wavlab::WavFile &wave)
                 {
                     wavlab::FX::phaser(
                         wave, wavlab::PhaserSettings{
                                   {values[0], values[1], values[2], waveform}, stages, mix});
                 });
}

void Commands::phaserStereoHelp()
{
    printHelp("phaser-stereo <input> <leftDepth> <leftFrequencyHz> <leftCentreFrequencyHz> "
              "<leftWaveform> <rightDepth> <rightFrequencyHz> <rightCentreFrequencyHz> "
              "<rightWaveform> <mix> [output] [--stages N]",
              "applies independent stereo phasing and promotes mono input");
}

int Commands::phaserStereoCommand(const std::string &command, int argc, char *argv[])
{
    std::size_t stages = wavlab::FX::defaultPhaserStages;
    int positionalArgc = argc;
    if (argc >= 14 && std::string_view(argv[argc - 2]) == "--stages")
    {
        if (!tryParse(argv[argc - 1], stages))
        {
            std::cerr << "Parameter 'stages' must be a positive integer\n";
            return EXIT_FAILURE;
        }
        positionalArgc -= 2;
    }
    if (!requireArguments(command, positionalArgc, 12, 13))
        return EXIT_FAILURE;
    double left[3]{};
    double right[3]{};
    double mix{};
    wavlab::LfoWaveform leftWaveform{};
    wavlab::LfoWaveform rightWaveform{};
    if (!parseNumbers(argv, 3, left) || !parseWaveform(argv[6], leftWaveform) ||
        !parseNumbers(argv, 7, right) || !parseWaveform(argv[10], rightWaveform) ||
        !tryParse(argv[11], mix))
        return EXIT_FAILURE;
    const std::filesystem::path input(argv[2]);
    const std::filesystem::path output = positionalArgc == 13 ? argv[12] : argv[2];
    return apply(input, output,
                 [&](wavlab::WavFile &wave)
                 {
                     wavlab::FX::phaser(wave, wavlab::StereoPhaserSettings{
                                                  {left[0], left[1], left[2], leftWaveform},
                                                  {right[0], right[1], right[2], rightWaveform},
                                                  stages,
                                                  mix});
                 });
}

void Commands::modulatedFilterHelp()
{
    printHelp("modulated-filter <input> <minimumFrequencyHz> <maximumFrequencyHz> "
              "<frequencyHz> <low-pass|high-pass> <waveform> <q> [output]",
              "applies an LFO-modulated low- or high-pass filter");
}

int Commands::modulatedFilterCommand(const std::string &command, int argc, char *argv[])
{
    if (!requireArguments(command, argc, 9, 10))
        return EXIT_FAILURE;
    double frequencies[3]{};
    double q{};
    wavlab::ModulatedFilterType filterType{};
    wavlab::LfoWaveform waveform{};
    if (!parseNumbers(argv, 3, frequencies) || !parseFilterType(argv[6], filterType) ||
        !parseWaveform(argv[7], waveform) || !tryParse(argv[8], q))
        return EXIT_FAILURE;
    const std::filesystem::path input(argv[2]);
    const std::filesystem::path output = argc == 10 ? argv[9] : argv[2];
    return apply(input, output,
                 [&](wavlab::WavFile &wave)
                 {
                     wavlab::FX::modulatedFilter(
                         wave,
                         wavlab::ModulatedFilterSettings{frequencies[0], frequencies[1],
                                                         frequencies[2], filterType, waveform, q});
                 });
}
