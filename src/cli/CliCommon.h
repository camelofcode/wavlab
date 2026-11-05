#pragma once

#include "Cli.h"

#include <wavlab/LFO.h>
#include <wavlab/WavFile.h>

#include <filesystem>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <utility>

struct InputOutput
{
    std::filesystem::path input;
    std::optional<std::filesystem::path> output;

    [[nodiscard]] std::filesystem::path outputPath() const { return output.value_or(input); }
};

inline void addInput(CLI::App &command, InputOutput &paths)
{
    command.add_option("INPUT", paths.input, "PCM WAV file to process")
        ->required()
        ->check(CLI::ExistingFile);
}

inline void addOutput(CLI::App &command, InputOutput &paths)
{
    command.add_option("OUTPUT", paths.output,
                       "Destination WAV file; defaults to overwriting INPUT");
}

inline void addInputOutput(CLI::App &command, InputOutput &paths)
{
    addInput(command, paths);
    addOutput(command, paths);
}

template <typename Process> void processFile(const InputOutput &paths, Process process)
{
    auto wave = wavlab::WavFile::open(paths.input);
    process(wave);
    wave.save(paths.outputPath());
}

template <typename Arguments, typename Configure, typename Action>
CLI::App *addCommand(CLI::App &app, CliAction &selectedAction, std::string_view name,
                     std::string_view description, Configure configure, Action action)
{
    auto arguments = std::make_shared<Arguments>();
    auto *command = app.add_subcommand(std::string(name), std::string(description));
    configure(*command, *arguments);
    command->callback([&selectedAction, arguments, action]
                      { selectedAction = [arguments, action] { action(*arguments); }; });
    return command;
}

inline wavlab::LfoWaveform lfoWaveform(std::string_view name)
{
    if (name == "triangle")
    {
        return wavlab::LfoWaveform::triangle;
    }
    if (name == "square")
    {
        return wavlab::LfoWaveform::square;
    }
    return wavlab::LfoWaveform::sine;
}

inline CLI::Validator normalisedValue() { return CLI::Range(0.0, 1.0); }

inline CLI::Validator waveformValue() { return CLI::IsMember({"sine", "triangle", "square"}); }
