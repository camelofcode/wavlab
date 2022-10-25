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
        exit(0);
    }

    std::string command = static_cast<std::string>(argv[1]);

    if (command == "help")
    {
        std::cout << "wavlab commands" << std::endl << std::endl;

        std::cout << "help" << " : show the command list" << std::endl;

        std::cout << "stat [inpath]" << " : show stats of a wave file" << std::endl;
        std::cout << "\tinpath: path to wave file" << std::endl;

        std::cout << "stat [inpath]" << " : show stats of a wave file" << std::endl;
        std::cout << "\tinpath: path to wave file" << std::endl;
    }
    else if (command == "stat")
    {
        if (argc != 3)
        {
            std::cout << "Incorrect number of arguments for command 'stat'. To see available commands run 'wavlab help'" << std::endl;
        }
        std::string path = static_cast<std::string>(argv[2]);
        WavFile wav(path, false);
        wav.printDetails();
    }

    /*
    TODO:
        normalise
        reverse
        flipphase
        swapchannels
        gain
        gaindb
        getpeakdb
        sinewave
        saturate
    */

    return 0;
}
