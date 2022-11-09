#include "FX.h"
#include "Commands.h"
#include "Constants.h"
#include "Delay.h"
#include "Tools.h"
#include "EQ.h"
#include <cmath>

/* saturate command */
void Commands::saturateHelp()
{
    std::cout << "saturate <input> <drive> [output]" << std::endl;
    std::cout << "\tdescription: applies a tape saturation effect to a wave file" << std::endl;
    std::cout << "\tparameters:" << std::endl;
    std::cout << "\t\tinput: path to wave file" << std::endl;
    std::cout << "\t\tdrive: increase for dirtier sound" << std::endl;
    std::cout << "\t\toutput: path to output file. Optional, if not specified input will be overwritten" << std::endl;
}

int Commands::saturateCommand(const std::string& command, int argc, char* argv[])
{
    if (argc != 4 && argc != 5)
    {
        Commands::printIncorrectArgumentsWarning(command);
        return 1;
    }
    std::string input = static_cast<std::string>(argv[2]);

    double drive = 0.0;
    if (!tryParse(argv[3], drive))
    {
        std::cout << "Parameter 'drive' could not be parsed. Please ensure it is a numeric value. Provided value: " << argv[3] << std::endl;
        return 1;
    }

    std::string output = argc == 5 ? static_cast<std::string>(argv[4]) : input;

    WavFile wav(input, true);
    FX::saturate(wav, drive);
    wav.save(output);
    return 0;
}

void FX::saturate(WavFile& wave, double drive)
{
    std::vector<double> newSamples;

    double gainFactor = (drive + 0.7) * 1.2;

    for (auto& sample : wave.getSamples())
    {
        double saturated = saturateSample(sample, gainFactor);
        newSamples.push_back(saturated);
    }
    wave.setSamples(newSamples);
}

double FX::saturateSample(double in, double gainFactor)
{
    //At one point (2009) this algorithm existed at http://tap-plugins.sourceforge.net/ladspa/sigmoid.html
    double preSaturation = in * gainFactor;
    return 2 / (1 + (pow(Constants::E, (-5.0 * preSaturation)))) - 1;
}

/* digital-delay-mono command*/
void Commands::digitalDelayMonoHelp()
{
    std::cout << "digital-delay-mono <input> <delayMilliseconds> <feedback> <mix> [output] [maxOutputLengthMs]" << std::endl;
    std::cout << "\tdescription: applies a mono digital delay to the specified wave file" << std::endl;
    std::cout << "\tparameters:" << std::endl;
    std::cout << "\t\tinput: path to wave file" << std::endl;
    std::cout << "\t\tdelayMilliseconds: delay time in milliseconds" << std::endl;
    std::cout << "\t\tfeedback: feedback of the delay effect, from 0-100%" << std::endl;
    std::cout << "\t\tmix: mix of the delay effect vs dry signal, from 0-100%" << std::endl;
    std::cout << "\t\toutput: path to output file. Optional, if not specified input will be overwritten" << std::endl;
    std::cout << "\t\tmaxOutputLengthMs: the max length of the output file in milliseconds, to allow for feedback tail" << std::endl;
}

