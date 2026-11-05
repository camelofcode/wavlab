#include "CliCommon.h"
#include "StringHelpers.h"

#include <wavlab/Tools.h>
#include <wavlab/WavFile.h>

#include <cstdint>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace
{

template <typename Process>
CLI::App *addFileCommand(CLI::App &app, CliAction &selectedAction, std::string_view name,
                         std::string_view description, Process process)
{
    return addCommand<InputOutput>(
        app, selectedAction, name, description, [](CLI::App &command, InputOutput &arguments)
        { addInputOutput(command, arguments); }, [process](const InputOutput &arguments)
        { processFile(arguments, [&](wavlab::WavFile &wave) { process(wave); }); });
}

struct MetadataArguments
{
    std::filesystem::path input;
    bool full{};
};

struct ValueArguments
{
    InputOutput paths;
    double value{};
};

struct SplitStereoArguments
{
    std::filesystem::path input;
    std::optional<std::filesystem::path> firstOutput;
    std::optional<std::filesystem::path> secondOutput;
};

struct CombineArguments
{
    std::filesystem::path firstInput;
    std::filesystem::path secondInput;
    std::filesystem::path output;
};

struct SplitArguments
{
    std::filesystem::path input;
    std::vector<std::uint64_t> timesMilliseconds;
};

struct MultipleInputArguments
{
    std::filesystem::path output;
    std::vector<std::filesystem::path> inputs;
};

void addPairedSplitOptions(CLI::App &command, SplitStereoArguments &arguments,
                           std::string_view firstName, std::string_view secondName)
{
    command.add_option("INPUT", arguments.input, "Stereo PCM WAV file to split")
        ->required()
        ->check(CLI::ExistingFile);
    auto *first = command.add_option(std::string(firstName), arguments.firstOutput,
                                     "First output; omit both outputs to derive filenames");
    auto *second = command.add_option(std::string(secondName), arguments.secondOutput,
                                      "Second output; omit both outputs to derive filenames");
    first->needs(second);
    second->needs(first);
}

void addCombineOptions(CLI::App &command, CombineArguments &arguments, std::string_view firstName,
                       std::string_view secondName)
{
    command.add_option(std::string(firstName), arguments.firstInput, "First mono input WAV file")
        ->required()
        ->check(CLI::ExistingFile);
    command
        .add_option(std::string(secondName), arguments.secondInput, "Second mono input WAV file")
        ->required()
        ->check(CLI::ExistingFile);
    command.add_option("OUTPUT", arguments.output, "Destination stereo WAV file")->required();
}

std::pair<std::filesystem::path, std::filesystem::path>
splitOutputs(const SplitStereoArguments &arguments, std::string_view firstSuffix,
             std::string_view secondSuffix)
{
    if (arguments.firstOutput && arguments.secondOutput)
    {
        return {*arguments.firstOutput, *arguments.secondOutput};
    }
    return {StringHelpers::pathWithStemSuffix(arguments.input, std::string(firstSuffix)),
            StringHelpers::pathWithStemSuffix(arguments.input, std::string(secondSuffix))};
}

std::vector<wavlab::WavFile> openFiles(const std::vector<std::filesystem::path> &paths)
{
    std::vector<wavlab::WavFile> waves;
    waves.reserve(paths.size());
    for (const auto &path : paths)
    {
        waves.push_back(wavlab::WavFile::open(path));
    }
    return waves;
}

} // namespace

