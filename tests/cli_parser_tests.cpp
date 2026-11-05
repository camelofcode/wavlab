#include "Cli.h"

#include <filesystem>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

namespace
{

int failures = 0;

void check(bool condition, std::string_view message)
{
    if (!condition)
    {
        std::cerr << "FAILED: " << message << '\n';
        ++failures;
    }
}

bool parses(std::vector<std::string> arguments)
{
    std::vector<char *> argv;
    argv.reserve(arguments.size());
    for (auto &argument : arguments)
    {
        argv.push_back(argument.data());
    }

    try
    {
        CommandLine commandLine;
        commandLine.parse(static_cast<int>(argv.size()), argv.data());
        return true;
    }
    catch (const CLI::ParseError &)
    {
        return false;
    }
}

std::string fixture()
{
    return (std::filesystem::path(WAVLAB_FIXTURE_DIR) / "signal_impulse_mono_s16.wav").string();
}

void testCanonicalCommandsAndAliases()
{
    const auto input = fixture();
    check(parses({"wavlab", "normalise", input}), "canonical British normalise parses");
    check(parses({"wavlab", "normalize", input}), "US-English normalise alias parses");
    check(parses({"wavlab", "metadata", "--full", input}), "full metadata parses");
    check(parses({"wavlab", "stat", input}), "legacy stat alias parses");
    check(
        parses({"wavlab", "digital-delay-mono", input, "2.5", "-0.5", "0.25", "--tail-ms", "100"}),
        "normalised delay arguments and named tail parse");
    check(parses({"wavlab", "tape-delay-stereo", input, "2", "-0.5", "3", "0.5", "0.25",
                  "--cutoff-hz", "1500", "--drive-db", "6"}),
          "stereo tape-delay options parse");
    check(parses({"wavlab", "phaser-mono", input, "1", "0.5", "1000", "triangle", "0.5", "--stages",
                  "16"}),
          "phaser stage boundary parses");
}

void testParserRejectsInvalidShapes()
{
    const auto input = fixture();
    check(!parses({"wavlab", "tremolo", input, "0.5", "1", "sawtooth"}),
          "unknown LFO waveform is rejected while parsing");
    check(!parses({"wavlab", "chorus-mono", input, "0.5", "1", "1.1"}),
          "out-of-range normalised mix is rejected while parsing");
    check(!parses({"wavlab", "phaser-mono", input, "1", "0.5", "1000", "sine", "0.5", "--stages",
                   "17"}),
          "excessive phaser stage count is rejected while parsing");
    check(!parses({"wavlab", "split-stereo", input, "left.wav"}),
          "paired split outputs cannot be partially specified");
    check(!parses({"wavlab", "normalise", "definitely-missing.wav"}),
          "missing input files are rejected while parsing");
    check(!parses({"wavlab"}), "a subcommand is required");
}

void testEveryCommandShape()
{
    const auto input = fixture();
    const std::vector<std::vector<std::string>> commands{
        {"wavlab", "sine-wave", "out.wav", "440", "8000", "16", "100"},
        {"wavlab", "square-wave", "out.wav", "440", "8000", "16", "100"},
        {"wavlab", "square-wave-additive", "out.wav", "440", "8000", "16", "100", "9"},
        {"wavlab", "triangle-wave", "out.wav", "440", "8000", "16", "100"},
        {"wavlab", "triangle-wave-additive", "out.wav", "440", "8000", "16", "100", "9"},
        {"wavlab", "sawtooth-wave", "out.wav", "440", "8000", "16", "100"},
        {"wavlab", "sawtooth-wave-additive", "out.wav", "440", "8000", "16", "100", "9"},
        {"wavlab", "white-noise", "out.wav", "8000", "16", "100"},
        {"wavlab", "pink-noise", "out.wav", "8000", "16", "100"},
        {"wavlab", "metadata", input},
        {"wavlab", "normalise-db", input, "-6", "out.wav"},
        {"wavlab", "gain", input, "0.5", "out.wav"},
        {"wavlab", "gain-db", input, "-6", "out.wav"},
        {"wavlab", "reverse", input, "out.wav"},
        {"wavlab", "flip-phase", input, "out.wav"},
        {"wavlab", "swap-channels", input, "out.wav"},
        {"wavlab", "split-stereo", input, "left.wav", "right.wav"},
        {"wavlab", "split-mid-side", input, "mid.wav", "side.wav"},
        {"wavlab", "left-right-to-stereo", input, input, "out.wav"},
        {"wavlab", "mid-side-to-stereo", input, input, "out.wav"},
        {"wavlab", "split", input, "1", "2"},
        {"wavlab", "mix", "out.wav", input, input},
        {"wavlab", "join", "out.wav", input, input},
        {"wavlab", "get-peak-db", input},
        {"wavlab", "get-rms-db", input},
        {"wavlab", "eq-peak", input, "1000", "-3", "1", "out.wav"},
        {"wavlab", "eq-lowshelf", input, "1000", "-3", "0.7", "out.wav"},
        {"wavlab", "eq-highshelf", input, "1000", "3", "0.7", "out.wav"},
        {"wavlab", "eq-allpass", input, "1000", "1", "out.wav"},
        {"wavlab", "eq-bandpass", input, "1000", "1", "out.wav"},
        {"wavlab", "eq-lowpass", input, "1000", "0.7", "out.wav"},
        {"wavlab", "eq-highpass", input, "1000", "0.7", "out.wav"},
        {"wavlab", "saturate", input, "6", "out.wav"},
        {"wavlab", "digital-delay-stereo", input, "2", "-0.5", "3", "0.5", "0.25", "out.wav"},
        {"wavlab", "analog-delay-mono", input, "2", "-0.5", "0.25", "out.wav", "--cutoff-hz",
         "1500"},
        {"wavlab", "analog-delay-stereo", input, "2", "-0.5", "3", "0.5", "0.25", "out.wav"},
        {"wavlab", "tape-delay-mono", input, "2", "-0.5", "0.25", "out.wav", "--drive-db", "6"},
        {"wavlab", "chorus-mono", input, "0.5", "1", "0.5", "out.wav"},
        {"wavlab", "chorus-stereo", input, "0.5", "1", "1", "2", "0.5", "out.wav"},
        {"wavlab", "vibrato-mono", input, "0.5", "1", "out.wav"},
        {"wavlab", "vibrato-stereo", input, "0.5", "1", "1", "2", "out.wav"},
        {"wavlab", "flanger-mono", input, "0.5", "1", "-0.5", "0.5", "out.wav"},
        {"wavlab", "flanger-stereo", input, "0.5", "1", "-0.5", "1", "2", "0.5", "0.5", "out.wav"},
        {"wavlab", "tremolo", input, "0.5", "1", "triangle", "out.wav"},
        {"wavlab", "autopan", input, "0.5", "1", "square", "out.wav"},
        {"wavlab", "phaser-stereo", input, "0.5", "1", "800", "sine", "1", "2", "1200", "triangle",
         "0.5", "out.wav"},
        {"wavlab", "modulated-filter", input, "200", "1500", "1", "low-pass", "triangle", "0.7",
         "out.wav"},
        {"wavlab", "reverb", input, "0.5", "1.5", "0.25", "out.wav", "--tail-ms", "100"},
    };

    for (auto arguments : commands)
    {
        check(parses(std::move(arguments)), "canonical command shape parses");
    }
}

} // namespace

int main()
{
    testCanonicalCommandsAndAliases();
    testParserRejectsInvalidShapes();
    testEveryCommandShape();
    return failures == 0 ? 0 : 1;
}