int Commands::digitalDelayMonoCommand(const std::string& command, int argc, char* argv[])
{
    if (argc != 6 && argc != 7 && argc != 8)
    {
        Commands::printIncorrectArgumentsWarning(command);
        return 1;
    }
    std::string input = static_cast<std::string>(argv[2]);

    uint32_t delayMilliseconds = 0;
    if (!tryParse(argv[3], delayMilliseconds))
    {
        std::cout << "Parameter 'delayMilliseconds' could not be parsed. Please ensure it is a numeric value. Provided value: " << argv[3] << std::endl;
        return 1;
    }

    double feedback = 0.0;
    if (!tryParse(argv[4], feedback))
    {
        std::cout << "Parameter 'feedback' could not be parsed. Please ensure it is a numeric value. Provided value: " << argv[4] << std::endl;
        return 1;
    }
    if (feedback < 0 || feedback > 100)
    {
        std::cout << "Parameter 'feedback' must be between 0 and 100. Provided value: " << argv[4] << std::endl;
        return 1;
    }

    double mix = 0.0;
    if (!tryParse(argv[5], mix))
    {
        std::cout << "Parameter 'mix' could not be parsed. Please ensure it is a numeric value. Provided value: " << argv[5] << std::endl;
        return 1;
    }
    if (mix < 0 || mix > 100)
    {
        std::cout << "Parameter 'mix' must be between 0 and 100. Provided value: " << argv[5] << std::endl;
        return 1;
    }

    std::string output = argc == 7 || argc == 8 ? static_cast<std::string>(argv[6]) : input;

    uint32_t maxFileLengthMs = 0;
    if (argc == 8) 
    {
        if (!tryParse(argv[7], maxFileLengthMs))
        {
            std::cout << "Parameter 'maxFileLengthMs' could not be parsed. Please ensure it is a numeric value. Provided value: " << argv[7] << std::endl;
            return 1;
        }
    }

    WavFile wav(input, true);
    FX::delay(wav, delayMilliseconds, feedback, mix, maxFileLengthMs);
    wav.save(output);
    return 0;
}

/* digital-delay-stereo command*/
void Commands::digitalDelayStereoHelp()
{
    std::cout << "digital-delay-stereo <input> <delayMillisecondsLeft> <feedbackLeft> <delayMillisecondsRight> <feedbackRight> <mix> [output] [maxOutputLengthMs]" << std::endl;
    std::cout << "\tdescription: applies a stereo digital delay to the specified wave file" << std::endl;
    std::cout << "\tparameters:" << std::endl;
    std::cout << "\t\tinput: path to wave file" << std::endl;
    std::cout << "\t\tdelayMillisecondsLeft: delay time in milliseconds for the left channel" << std::endl;
    std::cout << "\t\tfeedbackLeft: feedback of the delay effect, from 0-100% for the left channel" << std::endl;
    std::cout << "\t\tdelayMillisecondsRight: delay time in milliseconds for the right channel" << std::endl;
    std::cout << "\t\tfeedbackRight: feedback of the delay effect, from 0-100% for the right channel" << std::endl;
    std::cout << "\t\tmix: mix of the delay effect vs dry signal, from 0-100%" << std::endl;
    std::cout << "\t\toutput: path to output file. Optional, if not specified input will be overwritten" << std::endl;
    std::cout << "\t\tmaxOutputLengthMs: the max length of the output file in milliseconds, to allow for feedback tail" << std::endl;
}

