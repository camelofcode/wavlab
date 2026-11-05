//This file is an implementation of the praiseworthy "Cookbook formulae for audio EQ biquad filter coefficients" by Robert Bristow-Johnson <rbj@audioimagination.com>
//See https://webaudio.github.io/Audio-EQ-Cookbook/Audio-EQ-Cookbook.txt , https://webaudio.github.io/Audio-EQ-Cookbook/audio-eq-cookbook.html
//My implementation is however taken mostly from https://www.musicdsp.org/en/latest/Filters/128-c-class-implementation-of-rbj-filters.html

#include <wavlab/EQ.h>

#include "Commands.h"

using namespace wavlab;

void Commands::eqPeakHelp()
{
	std::cout << "eq-peak <input> <frequency> <gain> <q> [output]" << std::endl;
	std::cout << "\tdescription: applies a positive or negative peak EQ to a wave file" << std::endl;
	std::cout << "\tparameters:" << std::endl;
	std::cout << "\t\tinput: path to wave file" << std::endl;
	std::cout << "\t\tfrequency: frequency in Hz of the cut/boost" << std::endl;
	std::cout << "\t\tgain: gain in dB to cut/boost" << std::endl;
	std::cout << "\t\tq: q factor of the filter" << std::endl;
	std::cout << "\t\toutput: path to output file. Optional, if not specified input will be overwritten" << std::endl;
}
void Commands::eqLowShelfHelp()
{
	std::cout << "eq-lowshelf <input> <frequency> <gain> <q> [output]" << std::endl;
	std::cout << "\tdescription: applies a positive or negative low shelf EQ to a wave file" << std::endl;
	std::cout << "\tparameters:" << std::endl;
	std::cout << "\t\tinput: path to wave file" << std::endl;
	std::cout << "\t\tfrequency: frequency in Hz of the start of the shelf" << std::endl;
	std::cout << "\t\tgain: gain in dB to cut/boost" << std::endl;
	std::cout << "\t\tq: q factor of the filter" << std::endl;
	std::cout << "\t\toutput: path to output file. Optional, if not specified input will be overwritten" << std::endl;
}

void Commands::eqHighShelfHelp()
{
	std::cout << "eq-highshelf <input> <frequency> <gain> <q> [output]" << std::endl;
	std::cout << "\tdescription: applies a positive or negative high shelf EQ to a wave file" << std::endl;
	std::cout << "\tparameters:" << std::endl;
	std::cout << "\t\tinput: path to wave file" << std::endl;
	std::cout << "\t\tfrequency: frequency in Hz of the start of the shelf" << std::endl;
	std::cout << "\t\tgain: gain in dB to cut/boost" << std::endl;
	std::cout << "\t\tq: q factor of the filter" << std::endl;
	std::cout << "\t\toutput: path to output file. Optional, if not specified input will be overwritten" << std::endl;
}

int Commands::eqWithGainCommand(const std::string& command, int argc, char* argv[])
{
	if (argc != 6 && argc != 7)
	{
        Commands::printIncorrectArgumentsWarning(command);
		return EXIT_FAILURE;
	}

	std::string input = static_cast<std::string>(argv[2]);

	double frequency = 1000;
	if (!tryParse(argv[3], frequency))
	{
		std::cerr << "Parameter 'frequency' could not be parsed. Please ensure it is a numeric value. Provided value: " << argv[3] << std::endl;
		return EXIT_FAILURE;
	}

	double gain = 6;
	if (!tryParse(argv[4], gain))
	{
		std::cerr << "Parameter 'gain' could not be parsed. Please ensure it is a numeric value. Provided value: " << argv[4] << std::endl;
		return EXIT_FAILURE;
	}

	double q = 1;
	if (!tryParse(argv[5], q))
	{
		std::cerr << "Parameter 'q' could not be parsed. Please ensure it is a numeric value. Provided value: " << argv[5] << std::endl;
		return EXIT_FAILURE;
	}

	std::string output = argc == 7 ? static_cast<std::string>(argv[6]) : input;

	FilterType type = FilterType::None;
	if (command == "eq-peak") type = FilterType::Peak;
	if (command == "eq-lowshelf") type = FilterType::LowShelf;
	if (command == "eq-highshelf") type = FilterType::HighShelf;

	WavFile wav(input, true);
	EQ::process(wav, type, gain, frequency, q);
	wav.save(output);

	return EXIT_SUCCESS;
}

