#include <wavlab/FX.h>
#include <wavlab/EQ.h>

#include "Commands.h"
#include <cmath>

using namespace wavlab;

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
		return EXIT_FAILURE;
	}
	std::string input = static_cast<std::string>(argv[2]);

	double drive = 0.0;
	if (!tryParse(argv[3], drive))
	{
		std::cerr << "Parameter 'drive' could not be parsed. Please ensure it is a numeric value. Provided value: " << argv[3] << std::endl;
		return EXIT_FAILURE;
	}

	std::string output = argc == 5 ? static_cast<std::string>(argv[4]) : input;

	auto wav = WavFile::open(input);
	FX::saturate(wav, drive);
	wav.save(output);
	return EXIT_SUCCESS;
}

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
		return EXIT_FAILURE;
	}
	std::string input = static_cast<std::string>(argv[2]);

	uint32_t delayMilliseconds = 0;
	if (!tryParse(argv[3], delayMilliseconds))
	{
		std::cerr << "Parameter 'delayMilliseconds' could not be parsed. Please ensure it is a numeric value. Provided value: " << argv[3] << std::endl;
		return EXIT_FAILURE;
	}

	double feedback = 0.0;
	if (!tryParse(argv[4], feedback))
	{
		std::cerr << "Parameter 'feedback' could not be parsed. Please ensure it is a numeric value. Provided value: " << argv[4] << std::endl;
		return EXIT_FAILURE;
	}
	if (feedback < 0 || feedback > 100)
	{
		std::cerr << "Parameter 'feedback' must be between 0 and 100. Provided value: " << argv[4] << std::endl;
		return EXIT_FAILURE;
	}

	double mix = 0.0;
	if (!tryParse(argv[5], mix))
	{
		std::cerr << "Parameter 'mix' could not be parsed. Please ensure it is a numeric value. Provided value: " << argv[5] << std::endl;
		return EXIT_FAILURE;
	}
	if (mix < 0 || mix > 100)
	{
		std::cerr << "Parameter 'mix' must be between 0 and 100. Provided value: " << argv[5] << std::endl;
		return EXIT_FAILURE;
	}

	std::string output = argc == 7 || argc == 8 ? static_cast<std::string>(argv[6]) : input;

	uint32_t maxFileLengthMs = 0;
	if (argc == 8)
	{
		if (!tryParse(argv[7], maxFileLengthMs))
		{
			std::cerr << "Parameter 'maxFileLengthMs' could not be parsed. Please ensure it is a numeric value. Provided value: " << argv[7] << std::endl;
			return EXIT_FAILURE;
		}
	}

	auto wav = WavFile::open(input);
	FX::delay(wav, DelaySettings{{delayMilliseconds, feedback}, mix, maxFileLengthMs});
	wav.save(output);
	return EXIT_SUCCESS;
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
		return EXIT_FAILURE;
	}
	std::string input = static_cast<std::string>(argv[2]);

	uint32_t delayMillisecondsLeft = 0;
	if (!tryParse(argv[3], delayMillisecondsLeft))
	{
		std::cerr << "Parameter 'delayMillisecondsLeft' could not be parsed. Please ensure it is a numeric value. Provided value: " << argv[3] << std::endl;
		return EXIT_FAILURE;
	}

	double feedbackLeft = 0.0;
	if (!tryParse(argv[4], feedbackLeft))
	{
		std::cerr << "Parameter 'feedbackLeft' could not be parsed. Please ensure it is a numeric value. Provided value: " << argv[4] << std::endl;
		return EXIT_FAILURE;
	}
	if (feedbackLeft < 0 || feedbackLeft > 100)
	{
		std::cerr << "Parameter 'feedbackLeft' must be between 0 and 100. Provided value: " << argv[4] << std::endl;
		return EXIT_FAILURE;
	}

	uint32_t delayMillisecondsRight = 0;
	if (!tryParse(argv[5], delayMillisecondsRight))
	{
		std::cerr << "Parameter 'delayMillisecondsRight' could not be parsed. Please ensure it is a numeric value. Provided value: " << argv[5] << std::endl;
		return EXIT_FAILURE;
	}

	double feedbackRight = 0.0;
	if (!tryParse(argv[6], feedbackRight))
	{
		std::cerr << "Parameter 'feedbackRight' could not be parsed. Please ensure it is a numeric value. Provided value: " << argv[6] << std::endl;
		return EXIT_FAILURE;
	}
	if (feedbackRight < 0 || feedbackRight > 100)
	{
		std::cerr << "Parameter 'feedbackRight' must be between 0 and 100. Provided value: " << argv[6] << std::endl;
		return EXIT_FAILURE;
	}

	double mix = 0.0;
	if (!tryParse(argv[7], mix))
	{
		std::cerr << "Parameter 'mix' could not be parsed. Please ensure it is a numeric value. Provided value: " << argv[7] << std::endl;
		return EXIT_FAILURE;
	}
	if (mix < 0 || mix > 100)
	{
		std::cerr << "Parameter 'mix' must be between 0 and 100. Provided value: " << argv[7] << std::endl;
		return EXIT_FAILURE;
	}

	std::string output = argc == 9 || argc == 10 ? static_cast<std::string>(argv[8]) : input;

	uint32_t maxFileLengthMs = 0;
	if (argc == 10)
	{
		if (!tryParse(argv[9], maxFileLengthMs))
		{
			std::cerr << "Parameter 'maxFileLengthMs' could not be parsed. Please ensure it is a numeric value. Provided value: " << argv[9] << std::endl;
			return EXIT_FAILURE;
		}
	}

	auto wav = WavFile::open(input);
	FX::delay(
		wav,
		StereoDelaySettings{
			{delayMillisecondsLeft, feedbackLeft},
			{delayMillisecondsRight, feedbackRight},
			mix,
			maxFileLengthMs});
	wav.save(output);
	return EXIT_SUCCESS;
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
		return EXIT_FAILURE;
	}
	std::string input = static_cast<std::string>(argv[2]);

	uint32_t delayMilliseconds = 0;
	if (!tryParse(argv[3], delayMilliseconds))
	{
		std::cerr << "Parameter 'delayMilliseconds' could not be parsed. Please ensure it is a numeric value. Provided value: " << argv[3] << std::endl;
		return EXIT_FAILURE;
	}

	double feedback = 0.0;
	if (!tryParse(argv[4], feedback))
	{
		std::cerr << "Parameter 'feedback' could not be parsed. Please ensure it is a numeric value. Provided value: " << argv[4] << std::endl;
		return EXIT_FAILURE;
	}
	if (feedback < 0 || feedback > 100)
	{
		std::cerr << "Parameter 'feedback' must be between 0 and 100. Provided value: " << argv[4] << std::endl;
		return EXIT_FAILURE;
	}

	double mix = 0.0;
	if (!tryParse(argv[5], mix))
	{
		std::cerr << "Parameter 'mix' could not be parsed. Please ensure it is a numeric value. Provided value: " << argv[5] << std::endl;
		return EXIT_FAILURE;
	}
	if (mix < 0 || mix > 100)
	{
		std::cerr << "Parameter 'mix' must be between 0 and 100. Provided value: " << argv[5] << std::endl;
		return EXIT_FAILURE;
	}

	std::string output = argc == 7 || argc == 8 ? static_cast<std::string>(argv[6]) : input;

	uint32_t maxFileLengthMs = 0;
	if (argc == 8)
	{
		if (!tryParse(argv[7], maxFileLengthMs))
		{
			std::cerr << "Parameter 'maxFileLengthMs' could not be parsed. Please ensure it is a numeric value. Provided value: " << argv[7] << std::endl;
			return EXIT_FAILURE;
		}
	}

	auto wav = WavFile::open(input);
	EQInstance lowPass(FilterType::LowPass, -24, 2000, 1, wav.sampleRate());

	auto feedbackProcessor = [&lowPass](double in) { return lowPass.processSample(in); };

	FX::delay(
		wav,
		DelaySettings{{delayMilliseconds, feedback}, mix, maxFileLengthMs},
		feedbackProcessor);
	wav.save(output);
	return EXIT_SUCCESS;
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
		return EXIT_FAILURE;
	}
	std::string input = static_cast<std::string>(argv[2]);

	uint32_t delayMillisecondsLeft = 0;
	if (!tryParse(argv[3], delayMillisecondsLeft))
	{
		std::cerr << "Parameter 'delayMillisecondsLeft' could not be parsed. Please ensure it is a numeric value. Provided value: " << argv[3] << std::endl;
		return EXIT_FAILURE;
	}

	double feedbackLeft = 0.0;
	if (!tryParse(argv[4], feedbackLeft))
	{
		std::cerr << "Parameter 'feedbackLeft' could not be parsed. Please ensure it is a numeric value. Provided value: " << argv[4] << std::endl;
		return EXIT_FAILURE;
	}
	if (feedbackLeft < 0 || feedbackLeft > 100)
	{
		std::cerr << "Parameter 'feedbackLeft' must be between 0 and 100. Provided value: " << argv[4] << std::endl;
		return EXIT_FAILURE;
	}

	uint32_t delayMillisecondsRight = 0;
	if (!tryParse(argv[5], delayMillisecondsRight))
	{
		std::cerr << "Parameter 'delayMillisecondsRight' could not be parsed. Please ensure it is a numeric value. Provided value: " << argv[5] << std::endl;
		return EXIT_FAILURE;
	}

	double feedbackRight = 0.0;
	if (!tryParse(argv[6], feedbackRight))
	{
		std::cerr << "Parameter 'feedbackRight' could not be parsed. Please ensure it is a numeric value. Provided value: " << argv[6] << std::endl;
		return EXIT_FAILURE;
	}
	if (feedbackRight < 0 || feedbackRight > 100)
	{
		std::cerr << "Parameter 'feedbackRight' must be between 0 and 100. Provided value: " << argv[6] << std::endl;
		return EXIT_FAILURE;
	}

	double mix = 0.0;
	if (!tryParse(argv[7], mix))
	{
		std::cerr << "Parameter 'mix' could not be parsed. Please ensure it is a numeric value. Provided value: " << argv[7] << std::endl;
		return EXIT_FAILURE;
	}
	if (mix < 0 || mix > 100)
	{
		std::cerr << "Parameter 'mix' must be between 0 and 100. Provided value: " << argv[7] << std::endl;
		return EXIT_FAILURE;
	}

	std::string output = argc == 9 || argc == 10 ? static_cast<std::string>(argv[8]) : input;

	uint32_t maxFileLengthMs = 0;
	if (argc == 10)
	{
		if (!tryParse(argv[9], maxFileLengthMs))
		{
			std::cerr << "Parameter 'maxFileLengthMs' could not be parsed. Please ensure it is a numeric value. Provided value: " << argv[9] << std::endl;
			return EXIT_FAILURE;
		}
	}

	auto wav = WavFile::open(input);
	EQInstance lowPassL(FilterType::LowPass, -24, 2000, 1, wav.sampleRate());
	EQInstance lowPassR(FilterType::LowPass, -24, 2000, 1, wav.sampleRate());

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

	FX::delay(
		wav,
		StereoDelaySettings{
			{delayMillisecondsLeft, feedbackLeft},
			{delayMillisecondsRight, feedbackRight},
			mix,
			maxFileLengthMs},
		feedbackProcessor);
	wav.save(output);
	return EXIT_SUCCESS;
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
		return EXIT_FAILURE;
	}
	std::string input = static_cast<std::string>(argv[2]);

	uint32_t delayMilliseconds = 0;
	if (!tryParse(argv[3], delayMilliseconds))
	{
		std::cerr << "Parameter 'delayMilliseconds' could not be parsed. Please ensure it is a numeric value. Provided value: " << argv[3] << std::endl;
		return EXIT_FAILURE;
	}

	double feedback = 0.0;
	if (!tryParse(argv[4], feedback))
	{
		std::cerr << "Parameter 'feedback' could not be parsed. Please ensure it is a numeric value. Provided value: " << argv[4] << std::endl;
		return EXIT_FAILURE;
	}
	if (feedback < 0 || feedback > 100)
	{
		std::cerr << "Parameter 'feedback' must be between 0 and 100. Provided value: " << argv[4] << std::endl;
		return EXIT_FAILURE;
	}

	double mix = 0.0;
	if (!tryParse(argv[5], mix))
	{
		std::cerr << "Parameter 'mix' could not be parsed. Please ensure it is a numeric value. Provided value: " << argv[5] << std::endl;
		return EXIT_FAILURE;
	}
	if (mix < 0 || mix > 100)
	{
		std::cerr << "Parameter 'mix' must be between 0 and 100. Provided value: " << argv[5] << std::endl;
		return EXIT_FAILURE;
	}

	std::string output = argc == 7 || argc == 8 ? static_cast<std::string>(argv[6]) : input;

	uint32_t maxFileLengthMs = 0;
	if (argc == 8)
	{
		if (!tryParse(argv[7], maxFileLengthMs))
		{
			std::cerr << "Parameter 'maxFileLengthMs' could not be parsed. Please ensure it is a numeric value. Provided value: " << argv[7] << std::endl;
			return EXIT_FAILURE;
		}
	}

	auto wav = WavFile::open(input);
	EQInstance lowPass(FilterType::LowPass, -24, 1000, 1, wav.sampleRate());

	auto feedbackProcessor = [&lowPass](double in) { return lowPass.processSample(FX::saturateSample(in, 0.55)); };

	FX::delay(
		wav,
		DelaySettings{{delayMilliseconds, feedback}, mix, maxFileLengthMs},
		feedbackProcessor);
	wav.save(output);
	return EXIT_SUCCESS;
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
		return EXIT_FAILURE;
	}
	std::string input = static_cast<std::string>(argv[2]);

	uint32_t delayMillisecondsLeft = 0;
	if (!tryParse(argv[3], delayMillisecondsLeft))
	{
		std::cerr << "Parameter 'delayMillisecondsLeft' could not be parsed. Please ensure it is a numeric value. Provided value: " << argv[3] << std::endl;
		return EXIT_FAILURE;
	}

	double feedbackLeft = 0.0;
	if (!tryParse(argv[4], feedbackLeft))
	{
		std::cerr << "Parameter 'feedbackLeft' could not be parsed. Please ensure it is a numeric value. Provided value: " << argv[4] << std::endl;
		return EXIT_FAILURE;
	}
	if (feedbackLeft < 0 || feedbackLeft > 100)
	{
		std::cerr << "Parameter 'feedbackLeft' must be between 0 and 100. Provided value: " << argv[4] << std::endl;
		return EXIT_FAILURE;
	}

	uint32_t delayMillisecondsRight = 0;
	if (!tryParse(argv[5], delayMillisecondsRight))
	{
		std::cerr << "Parameter 'delayMillisecondsRight' could not be parsed. Please ensure it is a numeric value. Provided value: " << argv[5] << std::endl;
		return EXIT_FAILURE;
	}

	double feedbackRight = 0.0;
	if (!tryParse(argv[6], feedbackRight))
	{
		std::cerr << "Parameter 'feedbackRight' could not be parsed. Please ensure it is a numeric value. Provided value: " << argv[6] << std::endl;
		return EXIT_FAILURE;
	}
	if (feedbackRight < 0 || feedbackRight > 100)
	{
		std::cerr << "Parameter 'feedbackRight' must be between 0 and 100. Provided value: " << argv[6] << std::endl;
		return EXIT_FAILURE;
	}

	double mix = 0.0;
	if (!tryParse(argv[7], mix))
	{
		std::cerr << "Parameter 'mix' could not be parsed. Please ensure it is a numeric value. Provided value: " << argv[7] << std::endl;
		return EXIT_FAILURE;
	}
	if (mix < 0 || mix > 100)
	{
		std::cerr << "Parameter 'mix' must be between 0 and 100. Provided value: " << argv[7] << std::endl;
		return EXIT_FAILURE;
	}

	std::string output = argc == 9 || argc == 10 ? static_cast<std::string>(argv[8]) : input;

	uint32_t maxFileLengthMs = 0;
	if (argc == 10)
	{
		if (!tryParse(argv[9], maxFileLengthMs))
		{
			std::cerr << "Parameter 'maxFileLengthMs' could not be parsed. Please ensure it is a numeric value. Provided value: " << argv[9] << std::endl;
			return EXIT_FAILURE;
		}
	}

	auto wav = WavFile::open(input);
	EQInstance lowPassL(FilterType::LowPass, -24, 1000, 1, wav.sampleRate());
	EQInstance lowPassR(FilterType::LowPass, -24, 1000, 1, wav.sampleRate());

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

	FX::delay(
		wav,
		StereoDelaySettings{
			{delayMillisecondsLeft, feedbackLeft},
			{delayMillisecondsRight, feedbackRight},
			mix,
			maxFileLengthMs},
		feedbackProcessor);
	wav.save(output);
	return EXIT_SUCCESS;
}