int Commands::digitalDelayStereoCommand(const std::string& command, int argc, char* argv[])
{
    if (argc != 8 && argc != 9 && argc != 10)
    {
        Commands::printIncorrectArgumentsWarning(command);
        return 1;
    }
    std::string input = static_cast<std::string>(argv[2]);

    uint32_t delayMillisecondsLeft = 0;
    if (!tryParse(argv[3], delayMillisecondsLeft))
    {
        std::cout << "Parameter 'delayMillisecondsLeft' could not be parsed. Please ensure it is a numeric value. Provided value: " << argv[3] << std::endl;
        return 1;
    }

    double feedbackLeft = 0.0;
    if (!tryParse(argv[4], feedbackLeft))
    {
        std::cout << "Parameter 'feedbackLeft' could not be parsed. Please ensure it is a numeric value. Provided value: " << argv[4] << std::endl;
        return 1;
    }
    if (feedbackLeft < 0 || feedbackLeft > 100)
    {
        std::cout << "Parameter 'feedbackLeft' must be between 0 and 100. Provided value: " << argv[4] << std::endl;
        return 1;
    }

    uint32_t delayMillisecondsRight = 0;
    if (!tryParse(argv[5], delayMillisecondsRight))
    {
        std::cout << "Parameter 'delayMillisecondsRight' could not be parsed. Please ensure it is a numeric value. Provided value: " << argv[5] << std::endl;
        return 1;
    }

    double feedbackRight = 0.0;
    if (!tryParse(argv[6], feedbackRight))
    {
        std::cout << "Parameter 'feedbackRight' could not be parsed. Please ensure it is a numeric value. Provided value: " << argv[6] << std::endl;
        return 1;
    }
    if (feedbackRight < 0 || feedbackRight > 100)
    {
        std::cout << "Parameter 'feedbackRight' must be between 0 and 100. Provided value: " << argv[6] << std::endl;
        return 1;
    }

    double mix = 0.0;
    if (!tryParse(argv[7], mix))
    {
        std::cout << "Parameter 'mix' could not be parsed. Please ensure it is a numeric value. Provided value: " << argv[7] << std::endl;
        return 1;
    }
    if (mix < 0 || mix > 100)
    {
        std::cout << "Parameter 'mix' must be between 0 and 100. Provided value: " << argv[7] << std::endl;
        return 1;
    }

    std::string output = argc == 9 || argc == 10 ? static_cast<std::string>(argv[8]) : input;

    uint32_t maxFileLengthMs = 0;
    if (argc == 10)
    {
        if (!tryParse(argv[9], maxFileLengthMs))
        {
            std::cout << "Parameter 'maxFileLengthMs' could not be parsed. Please ensure it is a numeric value. Provided value: " << argv[9] << std::endl;
            return 1;
        }
    }

    WavFile wav(input, true);
    FX::delay(wav, delayMillisecondsLeft, feedbackLeft, delayMillisecondsRight, feedbackRight, mix, maxFileLengthMs);
    wav.save(output);
    return 0;
}

/* analog-delay-mono command*/
void Commands::analogDelayMonoHelp()
{
    std::cout << "analog-delay-mono <input> <delayMilliseconds> <feedback> <mix> [output] [maxOutputLengthMs]" << std::endl;
    std::cout << "\tdescription: applies a mono analog style delay to the specified wave file, with frequency rolloff in the feedback loop" << std::endl;
    std::cout << "\tparameters:" << std::endl;
    std::cout << "\t\tinput: path to wave file" << std::endl;
    std::cout << "\t\tdelayMilliseconds: delay time in milliseconds" << std::endl;
    std::cout << "\t\tfeedback: feedback of the delay effect, from 0-100%" << std::endl;
    std::cout << "\t\tmix: mix of the delay effect vs dry signal, from 0-100%" << std::endl;
    std::cout << "\t\toutput: path to output file. Optional, if not specified input will be overwritten" << std::endl;
    std::cout << "\t\tmaxOutputLengthMs: the max length of the output file in milliseconds, to allow for feedback tail" << std::endl;
}

int Commands::analogDelayMonoCommand(const std::string& command, int argc, char* argv[])
{
    if (argc != 6 && argc != 7 && argc != 8)
    {
        Commands::printIncorrectArgumentsWarning(command);
        return 1;
    }
    std::string input = static_cast<std::string>(argv[2]);

    uint32_t delayMilliseconds = 0;
    if (!tryParse(argv[3], delayMilliseconds))
    {
        std::cout << "Parameter 'delayMilliseconds' could not be parsed. Please ensure it is a numeric value. Provided value: " << argv[3] << std::endl;
        return 1;
    }

    double feedback = 0.0;
    if (!tryParse(argv[4], feedback))
    {
        std::cout << "Parameter 'feedback' could not be parsed. Please ensure it is a numeric value. Provided value: " << argv[4] << std::endl;
        return 1;
    }
    if (feedback < 0 || feedback > 100)
    {
        std::cout << "Parameter 'feedback' must be between 0 and 100. Provided value: " << argv[4] << std::endl;
        return 1;
    }

    double mix = 0.0;
    if (!tryParse(argv[5], mix))
    {
        std::cout << "Parameter 'mix' could not be parsed. Please ensure it is a numeric value. Provided value: " << argv[5] << std::endl;
        return 1;
    }
    if (mix < 0 || mix > 100)
    {
        std::cout << "Parameter 'mix' must be between 0 and 100. Provided value: " << argv[5] << std::endl;
        return 1;
    }

    std::string output = argc == 7 || argc == 8 ? static_cast<std::string>(argv[6]) : input;

    uint32_t maxFileLengthMs = 0;
    if (argc == 8)
    {
        if (!tryParse(argv[7], maxFileLengthMs))
        {
            std::cout << "Parameter 'maxFileLengthMs' could not be parsed. Please ensure it is a numeric value. Provided value: " << argv[7] << std::endl;
            return 1;
        }
    }

    WavFile wav(input, true);
    EQInstance lowPass(FilterType::LowPass, -24, 2000, 1, wav.getSampleRate());

    auto feedbackProcessor = [&lowPass](double in) { return lowPass.processSample(in); };

    FX::delay(wav, delayMilliseconds, feedback, mix, maxFileLengthMs, feedbackProcessor);
    wav.save(output);
    return 0;
}

