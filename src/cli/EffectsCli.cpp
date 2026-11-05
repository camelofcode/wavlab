#include "CliCommon.h"

#include <wavlab/FX.h>

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>

namespace
{

struct SaturateArguments
{
    InputOutput paths;
    double driveDecibels{};
};

struct MonoDelayArguments
{
    InputOutput paths;
    double delayMilliseconds{};
    double feedback{};
    double mix{};
    std::uint64_t tailMilliseconds{};
    double cutoffHz{wavlab::FX::defaultDelayCutoffHz};
    double driveDecibels{};
};

struct StereoDelayArguments
{
    InputOutput paths;
    double leftDelayMilliseconds{};
    double leftFeedback{};
    double rightDelayMilliseconds{};
    double rightFeedback{};
    double mix{};
    std::uint64_t tailMilliseconds{};
    double cutoffHz{wavlab::FX::defaultDelayCutoffHz};
    double driveDecibels{};
};

void addMonoDelayOptions(CLI::App &command, MonoDelayArguments &arguments, bool cutoff, bool drive)
{
    addInput(command, arguments.paths);
    command
        .add_option("DELAY_MS", arguments.delayMilliseconds,
                    "Delay time in milliseconds; fractional values are supported")
        ->required();
    command.add_option("FEEDBACK", arguments.feedback, "Signed feedback from -1 to 1")->required();
    command.add_option("MIX", arguments.mix, "Wet proportion from 0 (dry) to 1 (wet)")
        ->required()
        ->check(normalisedValue());
    addOutput(command, arguments.paths);
    command
        .add_option("--tail-ms", arguments.tailMilliseconds,
                    "Additional output tail in milliseconds")
        ->default_val(0);
    if (cutoff)
    {
        command.add_option("--cutoff-hz", arguments.cutoffHz, "Feedback low-pass cutoff in Hz")
            ->default_val(wavlab::FX::defaultDelayCutoffHz);
    }
    if (drive)
    {
        command
            .add_option("--drive-db", arguments.driveDecibels, "Pre-saturation feedback gain in dB")
            ->default_val(0.0);
    }
}

void addStereoDelayOptions(CLI::App &command, StereoDelayArguments &arguments, bool cutoff,
                           bool drive)
{
    addInput(command, arguments.paths);
    command
        .add_option("LEFT_DELAY_MS", arguments.leftDelayMilliseconds, "Left delay in milliseconds")
        ->required();
    command.add_option("LEFT_FEEDBACK", arguments.leftFeedback, "Signed left feedback")->required();
    command
        .add_option("RIGHT_DELAY_MS", arguments.rightDelayMilliseconds,
                    "Right delay in milliseconds")
        ->required();
    command.add_option("RIGHT_FEEDBACK", arguments.rightFeedback, "Signed right feedback")
        ->required();
    command.add_option("MIX", arguments.mix, "Wet proportion from 0 (dry) to 1 (wet)")
        ->required()
        ->check(normalisedValue());
    addOutput(command, arguments.paths);
    command
        .add_option("--tail-ms", arguments.tailMilliseconds,
                    "Additional output tail in milliseconds")
        ->default_val(0);
    if (cutoff)
    {
        command.add_option("--cutoff-hz", arguments.cutoffHz, "Feedback low-pass cutoff in Hz")
            ->default_val(wavlab::FX::defaultDelayCutoffHz);
    }
    if (drive)
    {
        command
            .add_option("--drive-db", arguments.driveDecibels, "Pre-saturation feedback gain in dB")
            ->default_val(0.0);
    }
}

template <typename Process>
void addMonoDelay(CLI::App &app, CliAction &selectedAction, std::string_view name,
                  std::string_view description, bool cutoff, bool drive, Process process)
{
    addCommand<MonoDelayArguments>(
        app, selectedAction, name, description,
        [cutoff, drive](CLI::App &command, MonoDelayArguments &arguments)
        { addMonoDelayOptions(command, arguments, cutoff, drive); },
        [process](const MonoDelayArguments &arguments)
        {
            processFile(arguments.paths, [&](wavlab::WavFile &wave) { process(wave, arguments); });
        });
}

template <typename Process>
void addStereoDelay(CLI::App &app, CliAction &selectedAction, std::string_view name,
                    std::string_view description, bool cutoff, bool drive, Process process)
{
    addCommand<StereoDelayArguments>(
        app, selectedAction, name, description,
        [cutoff, drive](CLI::App &command, StereoDelayArguments &arguments)
        { addStereoDelayOptions(command, arguments, cutoff, drive); },
        [process](const StereoDelayArguments &arguments)
        {
            processFile(arguments.paths, [&](wavlab::WavFile &wave) { process(wave, arguments); });
        });
}

[[nodiscard]] wavlab::DelaySettings delaySettings(const MonoDelayArguments &arguments)
{
    return {{arguments.delayMilliseconds, arguments.feedback},
            arguments.mix,
            arguments.tailMilliseconds};
}

[[nodiscard]] wavlab::StereoDelaySettings delaySettings(const StereoDelayArguments &arguments)
{
    return {{arguments.leftDelayMilliseconds, arguments.leftFeedback},
            {arguments.rightDelayMilliseconds, arguments.rightFeedback},
            arguments.mix,
            arguments.tailMilliseconds};
}

struct MonoChorusArguments
{
    InputOutput paths;
    double depth{};
    double frequencyHz{};
    double mix{};
};

struct StereoChorusArguments
{
    InputOutput paths;
    double leftDepth{};
    double leftFrequencyHz{};
    double rightDepth{};
    double rightFrequencyHz{};
    double mix{};
};

void addMonoChorusOptions(CLI::App &command, MonoChorusArguments &arguments, bool mix)
{
    addInput(command, arguments.paths);
    command.add_option("DEPTH", arguments.depth, "Normalised LFO depth from 0 to 1")
        ->required()
        ->check(normalisedValue());
    command.add_option("FREQUENCY_HZ", arguments.frequencyHz, "LFO frequency in Hz")->required();
    if (mix)
    {
        command.add_option("MIX", arguments.mix, "Wet proportion from 0 to 1")
            ->required()
            ->check(normalisedValue());
    }
    addOutput(command, arguments.paths);
}

void addStereoChorusOptions(CLI::App &command, StereoChorusArguments &arguments, bool mix)
{
    addInput(command, arguments.paths);
    command.add_option("LEFT_DEPTH", arguments.leftDepth, "Normalised left depth")
        ->required()
        ->check(normalisedValue());
    command.add_option("LEFT_FREQUENCY_HZ", arguments.leftFrequencyHz, "Left LFO frequency")
        ->required();
    command.add_option("RIGHT_DEPTH", arguments.rightDepth, "Normalised right depth")
        ->required()
        ->check(normalisedValue());
    command.add_option("RIGHT_FREQUENCY_HZ", arguments.rightFrequencyHz, "Right LFO frequency")
        ->required();
    if (mix)
    {
        command.add_option("MIX", arguments.mix, "Wet proportion from 0 to 1")
            ->required()
            ->check(normalisedValue());
    }
    addOutput(command, arguments.paths);
}

struct MonoFlangerArguments
{
    InputOutput paths;
    double depth{};
    double frequencyHz{};
    double feedback{};
    double mix{};
};

struct StereoFlangerArguments
{
    InputOutput paths;
    double leftDepth{};
    double leftFrequencyHz{};
    double leftFeedback{};
    double rightDepth{};
    double rightFrequencyHz{};
    double rightFeedback{};
    double mix{};
};

struct LfoEffectArguments
{
    InputOutput paths;
    double depth{};
    double frequencyHz{};
    std::string waveform;
};

void addLfoEffectOptions(CLI::App &command, LfoEffectArguments &arguments)
{
    addInput(command, arguments.paths);
    command.add_option("DEPTH", arguments.depth, "Normalised modulation depth")
        ->required()
        ->check(normalisedValue());
    command.add_option("FREQUENCY_HZ", arguments.frequencyHz, "LFO frequency in Hz")->required();
    command.add_option("WAVEFORM", arguments.waveform, "sine, triangle, or square")
        ->required()
        ->check(waveformValue());
    addOutput(command, arguments.paths);
}

struct MonoPhaserArguments
{
    InputOutput paths;
    double depth{};
    double frequencyHz{};
    double centreFrequencyHz{};
    std::string waveform;
    double mix{};
    std::size_t stages{wavlab::FX::defaultPhaserStages};
};

struct StereoPhaserArguments
{
    InputOutput paths;
    double leftDepth{};
    double leftFrequencyHz{};
    double leftCentreFrequencyHz{};
    std::string leftWaveform;
    double rightDepth{};
    double rightFrequencyHz{};
    double rightCentreFrequencyHz{};
    std::string rightWaveform;
    double mix{};
    std::size_t stages{wavlab::FX::defaultPhaserStages};
};

struct ModulatedFilterArguments
{
    InputOutput paths;
    double minimumFrequencyHz{};
    double maximumFrequencyHz{};
    double frequencyHz{};
    std::string filterType;
    std::string waveform;
    double q{};
};

struct ReverbArguments
{
    InputOutput paths;
    double roomSize{};
    double decaySeconds{};
    double mix{};
    std::uint64_t tailMilliseconds{};
};

} // namespace