void Commands::eqAllPassHelp()
{
	std::cout << "eq-allpass <input> <frequency> <q> [output]" << std::endl;
	std::cout << "\tdescription: applies an all pass filter to a wave file" << std::endl;
	std::cout << "\tparameters:" << std::endl;
	std::cout << "\t\tinput: path to wave file" << std::endl;
	std::cout << "\t\tfrequency: centre frequency of the filter in Hz" << std::endl;
	std::cout << "\t\tq: q factor of the filter" << std::endl;
	std::cout << "\t\toutput: path to output file. Optional, if not specified input will be overwritten" << std::endl;
}

void Commands::eqBandPassHelp()
{
	std::cout << "eq-bandpass <input> <frequency> <q> [output]" << std::endl;
	std::cout << "\tdescription: applies a band pass filter to a wave file" << std::endl;
	std::cout << "\tparameters:" << std::endl;
	std::cout << "\t\tinput: path to wave file" << std::endl;
	std::cout << "\t\tfrequency: centre frequency of the filter in Hz" << std::endl;
	std::cout << "\t\tq: q factor of the filter" << std::endl;
	std::cout << "\t\toutput: path to output file. Optional, if not specified input will be overwritten" << std::endl;
}

void Commands::eqLowPassHelp()
{
	std::cout << "eq-lowpass <input> <frequency> <q> [output]" << std::endl;
	std::cout << "\tdescription: applies a low pass filter to a wave file" << std::endl;
	std::cout << "\tparameters:" << std::endl;
	std::cout << "\t\tinput: path to wave file" << std::endl;
	std::cout << "\t\tfrequency: centre frequency of the filter in Hz" << std::endl;
	std::cout << "\t\tq: q factor of the filter" << std::endl;
	std::cout << "\t\toutput: path to output file. Optional, if not specified input will be overwritten" << std::endl;
}

void Commands::eqHighPassHelp()
{
	std::cout << "eq-highpass <input> <frequency> <q> [output]" << std::endl;
	std::cout << "\tdescription: applies a high pass filter to a wave file" << std::endl;
	std::cout << "\tparameters:" << std::endl;
	std::cout << "\t\tinput: path to wave file" << std::endl;
	std::cout << "\t\tfrequency: centre frequency of the filter in Hz" << std::endl;
	std::cout << "\t\tq: q factor of the filter" << std::endl;
	std::cout << "\t\toutput: path to output file. Optional, if not specified input will be overwritten" << std::endl;
}

int Commands::eqPassCommand(const std::string& command, int argc, char* argv[])
{
	if (argc != 5 && argc != 6)
	{
        Commands::printIncorrectArgumentsWarning(command);
		return EXIT_FAILURE;
	}

	std::string input = static_cast<std::string>(argv[2]);

	double frequency = 1000;
	if (!tryParse(argv[3], frequency))
	{
		std::cerr << "Parameter 'frequency' could not be parsed. Please ensure it is a numeric value. Provided value: " << argv[3] << std::endl;
		return EXIT_FAILURE;
	}

	double q = 1;
	if (!tryParse(argv[4], q))
	{
		std::cerr << "Parameter 'q' could not be parsed. Please ensure it is a numeric value. Provided value: " << argv[5] << std::endl;
		return EXIT_FAILURE;
	}

	std::string output = argc == 6 ? static_cast<std::string>(argv[5]) : input;

	FilterType type = FilterType::None;
	if (command == "eq-allpass") type = FilterType::AllPass;
	if (command == "eq-bandpass") type = FilterType::BandPass;
	if (command == "eq-lowpass") type = FilterType::LowPass;
	if (command == "eq-highpass") type = FilterType::HighPass;

	WavFile wav(input, true);
	EQ::process(wav, type, 0, frequency, q);
	wav.save(output);

	return EXIT_SUCCESS;
}