void configureToolCommands(CLI::App &app, CliAction &selectedAction)
{
    auto *metadata = addCommand<MetadataArguments>(
        app, selectedAction, "metadata", "Display WAV metadata and optionally scan levels",
        [](CLI::App &command, MetadataArguments &arguments)
        {
            command.add_option("INPUT", arguments.input, "PCM WAV file to inspect")
                ->required()
                ->check(CLI::ExistingFile);
            command.add_flag("-f,--full", arguments.full, "Include peak and RMS levels");
        },
        [](const MetadataArguments &arguments)
        {
            auto wave = arguments.full ? wavlab::WavFile::open(arguments.input)
                                       : wavlab::WavFile::openMetadata(arguments.input);
            std::cout << wave.details();
            if (arguments.full)
            {
                std::cout << "\n\nLevels\n  Peak: " << std::fixed << std::setprecision(2)
                          << wavlab::Tools::getPeakDecibels(wave)
                          << " dBFS\n  RMS: " << wavlab::Tools::getRMSDecibels(wave) << " dBFS";
            }
            std::cout << '\n';
        });
    metadata->alias("stat");

    auto *normalise =
        addFileCommand(app, selectedAction, "normalise", "Normalise the peak level to 0 dBFS",
                       [](wavlab::WavFile &wave) { wavlab::Tools::normalise(wave); });
    normalise->alias("normalize");

    auto *normaliseDb = addCommand<ValueArguments>(
        app, selectedAction, "normalise-db", "Normalise the peak to a specified level in dBFS",
        [](CLI::App &command, ValueArguments &arguments)
        {
            addInput(command, arguments.paths);
            command.add_option("TARGET_DB", arguments.value, "Target peak level in dBFS")
                ->required();
            addOutput(command, arguments.paths);
        },
        [](const ValueArguments &arguments)
        {
            processFile(arguments.paths, [&](wavlab::WavFile &wave)
                        { wavlab::Tools::normaliseToDecibels(wave, arguments.value); });
        });
    normaliseDb->alias("normalize-db");

    addCommand<ValueArguments>(
        app, selectedAction, "gain", "Multiply every sample by a linear gain factor",
        [](CLI::App &command, ValueArguments &arguments)
        {
            addInput(command, arguments.paths);
            command.add_option("GAIN", arguments.value, "Linear gain factor")->required();
            addOutput(command, arguments.paths);
        },
        [](const ValueArguments &arguments)
        {
            processFile(arguments.paths,
                        [&](wavlab::WavFile &wave) { wavlab::Tools::gain(wave, arguments.value); });
        });

    addCommand<ValueArguments>(
        app, selectedAction, "gain-db", "Apply amplitude gain measured in decibels",
        [](CLI::App &command, ValueArguments &arguments)
        {
            addInput(command, arguments.paths);
            command.add_option("GAIN_DB", arguments.value, "Gain in decibels")->required();
            addOutput(command, arguments.paths);
        },
        [](const ValueArguments &arguments)
        {
            processFile(arguments.paths, [&](wavlab::WavFile &wave)
                        { wavlab::Tools::decibelGain(wave, arguments.value); });
        });

    addFileCommand(app, selectedAction, "reverse",
                   "Reverse audio frames while retaining channel order",
                   [](wavlab::WavFile &wave) { wavlab::Tools::reverse(wave); });
    addFileCommand(app, selectedAction, "flip-phase", "Invert the polarity of every sample",
                   [](wavlab::WavFile &wave) { wavlab::Tools::flipPhase(wave); });
    addFileCommand(app, selectedAction, "swap-channels", "Swap stereo left and right channels",
                   [](wavlab::WavFile &wave) { wavlab::Tools::swapChannels(wave); });

    addCommand<SplitStereoArguments>(
        app, selectedAction, "split-stereo", "Split stereo into separate mono left/right files",
        [](CLI::App &command, SplitStereoArguments &arguments)
        { addPairedSplitOptions(command, arguments, "LEFT_OUTPUT", "RIGHT_OUTPUT"); },
        [](const SplitStereoArguments &arguments)
        {
            const auto wave = wavlab::WavFile::open(arguments.input);
            auto [left, right] = wavlab::Tools::splitStereo(wave);
            const auto [leftOutput, rightOutput] = splitOutputs(arguments, "_l", "_r");
            left.save(leftOutput);
            right.save(rightOutput);
        });

    addCommand<SplitStereoArguments>(
        app, selectedAction, "split-mid-side", "Split stereo into separate mono mid/side files",
        [](CLI::App &command, SplitStereoArguments &arguments)
        { addPairedSplitOptions(command, arguments, "MID_OUTPUT", "SIDE_OUTPUT"); },
        [](const SplitStereoArguments &arguments)
        {
            const auto wave = wavlab::WavFile::open(arguments.input);
            auto [mid, side] = wavlab::Tools::splitStereoMidSide(wave);
            const auto [midOutput, sideOutput] = splitOutputs(arguments, "_mid", "_side");
            mid.save(midOutput);
            side.save(sideOutput);
        });

    addCommand<CombineArguments>(
        app, selectedAction, "left-right-to-stereo", "Combine mono left/right files into stereo",
        [](CLI::App &command, CombineArguments &arguments)
        { addCombineOptions(command, arguments, "LEFT_INPUT", "RIGHT_INPUT"); },
        [](const CombineArguments &arguments)
        {
            const auto left = wavlab::WavFile::open(arguments.firstInput);
            const auto right = wavlab::WavFile::open(arguments.secondInput);
            auto output = wavlab::Tools::combineLeftRight(left, right);
            output.save(arguments.output);
        });

    addCommand<CombineArguments>(
        app, selectedAction, "mid-side-to-stereo", "Decode mono mid/side files into stereo",
        [](CLI::App &command, CombineArguments &arguments)
        { addCombineOptions(command, arguments, "MID_INPUT", "SIDE_INPUT"); },
        [](const CombineArguments &arguments)
        {
            const auto mid = wavlab::WavFile::open(arguments.firstInput);
            const auto side = wavlab::WavFile::open(arguments.secondInput);
            auto output = wavlab::Tools::combineMidSide(mid, side);
            output.save(arguments.output);
        });

    addCommand<SplitArguments>(
        app, selectedAction, "split", "Split a WAV file at one or more millisecond positions",
        [](CLI::App &command, SplitArguments &arguments)
        {
            command.add_option("INPUT", arguments.input, "PCM WAV file to split")
                ->required()
                ->check(CLI::ExistingFile);
            command
                .add_option("SPLIT_TIME_MS", arguments.timesMilliseconds,
                            "Increasing split positions in milliseconds")
                ->required()
                ->expected(1, -1);
        },
        [](const SplitArguments &arguments)
        {
            const auto input = wavlab::WavFile::open(arguments.input);
            auto outputs = wavlab::Tools::split(input, arguments.timesMilliseconds);
            for (std::size_t index = 0; index < outputs.size(); ++index)
            {
                outputs[index].save(StringHelpers::pathWithStemSuffix(
                    arguments.input, "_" + std::to_string(index + 1)));
            }
        });

    const auto addMultipleInput =
        [&](std::string_view name, std::string_view description, auto process)
    {
        addCommand<MultipleInputArguments>(
            app, selectedAction, name, description,
            [](CLI::App &command, MultipleInputArguments &arguments)
            {
                command.add_option("OUTPUT", arguments.output, "Destination WAV file")->required();
                command.add_option("INPUT", arguments.inputs, "Two or more input WAV files")
                    ->required()
                    ->expected(2, -1)
                    ->check(CLI::ExistingFile);
            },
            [process](const MultipleInputArguments &arguments)
            {
                const auto inputs = openFiles(arguments.inputs);
                auto output = process(inputs);
                output.save(arguments.output);
            });
    };
    addMultipleInput("mix", "Mix two or more WAV files from the first frame",
                     [](const std::vector<wavlab::WavFile> &inputs)
                     { return wavlab::Tools::mix(inputs); });
    addMultipleInput("join", "Join two or more compatible WAV files end-to-end",
                     [](const std::vector<wavlab::WavFile> &inputs)
                     { return wavlab::Tools::join(inputs); });

    struct LevelArguments
    {
        std::filesystem::path input;
    };
    const auto addLevelCommand =
        [&](std::string_view name, std::string_view description, auto measure)
    {
        addCommand<LevelArguments>(
            app, selectedAction, name, description,
            [](CLI::App &command, LevelArguments &arguments)
            {
                command.add_option("INPUT", arguments.input, "PCM WAV file to scan")
                    ->required()
                    ->check(CLI::ExistingFile);
            },
            [measure](const LevelArguments &arguments)
            {
                const auto wave = wavlab::WavFile::open(arguments.input);
                std::cout << measure(wave) << " dBFS\n";
            });
    };
    addLevelCommand("get-peak-db", "Compatibility command: print peak level in dBFS",
                    [](const wavlab::WavFile &wave)
                    { return wavlab::Tools::getPeakDecibels(wave); });
    addLevelCommand("get-rms-db", "Compatibility command: print RMS level in dBFS",
                    [](const wavlab::WavFile &wave)
                    { return wavlab::Tools::getRMSDecibels(wave); });
}