/* analog-delay-stereo command*/
void Commands::analogDelayStereoHelp()
{
    std::cout << "analog-delay-stereo <input> <delayMillisecondsLeft> <feedbackLeft> <delayMillisecondsRight> <feedbackRight> <mix> [output] [maxOutputLengthMs]" << std::endl;
    std::cout << "\tdescription: applies a stereo analog style delay to the specified wave file, with frequency rolloff in the feedback loop" << std::endl;
    std::cout << "\tparameters:" << std::endl;
    std::cout << "\t\tinput: path to wave file" << std::endl;
    std::cout << "\t\tdelayMillisecondsLeft: delay time in milliseconds for the left channel" << std::endl;
    std::cout << "\t\tfeedbackLeft: feedback of the delay effect, from 0-100% for the left channel" << std::endl;
    std::cout << "\t\tdelayMillisecondsRight: delay time in milliseconds for the right channel" << std::endl;
    std::cout << "\t\tfeedbackRight: feedback of the delay effect, from 0-100% for the right channel" << std::endl;
    std::cout << "\t\tmix: mix of the delay effect vs dry signal, from 0-100%" << std::endl;
    std::cout << "\t\toutput: path to output file. Optional, if not specified input will be overwritten" << std::endl;
    std::cout << "\t\tmaxOutputLengthMs: the max length of the output file in milliseconds, to allow for feedback tail" << std::endl;
}

