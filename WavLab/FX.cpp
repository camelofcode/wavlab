#define _USE_MATH_DEFINES
#include "FX.h"
#include "Commands.h"
#include <cmath>

/* saturate command */
void Commands::saturateHelp()
{
    std::cout << "saturate <input> <drive> [output]" << std::endl;
    std::cout << "\tdescription: applies a tape saturation effect to a wave file" << std::endl;
    std::cout << "\tparameters:" << std::endl;
    std::cout << "\t\tinput: path to wave file" << std::endl;
    std::cout << "\t\tdrive: increase for dirtier sound" << std::endl;
    std::cout << "\t\toutput: path to output file. Optional, if not specified input will be overwritten" << std::endl << std::endl;
}

int Commands::saturateCommand(std::string command, int argc, char* argv[])
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

void FX::saturate(WavFile& wave, double drive)
{
	//At one point (2009) this algorithm existed at http://tap-plugins.sourceforge.net/ladspa/sigmoid.html

    double gainFactor = (drive + 0.7) * 1.2;

    std::vector<double> newSamples;

    for (auto& sample : wave.getSamples())
    {
        double preSaturation = sample * gainFactor;
        double saturated = 2 / (1 + (pow(M_E, (-5.0 * preSaturation)))) - 1;
        newSamples.push_back(saturated);
    }
    wave.setSamples(newSamples);
}