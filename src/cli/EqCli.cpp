#include "CliCommon.h"

#include <wavlab/EQ.h>

#include <string>
#include <string_view>

namespace
{

struct GainEqArguments
{
    InputOutput paths;
    double frequency{};
    double gainDecibels{};
    double shape{};
};

struct PassEqArguments
{
    InputOutput paths;
    double frequency{};
    double shape{};
};

template <typename Process>
void addGainEq(CLI::App &app, CliAction &selectedAction, std::string_view name,
               std::string_view description, std::string_view shapeName,
               std::string_view shapeDescription, Process process)
{
    addCommand<GainEqArguments>(
        app, selectedAction, name, description,
        [shapeName, shapeDescription](CLI::App &command, GainEqArguments &arguments)
        {
            addInput(command, arguments.paths);
            command.add_option("FREQUENCY", arguments.frequency, "Centre frequency in Hz")
                ->required();
            command.add_option("GAIN_DB", arguments.gainDecibels, "Boost or cut in decibels")
                ->required();
            command
                .add_option(std::string(shapeName), arguments.shape, std::string(shapeDescription))
                ->required();
            addOutput(command, arguments.paths);
        },
        [process](const GainEqArguments &arguments)
        {
            processFile(
                arguments.paths, [&](wavlab::WavFile &wave)
                { process(wave, arguments.frequency, arguments.shape, arguments.gainDecibels); });
        });
}

template <typename Process>
void addPassEq(CLI::App &app, CliAction &selectedAction, std::string_view name,
               std::string_view description, std::string_view shapeName,
               std::string_view shapeDescription, Process process)
{
    addCommand<PassEqArguments>(
        app, selectedAction, name, description,
        [shapeName, shapeDescription](CLI::App &command, PassEqArguments &arguments)
        {
            addInput(command, arguments.paths);
            command
                .add_option("FREQUENCY", arguments.frequency, "Centre or cutoff frequency in Hz")
                ->required();
            command
                .add_option(std::string(shapeName), arguments.shape, std::string(shapeDescription))
                ->required();
            addOutput(command, arguments.paths);
        },
        [process](const PassEqArguments &arguments)
        {
            processFile(arguments.paths, [&](wavlab::WavFile &wave)
                        { process(wave, arguments.frequency, arguments.shape); });
        });
}

} // namespace

void configureEqCommands(CLI::App &app, CliAction &selectedAction)
{
    addGainEq(app, selectedAction, "eq-peak", "Apply a peaking EQ boost or cut", "BANDWIDTH",
              "Affected width in octaves",
              [](wavlab::WavFile &wave, double frequency, double bandwidth, double gain)
              { wavlab::EQ::peak(wave, frequency, bandwidth, gain); });
    addGainEq(app, selectedAction, "eq-lowshelf", "Apply a low-shelf EQ boost or cut", "Q",
              "Quality factor", [](wavlab::WavFile &wave, double frequency, double q, double gain)
              { wavlab::EQ::lowShelf(wave, frequency, q, gain); });
    addGainEq(app, selectedAction, "eq-highshelf", "Apply a high-shelf EQ boost or cut", "Q",
              "Quality factor", [](wavlab::WavFile &wave, double frequency, double q, double gain)
              { wavlab::EQ::highShelf(wave, frequency, q, gain); });

    addPassEq(app, selectedAction, "eq-allpass",
              "Apply an all-pass filter that changes phase while preserving magnitude", "BANDWIDTH",
              "Phase-transition width in octaves",
              [](wavlab::WavFile &wave, double frequency, double bandwidth)
              { wavlab::EQ::allPass(wave, frequency, bandwidth); });
    addPassEq(app, selectedAction, "eq-bandpass", "Apply a constant-skirt-gain band-pass filter",
              "BANDWIDTH", "Pass-band width in octaves",
              [](wavlab::WavFile &wave, double frequency, double bandwidth)
              { wavlab::EQ::bandPass(wave, frequency, bandwidth); });
    addPassEq(app, selectedAction, "eq-lowpass", "Apply a 12 dB-per-octave low-pass filter", "Q",
              "Resonance quality factor", [](wavlab::WavFile &wave, double frequency, double q)
              { wavlab::EQ::lowPass(wave, frequency, q); });
    addPassEq(app, selectedAction, "eq-highpass", "Apply a 12 dB-per-octave high-pass filter", "Q",
              "Resonance quality factor", [](wavlab::WavFile &wave, double frequency, double q)
              { wavlab::EQ::highPass(wave, frequency, q); });
}