int Commands::analogDelayStereoCommand(const std::string& command, int argc, char* argv[])
{
    if (argc != 8 && argc != 9 && argc != 10)
    {
        Commands::printIncorrectArgumentsWarning(command);
        return 1;
    }
    std::string input = static_cast<std::string>(argv[2]);

    uint32_t delayMillisecondsLeft = 0;
    if (!tryParse(argv[3], delayMillisecondsLeft))
    {
        std::cout << "Parameter 'delayMillisecondsLeft' could not be parsed. Please ensure it is a numeric value. Provided value: " << argv[3] << std::endl;
        return 1;
    }

    double feedbackLeft = 0.0;
    if (!tryParse(argv[4], feedbackLeft))
    {
        std::cout << "Parameter 'feedbackLeft' could not be parsed. Please ensure it is a numeric value. Provided value: " << argv[4] << std::endl;
        return 1;
    }
    if (feedbackLeft < 0 || feedbackLeft > 100)
    {
        std::cout << "Parameter 'feedbackLeft' must be between 0 and 100. Provided value: " << argv[4] << std::endl;
        return 1;
    }

    uint32_t delayMillisecondsRight = 0;
    if (!tryParse(argv[5], delayMillisecondsRight))
    {
        std::cout << "Parameter 'delayMillisecondsRight' could not be parsed. Please ensure it is a numeric value. Provided value: " << argv[5] << std::endl;
        return 1;
    }

    double feedbackRight = 0.0;
    if (!tryParse(argv[6], feedbackRight))
    {
        std::cout << "Parameter 'feedbackRight' could not be parsed. Please ensure it is a numeric value. Provided value: " << argv[6] << std::endl;
        return 1;
    }
    if (feedbackRight < 0 || feedbackRight > 100)
    {
        std::cout << "Parameter 'feedbackRight' must be between 0 and 100. Provided value: " << argv[6] << std::endl;
        return 1;
    }

    double mix = 0.0;
    if (!tryParse(argv[7], mix))
    {
        std::cout << "Parameter 'mix' could not be parsed. Please ensure it is a numeric value. Provided value: " << argv[7] << std::endl;
        return 1;
    }
    if (mix < 0 || mix > 100)
    {
        std::cout << "Parameter 'mix' must be between 0 and 100. Provided value: " << argv[7] << std::endl;
        return 1;
    }

    std::string output = argc == 9 || argc == 10 ? static_cast<std::string>(argv[8]) : input;

    uint32_t maxFileLengthMs = 0;
    if (argc == 10)
    {
        if (!tryParse(argv[9], maxFileLengthMs))
        {
            std::cout << "Parameter 'maxFileLengthMs' could not be parsed. Please ensure it is a numeric value. Provided value: " << argv[9] << std::endl;
            return 1;
        }
    }

    WavFile wav(input, true);
    EQInstance lowPassL(FilterType::LowPass, -24, 2000, 1, wav.getSampleRate());
    EQInstance lowPassR(FilterType::LowPass, -24, 2000, 1, wav.getSampleRate());

    int count = 0;

    auto feedbackProcessor = [&lowPassL, &lowPassR, &count](double in) 
    { 
        double out = 0.0;
        if (count % 2 == 0)
        {
            out = lowPassL.processSample(in);
        }
        else 
        {
            out = lowPassR.processSample(in);
        }

        count++;
        return out;
    };

    FX::delay(wav, true, delayMillisecondsLeft, feedbackLeft, delayMillisecondsRight, feedbackRight, mix, maxFileLengthMs, feedbackProcessor);
    wav.save(output);
    return 0;
}

/* tape-delay-mono command*/
void Commands::tapeDelayMonoHelp()
{
    std::cout << "tape-delay-mono <input> <delayMilliseconds> <feedback> <mix> [output] [maxOutputLengthMs]" << std::endl;
    std::cout << "\tdescription: applies a mono tape style delay to the specified wave file, with frequency rolloff and tape saturation in the feedback loop. Warning: extremely prone to self oscilliation at higher feedbacks :)" << std::endl;
    std::cout << "\tparameters:" << std::endl;
    std::cout << "\t\tinput: path to wave file" << std::endl;
    std::cout << "\t\tdelayMilliseconds: delay time in milliseconds" << std::endl;
    std::cout << "\t\tfeedback: feedback of the delay effect, from 0-100%" << std::endl;
    std::cout << "\t\tmix: mix of the delay effect vs dry signal, from 0-100%" << std::endl;
    std::cout << "\t\toutput: path to output file. Optional, if not specified input will be overwritten" << std::endl;
    std::cout << "\t\tmaxOutputLengthMs: the max length of the output file in milliseconds, to allow for feedback tail" << std::endl;
}