void configureEffectCommands(CLI::App &app, CliAction &selectedAction)
{
    addCommand<SaturateArguments>(
        app, selectedAction, "saturate", "Apply sigmoid soft clipping like tape saturation",
        [](CLI::App &command, SaturateArguments &arguments)
        {
            addInput(command, arguments.paths);
            command
                .add_option("DRIVE_DB", arguments.driveDecibels, "Pre-saturation input gain in dB")
                ->required();
            addOutput(command, arguments.paths);
        },
        [](const SaturateArguments &arguments)
        {
            processFile(arguments.paths, [&](wavlab::WavFile &wave)
                        { wavlab::FX::saturate(wave, arguments.driveDecibels); });
        });

    addMonoDelay(app, selectedAction, "digital-delay-mono",
                 "Apply clean delay with common channel settings", false, false,
                 [](wavlab::WavFile &wave, const MonoDelayArguments &arguments)
                 { wavlab::FX::digitalDelay(wave, delaySettings(arguments)); });
    addStereoDelay(app, selectedAction, "digital-delay-stereo",
                   "Apply clean stereo delay with independent channel settings", false, false,
                   [](wavlab::WavFile &wave, const StereoDelayArguments &arguments)
                   { wavlab::FX::digitalDelay(wave, delaySettings(arguments)); });
    addMonoDelay(app, selectedAction, "analog-delay-mono",
                 "Apply low-pass-filtered delay with common settings", true, false,
                 [](wavlab::WavFile &wave, const MonoDelayArguments &arguments)
                 { wavlab::FX::analogDelay(wave, delaySettings(arguments), arguments.cutoffHz); });
    addStereoDelay(
        app, selectedAction, "analog-delay-stereo",
        "Apply low-pass-filtered stereo delay with independent settings", true, false,
        [](wavlab::WavFile &wave, const StereoDelayArguments &arguments)
        { wavlab::FX::analogDelay(wave, delaySettings(arguments), arguments.cutoffHz); });
    addMonoDelay(app, selectedAction, "tape-delay-mono",
                 "Apply saturated, low-pass-filtered delay with common settings", true, true,
                 [](wavlab::WavFile &wave, const MonoDelayArguments &arguments)
                 {
                     wavlab::FX::tapeDelay(wave, delaySettings(arguments), arguments.driveDecibels,
                                           arguments.cutoffHz);
                 });
    addStereoDelay(app, selectedAction, "tape-delay-stereo",
                   "Apply saturated stereo delay with independent channel settings", true, true,
                   [](wavlab::WavFile &wave, const StereoDelayArguments &arguments)
                   {
                       wavlab::FX::tapeDelay(wave, delaySettings(arguments),
                                             arguments.driveDecibels, arguments.cutoffHz);
                   });

    addCommand<MonoChorusArguments>(
        app, selectedAction, "chorus-mono", "Apply common-setting chorus to every channel",
        [](CLI::App &command, MonoChorusArguments &arguments)
        { addMonoChorusOptions(command, arguments, true); },
        [](const MonoChorusArguments &arguments)
        {
            processFile(arguments.paths,
                        [&](wavlab::WavFile &wave)
                        {
                            wavlab::FX::chorus(
                                wave, wavlab::ChorusSettings{
                                          {arguments.depth, arguments.frequencyHz}, arguments.mix});
                        });
        });
    addCommand<StereoChorusArguments>(
        app, selectedAction, "chorus-stereo", "Apply independent stereo chorus",
        [](CLI::App &command, StereoChorusArguments &arguments)
        { addStereoChorusOptions(command, arguments, true); },
        [](const StereoChorusArguments &arguments)
        {
            processFile(arguments.paths,
                        [&](wavlab::WavFile &wave)
                        {
                            wavlab::FX::chorus(
                                wave, wavlab::StereoChorusSettings{
                                          {arguments.leftDepth, arguments.leftFrequencyHz},
                                          {arguments.rightDepth, arguments.rightFrequencyHz},
                                          arguments.mix});
                        });
        });
    addCommand<MonoChorusArguments>(
        app, selectedAction, "vibrato-mono", "Apply fully wet chorus as pitch modulation",
        [](CLI::App &command, MonoChorusArguments &arguments)
        { addMonoChorusOptions(command, arguments, false); },
        [](const MonoChorusArguments &arguments)
        {
            processFile(arguments.paths,
                        [&](wavlab::WavFile &wave)
                        {
                            wavlab::FX::vibrato(
                                wave,
                                wavlab::VibratoSettings{{arguments.depth, arguments.frequencyHz}});
                        });
        });
    addCommand<StereoChorusArguments>(
        app, selectedAction, "vibrato-stereo", "Apply independent stereo pitch modulation",
        [](CLI::App &command, StereoChorusArguments &arguments)
        { addStereoChorusOptions(command, arguments, false); },
        [](const StereoChorusArguments &arguments)
        {
            processFile(arguments.paths,
                        [&](wavlab::WavFile &wave)
                        {
                            wavlab::FX::vibrato(
                                wave, wavlab::StereoVibratoSettings{
                                          {arguments.leftDepth, arguments.leftFrequencyHz},
                                          {arguments.rightDepth, arguments.rightFrequencyHz}});
                        });
        });

    addCommand<MonoFlangerArguments>(
        app, selectedAction, "flanger-mono", "Apply common-setting triangle-LFO flanging",
        [](CLI::App &command, MonoFlangerArguments &arguments)
        {
            addInput(command, arguments.paths);
            command.add_option("DEPTH", arguments.depth, "Normalised depth")
                ->required()
                ->check(normalisedValue());
            command.add_option("FREQUENCY_HZ", arguments.frequencyHz, "LFO frequency in Hz")
                ->required();
            command
                .add_option("FEEDBACK", arguments.feedback, "Signed feedback from -0.95 to 0.95")
                ->required();
            command.add_option("MIX", arguments.mix, "Wet proportion from 0 to 1")
                ->required()
                ->check(normalisedValue());
            addOutput(command, arguments.paths);
        },
        [](const MonoFlangerArguments &arguments)
        {
            processFile(arguments.paths,
                        [&](wavlab::WavFile &wave)
                        {
                            wavlab::FX::flanger(wave, wavlab::FlangerSettings{
                                                          {arguments.depth, arguments.frequencyHz,
                                                           arguments.feedback},
                                                          arguments.mix});
                        });
        });
    addCommand<StereoFlangerArguments>(
        app, selectedAction, "flanger-stereo", "Apply independent stereo flanging",
        [](CLI::App &command, StereoFlangerArguments &arguments)
        {
            addInput(command, arguments.paths);
            command.add_option("LEFT_DEPTH", arguments.leftDepth, "Normalised left depth")
                ->required()
                ->check(normalisedValue());
            command
                .add_option("LEFT_FREQUENCY_HZ", arguments.leftFrequencyHz, "Left LFO frequency")
                ->required();
            command.add_option("LEFT_FEEDBACK", arguments.leftFeedback, "Signed left feedback")
                ->required();
            command.add_option("RIGHT_DEPTH", arguments.rightDepth, "Normalised right depth")
                ->required()
                ->check(normalisedValue());
            command
                .add_option("RIGHT_FREQUENCY_HZ", arguments.rightFrequencyHz, "Right LFO frequency")
                ->required();
            command.add_option("RIGHT_FEEDBACK", arguments.rightFeedback, "Signed right feedback")
                ->required();
            command.add_option("MIX", arguments.mix, "Wet proportion from 0 to 1")
                ->required()
                ->check(normalisedValue());
            addOutput(command, arguments.paths);
        },
        [](const StereoFlangerArguments &arguments)
        {
            processFile(arguments.paths,
                        [&](wavlab::WavFile &wave)
                        {
                            wavlab::FX::flanger(
                                wave, wavlab::StereoFlangerSettings{
                                          {arguments.leftDepth, arguments.leftFrequencyHz,
                                           arguments.leftFeedback},
                                          {arguments.rightDepth, arguments.rightFrequencyHz,
                                           arguments.rightFeedback},
                                          arguments.mix});
                        });
        });

    const auto addLfoEffect = [&](std::string_view name, std::string_view description, auto process)
    {
        addCommand<LfoEffectArguments>(
            app, selectedAction, name, description,
            [](CLI::App &command, LfoEffectArguments &arguments)
            { addLfoEffectOptions(command, arguments); },
            [process](const LfoEffectArguments &arguments)
            {
                processFile(arguments.paths,
                            [&](wavlab::WavFile &wave)
                            {
                                process(wave, arguments.depth, arguments.frequencyHz,
                                        lfoWaveform(arguments.waveform));
                            });
            });
    };
    addLfoEffect(
        "tremolo", "Apply LFO-controlled volume modulation",
        [](wavlab::WavFile &wave, double depth, double frequency, wavlab::LfoWaveform waveform)
        { wavlab::FX::tremolo(wave, {depth, frequency, waveform}); });
    addLfoEffect(
        "autopan", "Apply LFO-controlled panning and always output stereo",
        [](wavlab::WavFile &wave, double depth, double frequency, wavlab::LfoWaveform waveform)
        { wavlab::FX::autoPan(wave, {depth, frequency, waveform}); });

    addCommand<MonoPhaserArguments>(
        app, selectedAction, "phaser-mono", "Apply common-setting multi-stage phasing",
        [](CLI::App &command, MonoPhaserArguments &arguments)
        {
            addInput(command, arguments.paths);
            command.add_option("DEPTH", arguments.depth, "Normalised sweep depth")
                ->required()
                ->check(normalisedValue());
            command.add_option("FREQUENCY_HZ", arguments.frequencyHz, "LFO frequency")->required();
            command
                .add_option("CENTRE_FREQUENCY_HZ", arguments.centreFrequencyHz,
                            "Geometric sweep centre")
                ->required();
            command.add_option("WAVEFORM", arguments.waveform, "sine, triangle, or square")
                ->required()
                ->check(waveformValue());
            command.add_option("MIX", arguments.mix, "Wet proportion from 0 to 1")
                ->required()
                ->check(normalisedValue());
            addOutput(command, arguments.paths);
            command.add_option("--stages", arguments.stages, "All-pass stage count from 1 to 16")
                ->default_val(wavlab::FX::defaultPhaserStages)
                ->check(CLI::Range(std::size_t{1}, wavlab::FX::maximumPhaserStages));
        },
        [](const MonoPhaserArguments &arguments)
        {
            processFile(arguments.paths,
                        [&](wavlab::WavFile &wave)
                        {
                            wavlab::FX::phaser(wave, wavlab::PhaserSettings{
                                                         {arguments.depth, arguments.frequencyHz,
                                                          arguments.centreFrequencyHz,
                                                          lfoWaveform(arguments.waveform)},
                                                         arguments.stages,
                                                         arguments.mix});
                        });
        });

    addCommand<StereoPhaserArguments>(
        app, selectedAction, "phaser-stereo", "Apply independent stereo multi-stage phasing",
        [](CLI::App &command, StereoPhaserArguments &arguments)
        {
            addInput(command, arguments.paths);
            command.add_option("LEFT_DEPTH", arguments.leftDepth, "Normalised left sweep depth")
                ->required()
                ->check(normalisedValue());
            command
                .add_option("LEFT_FREQUENCY_HZ", arguments.leftFrequencyHz, "Left LFO frequency")
                ->required();
            command
                .add_option("LEFT_CENTRE_FREQUENCY_HZ", arguments.leftCentreFrequencyHz,
                            "Left sweep centre")
                ->required();
            command
                .add_option("LEFT_WAVEFORM", arguments.leftWaveform,
                            "Left sine, triangle, or square waveform")
                ->required()
                ->check(waveformValue());
            command.add_option("RIGHT_DEPTH", arguments.rightDepth, "Normalised right sweep depth")
                ->required()
                ->check(normalisedValue());
            command
                .add_option("RIGHT_FREQUENCY_HZ", arguments.rightFrequencyHz, "Right LFO frequency")
                ->required();
            command
                .add_option("RIGHT_CENTRE_FREQUENCY_HZ", arguments.rightCentreFrequencyHz,
                            "Right sweep centre")
                ->required();
            command
                .add_option("RIGHT_WAVEFORM", arguments.rightWaveform,
                            "Right sine, triangle, or square waveform")
                ->required()
                ->check(waveformValue());
            command.add_option("MIX", arguments.mix, "Wet proportion from 0 to 1")
                ->required()
                ->check(normalisedValue());
            addOutput(command, arguments.paths);
            command.add_option("--stages", arguments.stages, "All-pass stage count from 1 to 16")
                ->default_val(wavlab::FX::defaultPhaserStages)
                ->check(CLI::Range(std::size_t{1}, wavlab::FX::maximumPhaserStages));
        },
        [](const StereoPhaserArguments &arguments)
        {
            processFile(arguments.paths,
                        [&](wavlab::WavFile &wave)
                        {
                            wavlab::FX::phaser(
                                wave, wavlab::StereoPhaserSettings{
                                          {arguments.leftDepth, arguments.leftFrequencyHz,
                                           arguments.leftCentreFrequencyHz,
                                           lfoWaveform(arguments.leftWaveform)},
                                          {arguments.rightDepth, arguments.rightFrequencyHz,
                                           arguments.rightCentreFrequencyHz,
                                           lfoWaveform(arguments.rightWaveform)},
                                          arguments.stages,
                                          arguments.mix});
                        });
        });

    addCommand<ModulatedFilterArguments>(
        app, selectedAction, "modulated-filter", "Apply an LFO-modulated low/high-pass filter",
        [](CLI::App &command, ModulatedFilterArguments &arguments)
        {
            addInput(command, arguments.paths);
            command
                .add_option("MINIMUM_FREQUENCY_HZ", arguments.minimumFrequencyHz,
                            "Lowest cutoff in Hz")
                ->required();
            command
                .add_option("MAXIMUM_FREQUENCY_HZ", arguments.maximumFrequencyHz,
                            "Highest cutoff in Hz")
                ->required();
            command.add_option("FREQUENCY_HZ", arguments.frequencyHz, "LFO frequency in Hz")
                ->required();
            command.add_option("FILTER_TYPE", arguments.filterType, "low-pass or high-pass")
                ->required()
                ->check(CLI::IsMember({"low-pass", "high-pass"}));
            command.add_option("WAVEFORM", arguments.waveform, "sine, triangle, or square")
                ->required()
                ->check(waveformValue());
            command.add_option("Q", arguments.q, "Filter resonance quality factor")->required();
            addOutput(command, arguments.paths);
        },
        [](const ModulatedFilterArguments &arguments)
        {
            const auto type = arguments.filterType == "low-pass"
                                  ? wavlab::ModulatedFilterType::lowPass
                                  : wavlab::ModulatedFilterType::highPass;
            processFile(arguments.paths,
                        [&](wavlab::WavFile &wave)
                        {
                            wavlab::FX::modulatedFilter(
                                wave, {arguments.minimumFrequencyHz, arguments.maximumFrequencyHz,
                                       arguments.frequencyHz, type, lfoWaveform(arguments.waveform),
                                       arguments.q});
                        });
        });

    addCommand<ReverbArguments>(
        app, selectedAction, "reverb", "Apply classic Schroeder algorithmic reverb",
        [](CLI::App &command, ReverbArguments &arguments)
        {
            addInput(command, arguments.paths);
            command.add_option("ROOM_SIZE", arguments.roomSize, "Normalised room size from 0 to 1")
                ->required()
                ->check(normalisedValue());
            command.add_option("DECAY_SECONDS", arguments.decaySeconds, "RT60 decay in seconds")
                ->required();
            command.add_option("MIX", arguments.mix, "Wet proportion from 0 to 1")
                ->required()
                ->check(normalisedValue());
            addOutput(command, arguments.paths);
            command
                .add_option("--tail-ms", arguments.tailMilliseconds,
                            "Additional output tail in milliseconds")
                ->default_val(0);
        },
        [](const ReverbArguments &arguments)
        {
            processFile(arguments.paths,
                        [&](wavlab::WavFile &wave)
                        {
                            wavlab::FX::reverb(wave, {arguments.roomSize, arguments.decaySeconds,
                                                      arguments.mix, arguments.tailMilliseconds});
                        });
        });
}
