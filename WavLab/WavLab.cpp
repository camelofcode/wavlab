#include <iostream>
#include "TryParse.h"
#include "WavFile.h"
#include "Generators.h"
#include "Tools.h"
#include "FX.h"

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        std::cout << "No command specified. To see available commands run 'wavlab help'" << std::endl;
        return 1;
    }

    std::string command = static_cast<std::string>(argv[1]);

    if (command == "help")
    {
        std::cout << "wavlab commands" << std::endl << std::endl;

        std::cout << "help" << std::endl;
        std::cout << "\tdescription: shows the command list" << std::endl << std::endl;

        std::cout << "stat <input>" << std::endl;
        std::cout << "\tdescription: show stats of a wave file" << std::endl;
        std::cout << "\tparameters:" << std::endl;
        std::cout << "\t\tinput: path to wave file" << std::endl << std::endl;

        std::cout << "normalise <input> [output]" << std::endl;
        std::cout << "\tdescription: normalises a wave file" << std::endl;
        std::cout << "\tparameters:" << std::endl;
        std::cout << "\t\tinput: path to wave file" << std::endl;
        std::cout << "\t\toutput: path to output file. Optional, if not specified input will be overwritten" << std::endl << std::endl;

        std::cout << "reverse <input> [output]" << std::endl;
        std::cout << "\tdescription: reverses a wave file" << std::endl;
        std::cout << "\tparameters:" << std::endl;
        std::cout << "\t\tinput: path to wave file" << std::endl;
        std::cout << "\t\toutput: path to output file. Optional, if not specified input will be overwritten" << std::endl << std::endl;

        std::cout << "flip-phase <input> [output]" << std::endl;
        std::cout << "\tdescription: reverses the phase of a wave file" << std::endl;
        std::cout << "\tparameters:" << std::endl;
        std::cout << "\t\tinput: path to wave file" << std::endl;
        std::cout << "\t\toutput: path to output file. Optional, if not specified input will be overwritten" << std::endl << std::endl;

        std::cout << "swap-channels <input> [output]" << std::endl;
        std::cout << "\tdescription: swaps the left and right channels of a stereo wave file" << std::endl;
        std::cout << "\tparameters:" << std::endl;
        std::cout << "\t\tinput: path to wave file" << std::endl;
        std::cout << "\t\toutput: path to output file. Optional, if not specified input will be overwritten" << std::endl << std::endl;

        std::cout << "get-peak-db <input>" << std::endl;
        std::cout << "\tdescription: shows the peak dB level of a wave file" << std::endl;
        std::cout << "\tparameters:" << std::endl;
        std::cout << "\t\tinput: path to wave file" << std::endl << std::endl;

        std::cout << "gain <input> <gain> [output]" << std::endl;
        std::cout << "\tdescription: applies gain to a wave file" << std::endl;
        std::cout << "\tparameters:" << std::endl;
        std::cout << "\t\tinput: path to wave file" << std::endl;
        std::cout << "\t\tgain: gain multiplier" << std::endl;
        std::cout << "\t\toutput: path to output file. Optional, if not specified input will be overwritten" << std::endl << std::endl;

        std::cout << "gain-db <input> <dB> [output]" << std::endl;
        std::cout << "\tdescription: applies gain to a wave file, with input in decibels" << std::endl;
        std::cout << "\tparameters:" << std::endl;
        std::cout << "\t\tinput: path to wave file" << std::endl;
        std::cout << "\t\tdB: gain to apply in decibels" << std::endl;
        std::cout << "\t\toutput: path to output file. Optional, if not specified input will be overwritten" << std::endl << std::endl;

        std::cout << "saturate <input> <drive> [output]" << std::endl;
        std::cout << "\tdescription: applies a tape saturation effect to a wave file" << std::endl;
        std::cout << "\tparameters:" << std::endl;
        std::cout << "\t\tinput: path to wave file" << std::endl;
        std::cout << "\t\tdrive: increase for dirtier sound" << std::endl;
        std::cout << "\t\toutput: path to output file. Optional, if not specified input will be overwritten" << std::endl << std::endl;

        std::cout << "sine-wave <output> <frequency> <sampleRate> <bitDepth> <length>" << std::endl;
        std::cout << "\tdescription: writes a sine wave to the specified output file" << std::endl;
        std::cout << "\tparameters:" << std::endl;
        std::cout << "\t\toutput: path to output wave file" << std::endl;
        std::cout << "\t\tfrequency: frequency in Hz of the sine wave" << std::endl;
        std::cout << "\t\tsampleRate: sample rate of the output wave file" << std::endl;
        std::cout << "\t\tbitDepth: bit depth of the output wave file" << std::endl;
        std::cout << "\t\tlength: length of the output file in milliseconds" << std::endl << std::endl;

        return 0;
    }
    else if (command == "stat")
    {
        if (argc != 3)
        {
            std::cout << "Incorrect number of arguments for command 'stat'. To see available commands run 'wavlab help'" << std::endl;
            return 1;
        }
        std::string path = static_cast<std::string>(argv[2]);
        WavFile wav(path, false);
        wav.printDetails();
        return 0;
    }
    else if (command == "normalise")
    {
        if (argc != 3 && argc != 4)
        {
            std::cout << "Incorrect number of arguments for command 'normalise'. To see available commands run 'wavlab help'" << std::endl;
            return 1;
        }
        std::string input = static_cast<std::string>(argv[2]);
        std::string output = argc == 4 ? static_cast<std::string>(argv[3]) : input;

        WavFile wav(input, true);
        Tools::normalise(wav);
        wav.save(output);
        return 0;
    }
    else if (command == "reverse")
    {
        if (argc != 3 && argc != 4)
        {
            std::cout << "Incorrect number of arguments for command 'reverse'. To see available commands run 'wavlab help'" << std::endl;
            return 1;
        }
        std::string input = static_cast<std::string>(argv[2]);
        std::string output = argc == 4 ? static_cast<std::string>(argv[3]) : input;

        WavFile wav(input, true);
        Tools::reverse(wav);
        wav.save(output);
        return 0;
    }
    else if (command == "flip-phase")
    {
        if (argc != 3 && argc != 4)
        {
            std::cout << "Incorrect number of arguments for command 'flip-phase'. To see available commands run 'wavlab help'" << std::endl;
            return 1;
        }
        std::string input = static_cast<std::string>(argv[2]);
        std::string output = argc == 4 ? static_cast<std::string>(argv[3]) : input;

        WavFile wav(input, true);
        Tools::flipPhase(wav);
        wav.save(output);
        return 0;
    }
    else if (command == "swap-channels")
    {
        if (argc != 3 && argc != 4)
        {
            std::cout << "Incorrect number of arguments for command 'swap-channels'. To see available commands run 'wavlab help'" << std::endl;
            return 1;
        }
        std::string input = static_cast<std::string>(argv[2]);
        std::string output = argc == 4 ? static_cast<std::string>(argv[3]) : input;

        WavFile wav(input, true);
        Tools::swapChannels(wav);
        wav.save(output);
        return 0;
    }
    else if (command == "get-peak-db")
    {
        if (argc != 3)
        {
            std::cout << "Incorrect number of arguments for command 'get-peak-db'. To see available commands run 'wavlab help'" << std::endl;
            return 1;
        }
        std::string input = static_cast<std::string>(argv[2]);

        WavFile wav(input, true);
        std::cout << Tools::getPeakDecibels(wav) << "dB";
        return 0;
    }
    else if (command == "gain")
    {
        if (argc != 4 && argc != 5)
        {
            std::cout << "Incorrect number of arguments for command 'gain'. To see available commands run 'wavlab help'" << std::endl;
            return 1;
        }
        std::string input = static_cast<std::string>(argv[2]);

        double gain = 0.0;
        if (!tryParse(argv[3], gain))
        {
            std::cout << "Parameter 'gain' could not be parsed. Please ensure it is a numeric value. Provided value: " << argv[3] << std::endl;
            return 1;
        }

        std::string output = argc == 5 ? static_cast<std::string>(argv[4]) : input;

        WavFile wav(input, true);
        Tools::gain(wav, gain);
        wav.save(output);
        return 0;
    }
    else if (command == "gain-db")
    {
        if (argc != 4 && argc != 5)
        {
            std::cout << "Incorrect number of arguments for command 'gain-db'. To see available commands run 'wavlab help'" << std::endl;
            return 1;
        }
        std::string input = static_cast<std::string>(argv[2]);

        double dB = 0.0;
        if (!tryParse(argv[3], dB))
        {
            std::cout << "Parameter 'db' could not be parsed. Please ensure it is a numeric value. Provided value: " << argv[3] << std::endl;
            return 1;
        }

        std::string output = argc == 5 ? static_cast<std::string>(argv[4]) : input;

        WavFile wav(input, true);
        Tools::decibelGain(wav, dB);
        wav.save(output);
        return 0;
    }
    else if (command == "saturate")
    {
        if (argc != 4 && argc != 5)
        {
            std::cout << "Incorrect number of arguments for command 'saturate'. To see available commands run 'wavlab help'" << std::endl;
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
    else if (command == "sine-wave")
    {
        if (argc != 7)
        {
            std::cout << "Incorrect number of arguments for command 'saturate'. To see available commands run 'wavlab help'" << std::endl;
            return 1;
        }
        std::string output = static_cast<std::string>(argv[2]);

        double frequency = 44100;
        if (!tryParse(argv[3], frequency))
        {
            std::cout << "Parameter 'frequency' could not be parsed. Please ensure it is a numeric value. Provided value: " << argv[3] << std::endl;
            return 1;
        }

        uint32_t sampleRate = 44100;
        if (!tryParse(argv[4], sampleRate))
        {
            std::cout << "Parameter 'sampleRate' could not be parsed. Please ensure it is a numeric value. Provided value: " << argv[4] << std::endl;
            return 1;
        }

        unsigned short bitDepth = 16;
        if (!tryParse(argv[5], bitDepth))
        {
            std::cout << "Parameter 'bitDepth' could not be parsed. Please ensure it is a numeric value. Provided value: " << argv[5] << std::endl;
            return 1;
        }

        int lengthMs = 1000;
        if (!tryParse(argv[6], lengthMs))
        {
            std::cout << "Parameter 'lengthMs' could not be parsed. Please ensure it is a numeric value. Provided value: " << argv[6] << std::endl;
            return 1;
        }

        WavFile wav(1, sampleRate, bitDepth);
        Generators::sineWave(wav, lengthMs, frequency);
        wav.save(output);

        return 0;
    }

    return 0;
}