int Commands::tapeDelayMonoCommand(const std::string& command, int argc, char* argv[])
{
    if (argc != 6 && argc != 7 && argc != 8)
    {
        Commands::printIncorrectArgumentsWarning(command);
        return 1;
    }
    std::string input = static_cast<std::string>(argv[2]);

    uint32_t delayMilliseconds = 0;
    if (!tryParse(argv[3], delayMilliseconds))
    {
        std::cout << "Parameter 'delayMilliseconds' could not be parsed. Please ensure it is a numeric value. Provided value: " << argv[3] << std::endl;
        return 1;
    }

    double feedback = 0.0;
    if (!tryParse(argv[4], feedback))
    {
        std::cout << "Parameter 'feedback' could not be parsed. Please ensure it is a numeric value. Provided value: " << argv[4] << std::endl;
        return 1;
    }
    if (feedback < 0 || feedback > 100)
    {
        std::cout << "Parameter 'feedback' must be between 0 and 100. Provided value: " << argv[4] << std::endl;
        return 1;
    }

    double mix = 0.0;
    if (!tryParse(argv[5], mix))
    {
        std::cout << "Parameter 'mix' could not be parsed. Please ensure it is a numeric value. Provided value: " << argv[5] << std::endl;
        return 1;
    }
    if (mix < 0 || mix > 100)
    {
        std::cout << "Parameter 'mix' must be between 0 and 100. Provided value: " << argv[5] << std::endl;
        return 1;
    }

    std::string output = argc == 7 || argc == 8 ? static_cast<std::string>(argv[6]) : input;

    uint32_t maxFileLengthMs = 0;
    if (argc == 8)
    {
        if (!tryParse(argv[7], maxFileLengthMs))
        {
            std::cout << "Parameter 'maxFileLengthMs' could not be parsed. Please ensure it is a numeric value. Provided value: " << argv[7] << std::endl;
            return 1;
        }
    }

    WavFile wav(input, true);
    EQInstance lowPass(FilterType::LowPass, -24, 1000, 1, wav.getSampleRate());

    auto feedbackProcessor = [&lowPass](double in) { return lowPass.processSample(FX::saturateSample(in, 0.55)); };

    FX::delay(wav, delayMilliseconds, feedback, mix, maxFileLengthMs, feedbackProcessor);
    wav.save(output);
    return 0;
}

/* tape-delay-stereo command*/
void Commands::tapeDelayStereoHelp()
{
    std::cout << "tape-delay-stereo <input> <delayMillisecondsLeft> <feedbackLeft> <delayMillisecondsRight> <feedbackRight> <mix> [output] [maxOutputLengthMs]" << std::endl;
    std::cout << "\tdescription: applies a stereo tape style delay to the specified wave file, with frequency rolloff and tape saturation in the feedback loop. Warning: extremely prone to self oscilliation at higher feedbacks :)" << std::endl;
    std::cout << "\tparameters:" << std::endl;
    std::cout << "\t\tinput: path to wave file" << std::endl;
    std::cout << "\t\tdelayMillisecondsLeft: delay time in milliseconds for the left channel" << std::endl;
    std::cout << "\t\tfeedbackLeft: feedback of the delay effect, from 0-100% for the left channel" << std::endl;
    std::cout << "\t\tdelayMillisecondsRight: delay time in milliseconds for the right channel" << std::endl;
    std::cout << "\t\tfeedbackRight: feedback of the delay effect, from 0-100% for the right channel" << std::endl;
    std::cout << "\t\tmix: mix of the delay effect vs dry signal, from 0-100%" << std::endl;
    std::cout << "\t\toutput: path to output file. Optional, if not specified input will be overwritten" << std::endl;
    std::cout << "\t\tmaxOutputLengthMs: the max length of the output file in milliseconds, to allow for feedback tail" << std::endl;
}

