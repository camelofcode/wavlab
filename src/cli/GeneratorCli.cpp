#include "CliCommon.h"

#include <wavlab/Generators.h>
#include <wavlab/WavFile.h>

#include <cstdint>
#include <filesystem>
#include <string_view>

namespace
{

struct WaveArguments
{
    std::filesystem::path output;
    double frequency{};
    std::uint32_t sampleRate{};
    std::uint16_t bitDepth{};
    std::uint64_t lengthMilliseconds{};
    std::uint32_t harmonics{1'000};
};

struct NoiseArguments
{
    std::filesystem::path output;
    std::uint32_t sampleRate{};
    std::uint16_t bitDepth{};
    std::uint64_t lengthMilliseconds{};
};

void addWaveArguments(CLI::App &command, WaveArguments &arguments, bool additive)
{
    command.add_option("OUTPUT", arguments.output, "Destination WAV file")->required();
    command.add_option("FREQUENCY", arguments.frequency, "Frequency in Hz")->required();
    command.add_option("SAMPLE_RATE", arguments.sampleRate, "Output sample rate in Hz")->required();
    command.add_option("BIT_DEPTH", arguments.bitDepth, "Integer PCM depth: 8, 16, 24, or 32")
        ->required()
        ->check(CLI::IsMember({8, 16, 24, 32}));
    command.add_option("LENGTH_MS", arguments.lengthMilliseconds, "Output length in milliseconds")
        ->required();
    if (additive)
    {
        command
            .add_option("HARMONICS", arguments.harmonics,
                        "Maximum harmonics; frequencies at or above Nyquist are omitted")
            ->default_val(1'000);
    }
}

template <typename Generate> void generateWave(const WaveArguments &arguments, Generate generate)
{
    auto wave = wavlab::WavFile::create(1, arguments.sampleRate, arguments.bitDepth);
    generate(wave);
    wave.save(arguments.output);
}

template <typename Generate>
void addWaveCommand(CLI::App &app, CliAction &selectedAction, std::string_view name,
                    std::string_view description, bool additive, Generate generate)
{
    addCommand<WaveArguments>(
        app, selectedAction, name, description,
        [additive](CLI::App &command, WaveArguments &arguments)
        { addWaveArguments(command, arguments, additive); },
        [generate](const WaveArguments &arguments)
        { generateWave(arguments, [&](wavlab::WavFile &wave) { generate(wave, arguments); }); });
}

template <typename Generate>
void addNoiseCommand(CLI::App &app, CliAction &selectedAction, std::string_view name,
                     std::string_view description, Generate generate)
{
    addCommand<NoiseArguments>(
        app, selectedAction, name, description,
        [](CLI::App &command, NoiseArguments &arguments)
        {
            command.add_option("OUTPUT", arguments.output, "Destination WAV file")->required();
            command.add_option("SAMPLE_RATE", arguments.sampleRate, "Output sample rate in Hz")
                ->required();
            command
                .add_option("BIT_DEPTH", arguments.bitDepth, "Integer PCM depth: 8, 16, 24, or 32")
                ->required()
                ->check(CLI::IsMember({8, 16, 24, 32}));
            command
                .add_option("LENGTH_MS", arguments.lengthMilliseconds,
                            "Output length in milliseconds")
                ->required();
        },
        [generate](const NoiseArguments &arguments)
        {
            auto wave = wavlab::WavFile::create(1, arguments.sampleRate, arguments.bitDepth);
            generate(wave, arguments.lengthMilliseconds);
            wave.save(arguments.output);
        });
}

} // namespace

void configureGeneratorCommands(CLI::App &app, CliAction &selectedAction)
{
    addWaveCommand(
        app, selectedAction, "sine-wave", "Generate a mono sine-wave WAV file", false,
        [](wavlab::WavFile &wave, const WaveArguments &arguments)
        { wavlab::Generators::sineWave(wave, arguments.lengthMilliseconds, arguments.frequency); });
    addWaveCommand(
        app, selectedAction, "square-wave", "Generate a mono square-wave WAV file", false,
        [](wavlab::WavFile &wave, const WaveArguments &arguments)
        {
            wavlab::Generators::squareWave(wave, arguments.lengthMilliseconds, arguments.frequency);
        });
    addWaveCommand(app, selectedAction, "square-wave-additive",
                   "Generate a band-limited square wave by additive synthesis", true,
                   [](wavlab::WavFile &wave, const WaveArguments &arguments)
                   {
                       wavlab::Generators::squareWaveAdditive(wave, arguments.lengthMilliseconds,
                                                              arguments.frequency,
                                                              arguments.harmonics);
                   });
    addWaveCommand(app, selectedAction, "triangle-wave", "Generate a mono triangle-wave WAV file",
                   false,
                   [](wavlab::WavFile &wave, const WaveArguments &arguments)
                   {
                       wavlab::Generators::triangleWave(wave, arguments.lengthMilliseconds,
                                                        arguments.frequency);
                   });
    addWaveCommand(app, selectedAction, "triangle-wave-additive",
                   "Generate a band-limited triangle wave by additive synthesis", true,
                   [](wavlab::WavFile &wave, const WaveArguments &arguments)
                   {
                       wavlab::Generators::triangleWaveAdditive(wave, arguments.lengthMilliseconds,
                                                                arguments.frequency,
                                                                arguments.harmonics);
                   });
    addWaveCommand(app, selectedAction, "sawtooth-wave", "Generate a mono sawtooth-wave WAV file",
                   false,
                   [](wavlab::WavFile &wave, const WaveArguments &arguments)
                   {
                       wavlab::Generators::sawtoothWave(wave, arguments.lengthMilliseconds,
                                                        arguments.frequency);
                   });
    addWaveCommand(app, selectedAction, "sawtooth-wave-additive",
                   "Generate a band-limited sawtooth wave by additive synthesis", true,
                   [](wavlab::WavFile &wave, const WaveArguments &arguments)
                   {
                       wavlab::Generators::sawtoothWaveAdditive(wave, arguments.lengthMilliseconds,
                                                                arguments.frequency,
                                                                arguments.harmonics);
                   });
    addNoiseCommand(app, selectedAction, "white-noise", "Generate a mono white-noise WAV file",
                    [](wavlab::WavFile &wave, std::uint64_t length)
                    { wavlab::Generators::whiteNoise(wave, length); });
    addNoiseCommand(app, selectedAction, "pink-noise", "Generate a mono pink-noise WAV file",
                    [](wavlab::WavFile &wave, std::uint64_t length)
                    { wavlab::Generators::pinkNoise(wave, length); });
}
