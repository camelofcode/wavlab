#include <iostream>
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
        std::cout << "\tdescription: shows the command list" << std::endl;

        std::cout << "stat <input>" << std::endl;
        std::cout << "\tdescription: show stats of a wave file" << std::endl;
        std::cout << "\tparameters:" << std::endl;
        std::cout << "\t\tinput: path to wave file" << std::endl;

        std::cout << "normalise <input> [output]" << std::endl;
        std::cout << "\tdescription: normalises a wave file" << std::endl;
        std::cout << "\tparameters:" << std::endl;
        std::cout << "\t\tinput: path to wave file" << std::endl;
        std::cout << "\t\toutput: path to output file. Optional, if not specified input will be overwritten" << std::endl;

        std::cout << "reverse <input> [output]" << std::endl;
        std::cout << "\tdescription: reverses a wave file" << std::endl;
        std::cout << "\tparameters:" << std::endl;
        std::cout << "\t\tinput: path to wave file" << std::endl;
        std::cout << "\t\toutput: path to output file. Optional, if not specified input will be overwritten" << std::endl;

        std::cout << "flip-phase <input> [output]" << std::endl;
        std::cout << "\tdescription: reverses the phase of a wave file" << std::endl;
        std::cout << "\tparameters:" << std::endl;
        std::cout << "\t\tinput: path to wave file" << std::endl;
        std::cout << "\t\toutput: path to output file. Optional, if not specified input will be overwritten" << std::endl;

        std::cout << "swap-channels <input> [output]" << std::endl;
        std::cout << "\tdescription: swaps the left and right channels of a stereo wave file" << std::endl;
        std::cout << "\tparameters:" << std::endl;
        std::cout << "\t\tinput: path to wave file" << std::endl;
        std::cout << "\t\toutput: path to output file. Optional, if not specified input will be overwritten" << std::endl;

        std::cout << "get-peak-db <input>" << std::endl;
        std::cout << "\tdescription: shows the peak dB level of a wave file" << std::endl;
        std::cout << "\tparameters:" << std::endl;
        std::cout << "\t\tinput: path to wave file" << std::endl;

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

    /*
    TODO:
        gain
        gain-db
        sine-wave
        saturate
    */

    return 0;
}