int Commands::tapeDelayStereoCommand(const std::string& command, int argc, char* argv[])
{
    if (argc != 8 && argc != 9 && argc != 10)
    {
        Commands::printIncorrectArgumentsWarning(command);
        return 1;
    }
    std::string input = static_cast<std::string>(argv[2]);

    uint32_t delayMillisecondsLeft = 0;
    if (!tryParse(argv[3], delayMillisecondsLeft))
    {
        std::cout << "Parameter 'delayMillisecondsLeft' could not be parsed. Please ensure it is a numeric value. Provided value: " << argv[3] << std::endl;
        return 1;
    }

    double feedbackLeft = 0.0;
    if (!tryParse(argv[4], feedbackLeft))
    {
        std::cout << "Parameter 'feedbackLeft' could not be parsed. Please ensure it is a numeric value. Provided value: " << argv[4] << std::endl;
        return 1;
    }
    if (feedbackLeft < 0 || feedbackLeft > 100)
    {
        std::cout << "Parameter 'feedbackLeft' must be between 0 and 100. Provided value: " << argv[4] << std::endl;
        return 1;
    }

    uint32_t delayMillisecondsRight = 0;
    if (!tryParse(argv[5], delayMillisecondsRight))
    {
        std::cout << "Parameter 'delayMillisecondsRight' could not be parsed. Please ensure it is a numeric value. Provided value: " << argv[5] << std::endl;
        return 1;
    }

    double feedbackRight = 0.0;
    if (!tryParse(argv[6], feedbackRight))
    {
        std::cout << "Parameter 'feedbackRight' could not be parsed. Please ensure it is a numeric value. Provided value: " << argv[6] << std::endl;
        return 1;
    }
    if (feedbackRight < 0 || feedbackRight > 100)
    {
        std::cout << "Parameter 'feedbackRight' must be between 0 and 100. Provided value: " << argv[6] << std::endl;
        return 1;
    }

    double mix = 0.0;
    if (!tryParse(argv[7], mix))
    {
        std::cout << "Parameter 'mix' could not be parsed. Please ensure it is a numeric value. Provided value: " << argv[7] << std::endl;
        return 1;
    }
    if (mix < 0 || mix > 100)
    {
        std::cout << "Parameter 'mix' must be between 0 and 100. Provided value: " << argv[7] << std::endl;
        return 1;
    }

    std::string output = argc == 9 || argc == 10 ? static_cast<std::string>(argv[8]) : input;

    uint32_t maxFileLengthMs = 0;
    if (argc == 10)
    {
        if (!tryParse(argv[9], maxFileLengthMs))
        {
            std::cout << "Parameter 'maxFileLengthMs' could not be parsed. Please ensure it is a numeric value. Provided value: " << argv[9] << std::endl;
            return 1;
        }
    }

    WavFile wav(input, true);
    EQInstance lowPassL(FilterType::LowPass, -24, 1000, 1, wav.getSampleRate());
    EQInstance lowPassR(FilterType::LowPass, -24, 1000, 1, wav.getSampleRate());

    int count = 0;

    auto feedbackProcessor = [&lowPassL, &lowPassR, &count](double in)
    {
        double out = 0.0;
        if (count % 2 == 0)
        {
            out = lowPassL.processSample(FX::saturateSample(in, 0.55));
        }
        else
        {
            out = lowPassR.processSample(FX::saturateSample(in, 0.55));
        }

        count++;
        return out;
    };

    FX::delay(wav, true, delayMillisecondsLeft, feedbackLeft, delayMillisecondsRight, feedbackRight, mix, maxFileLengthMs, feedbackProcessor);
    wav.save(output);
    return 0;
}

/* various delay overloads */

void FX::delay(WavFile& wave, uint32_t delayMs, double feedback, double mix, uint32_t maxFileLengthMs)
{
    //Passes to mono overload with passthrough feedback processor.
    delay(wave, delayMs, feedback, mix, maxFileLengthMs, [](double in) { return in; });
}

void FX::delay(WavFile& wave, uint32_t delayMsL, double feedbackL, uint32_t delayMsR, double feedbackR, double mix, uint32_t maxFileLengthMs)
{
    //Passes to stereo overload with passthrough feedback processor.
    delay(wave, true, delayMsL, feedbackL, delayMsR, feedbackR, mix, maxFileLengthMs, [](double in) { return in; });
}

void FX::delay(WavFile& wave, uint32_t delayMs, double feedback, double mix, uint32_t maxFileLengthMs, const std::function<double(double input)>& feedbackProcessor)
{
    //Passes to stereo overload with specified feedback processor.
    delay(wave, false, delayMs, feedback, delayMs, feedback, mix, maxFileLengthMs, feedbackProcessor);
}

/* full delay implementation that will work for all possible types */