/* various delay overloads */

void Commands::reverbHelp()
{
	std::cout << "reverb <input> <size> <time> <mix> [output] [maxOutputLengthMs]" << std::endl;
	std::cout << "\tdescription: applies a basic algorithmic reverb effect to a wave file" << std::endl;
	std::cout << "\tparameters:" << std::endl;
	std::cout << "\t\tinput: path to wave file" << std::endl;
	std::cout << "\t\tsize: size of the reverb effect, from 0-100%" << std::endl;
	std::cout << "\t\ttime: time of the reverb effect, from 0-100%" << std::endl;
	std::cout << "\t\tmix: mix of the reverb effect, from 0-100%" << std::endl;
	std::cout << "\t\toutput: path to output file. Optional, if not specified input will be overwritten" << std::endl;
	std::cout << "\t\tmaxOutputLengthMs: the max length of the output file in milliseconds, to allow for feedback tail" << std::endl;
}

int Commands::reverbCommand(const std::string& command, int argc, char* argv[])
{
	if (argc != 6 && argc != 7 && argc != 8)
	{
		Commands::printIncorrectArgumentsWarning(command);
		return EXIT_FAILURE;
	}
	std::string input = static_cast<std::string>(argv[2]);

	double size = 0.0;
	if (!tryParse(argv[3], size))
	{
		std::cerr << "Parameter 'size' could not be parsed. Please ensure it is a numeric value. Provided value: " << argv[3] << std::endl;
		return EXIT_FAILURE;
	}
	if (size < 0 || size > 100)
	{
		std::cerr << "Parameter 'size' must be between 0 and 100. Provided value: " << argv[3] << std::endl;
		return EXIT_FAILURE;
	}

	double time = 0.0;
	if (!tryParse(argv[4], time))
	{
		std::cerr << "Parameter 'time' could not be parsed. Please ensure it is a numeric value. Provided value: " << argv[4] << std::endl;
		return EXIT_FAILURE;
	}
	if (time < 0 || time > 100)
	{
		std::cerr << "Parameter 'time' must be between 0 and 100. Provided value: " << argv[4] << std::endl;
		return EXIT_FAILURE;
	}

	double mix = 0.0;
	if (!tryParse(argv[5], mix))
	{
		std::cerr << "Parameter 'mix' could not be parsed. Please ensure it is a numeric value. Provided value: " << argv[5] << std::endl;
		return EXIT_FAILURE;
	}
	if (mix < 0 || mix > 100)
	{
		std::cerr << "Parameter 'mix' must be between 0 and 100. Provided value: " << argv[5] << std::endl;
		return EXIT_FAILURE;
	}

	std::string output = argc == 7 || argc == 8 ? static_cast<std::string>(argv[6]) : input;

	uint32_t maxFileLengthMs = 0;
	if (argc == 8)
	{
		if (!tryParse(argv[7], maxFileLengthMs))
		{
			std::cerr << "Parameter 'maxFileLengthMs' could not be parsed. Please ensure it is a numeric value. Provided value: " << argv[7] << std::endl;
			return EXIT_FAILURE;
		}
	}

	auto wav = WavFile::open(input);
	FX::reverb(wav, ReverbSettings{size, time, mix, maxFileLengthMs});
	wav.save(output);
	return EXIT_SUCCESS;
}