void FX::delay(WavFile& wave, bool stereo, uint32_t delayMsL, double feedbackL, uint32_t delayMsR, double feedbackR, double mix, uint32_t maxFileLengthMs, const std::function<double(double input)>& feedbackProcessor)
{
    double scaledFeedbackL = feedbackL / 100.0;
    double scaledFeedbackR = feedbackR / 100.0;
    double scaledMix = mix / 100.0;
    std::vector<double> originalSamples = wave.getSamples();
    auto maxNumberOfSamples = static_cast<uint32_t>(maxFileLengthMs == 0 ? (originalSamples.size() / wave.getNumberOfChannels()) : wave.getSampleRate() * (maxFileLengthMs / 1000.0)); // NOLINT(cppcoreguidelines-narrowing-conversions,bugprone-integer-division)
    uint32_t currentNumberOfSamples = 0;
    double maxSample = 0.0;

    std::vector<double> newSamples;

    if (wave.getNumberOfChannels() == 1)
    {
        if (stereo)
        {
            Delay delayL(delayMsL * 2, delayMsL, wave.getSampleRate());
            Delay delayR(delayMsR * 2, delayMsR, wave.getSampleRate());

            for (auto& sample : originalSamples)
            {
                if (sample > maxSample)
                {
                    maxSample = sample;
                }
                newSamples.push_back(delayL.next(sample, scaledFeedbackL, feedbackProcessor, scaledMix));
                newSamples.push_back(delayR.next(sample, scaledFeedbackR, feedbackProcessor, scaledMix));
                currentNumberOfSamples++;
            }
            //Lengthen file for delay tail, up to max value provided
            while (currentNumberOfSamples < maxNumberOfSamples)
            {
                newSamples.push_back(delayL.next(0, scaledFeedbackL, feedbackProcessor, scaledMix));
                newSamples.push_back(delayR.next(0, scaledFeedbackR, feedbackProcessor, scaledMix));
                currentNumberOfSamples++;
            }
        }
        else 
        {
            Delay delay(delayMsL * 2, delayMsL, wave.getSampleRate());
            for (auto& sample : originalSamples)
            {
                if (sample > maxSample)
                {
                    maxSample = sample;
                }
                newSamples.push_back(delay.next(sample, scaledFeedbackL, feedbackProcessor, scaledMix));
                currentNumberOfSamples++;
            }
            //Lengthen file for delay tail, up to max value provided
            while (currentNumberOfSamples < maxNumberOfSamples)
            {
                newSamples.push_back(delay.next(0, scaledFeedbackL, feedbackProcessor, scaledMix));
                currentNumberOfSamples++;
            }
        }

    }
    else if(wave.getNumberOfChannels() == 2)
    {
        Delay delayL(delayMsL * 2, delayMsL, wave.getSampleRate());
        Delay delayR(delayMsR * 2, delayMsR, wave.getSampleRate());

        for (size_t i = 0; i < originalSamples.size() - 1; i = i + 2)
        {
            double left = originalSamples[i];
            double right = originalSamples[i + 1];

            if (left > maxSample)
            {
                maxSample = left;
            }
            if (right > maxSample)
            {
                maxSample = right;
            }

            newSamples.push_back(delayL.next(left, scaledFeedbackL, feedbackProcessor, scaledMix));
            newSamples.push_back(delayR.next(right, scaledFeedbackR, feedbackProcessor, scaledMix));
            currentNumberOfSamples++;
        }
        //Lengthen sample for delay tail, up to max value provided
        while (currentNumberOfSamples < maxNumberOfSamples)
        {
            newSamples.push_back(delayL.next(0, scaledFeedbackL, feedbackProcessor, scaledMix));
            newSamples.push_back(delayR.next(0, scaledFeedbackR, feedbackProcessor,scaledMix));
            currentNumberOfSamples++;
        }
    }
    //Makes sure a mono input file becomes stereo if stereo delay applied
    wave.setNumberOfChannels(stereo || wave.getNumberOfChannels() == 2 ? 2 : 1);

    wave.setSamples(newSamples);

    //Ensures input and output are the same level when mix = 50%
    Tools::normalise(wave, maxSample);
}