#include "Commands.h"

CommandsContainer::CommandsContainer()
{
	//Tools
	registerCommand("stat", &Commands::statHelp, &Commands::statCommand);
	registerCommand("normalise", &Commands::normaliseHelp, &Commands::normaliseCommand);
	registerCommand("reverse", &Commands::reverseHelp, &Commands::reverseCommand);
	registerCommand("flip-phase", &Commands::flipPhaseHelp, &Commands::flipPhaseCommand);
	registerCommand("swap-channels", &Commands::swapChannelsHelp, &Commands::swapChannelsCommand);
	registerCommand("split-stereo", &Commands::splitStereoHelp, &Commands::splitStereoCommand);
	registerCommand("left-right-to-stereo", &Commands::combineLeftRightHelp, &Commands::combineLeftRightCommand);
	registerCommand("split-mid-side", &Commands::splitStereoMidSideHelp, &Commands::splitStereoMidSideCommand);
	registerCommand("mid-side-to-stereo", &Commands::combineMidSideHelp, &Commands::combineMidSideCommand);
	registerCommand("get-peak-db", &Commands::getPeakDbHelp, &Commands::getPeakDbCommand);
	registerCommand("get-rms-db", &Commands::getRMSDbHelp, &Commands::getRMSDbCommand);
	registerCommand("gain", &Commands::gainHelp, &Commands::gainCommand);
	registerCommand("gain-db", &Commands::gainDbHelp, &Commands::gainDbCommand);
	registerCommand("mix", &Commands::mixHelp, &Commands::mixCommand);
	registerCommand("split", &Commands::splitHelp, &Commands::splitCommand);
	registerCommand("join", &Commands::joinHelp, &Commands::joinCommand);

	//EQ
	registerCommand("eq-peak", &Commands::eqPeakHelp, &Commands::eqWithGainCommand);
	registerCommand("eq-lowshelf", &Commands::eqLowShelfHelp, &Commands::eqWithGainCommand);
	registerCommand("eq-highshelf", &Commands::eqHighShelfHelp, &Commands::eqWithGainCommand);
	registerCommand("eq-allpass", &Commands::eqAllPassHelp, &Commands::eqPassCommand);
	registerCommand("eq-bandpass", &Commands::eqBandPassHelp, &Commands::eqPassCommand);
	registerCommand("eq-lowpass", &Commands::eqLowPassHelp, &Commands::eqPassCommand);
	registerCommand("eq-highpass", &Commands::eqHighPassHelp, &Commands::eqPassCommand);

	//FX
	registerCommand("saturate", &Commands::saturateHelp, &Commands::saturateCommand);

	registerCommand("digital-delay-mono", &Commands::digitalDelayMonoHelp, &Commands::digitalDelayMonoCommand);
	registerCommand("digital-delay-stereo", &Commands::digitalDelayStereoHelp, &Commands::digitalDelayStereoCommand);
	registerCommand("analog-delay-mono", &Commands::analogDelayMonoHelp, &Commands::analogDelayMonoCommand);
	registerCommand("analog-delay-stereo", &Commands::analogDelayStereoHelp, &Commands::analogDelayStereoCommand);
	registerCommand("tape-delay-mono", &Commands::tapeDelayMonoHelp, &Commands::tapeDelayMonoCommand);
	registerCommand("tape-delay-stereo", &Commands::tapeDelayStereoHelp, &Commands::tapeDelayStereoCommand);

	//Generators
	registerCommand("sine-wave", &Commands::sineWaveHelp, &Commands::sineWaveCommand);
	registerCommand("square-wave", &Commands::squareWaveHelp, &Commands::squareWaveCommand);
	registerCommand("square-wave-additive", &Commands::squareWaveAdditiveHelp, &Commands::squareWaveAdditiveCommand);
	registerCommand("triangle-wave", &Commands::triangleWaveHelp, &Commands::triangleWaveCommand);
	registerCommand("triangle-wave-additive", &Commands::triangleWaveAdditiveHelp, &Commands::triangleWaveAdditiveCommand);
	registerCommand("sawtooth-wave", &Commands::sawtoothWaveHelp, &Commands::sawtoothWaveCommand);
	registerCommand("sawtooth-wave-additive", &Commands::sawtoothWaveAdditiveHelp, &Commands::sawtoothWaveAdditiveCommand);
	registerCommand("white-noise", &Commands::whiteNoiseHelp, &Commands::whiteNoiseCommand);
	registerCommand("pink-noise", &Commands::pinkNoiseHelp, &Commands::pinkNoiseCommand);
};

void CommandsContainer::registerCommand(const std::string& command, const std::function<void()>& help, const std::function<int(const std::string& command, int argc, char* argv[])>& action)
{
	helpMap.insert({ command, help });
	commandMap.insert({ command, action });
}

void CommandsContainer::showAllHelp()
{
	for (auto const& help : helpMap)
	{
		help.second();
		std::cout << std::endl;
	}
}

void CommandsContainer::help(const std::string& command)
{
	auto item = helpMap.find(command);
	if (item == helpMap.end())
	{
		std::cout << "Unrecognised command specified. To see available commands run 'wavlab help'" << std::endl;
	}
	else
	{
		item->second();		
		std::cout << std::endl;
	}
}

int CommandsContainer::command(const std::string& command, int argc, char* argv[])
{
	auto item = commandMap.find(command);
	if (item == commandMap.end())
	{
		std::cout << "Unrecognised command specified. To see available commands run 'wavlab help'" << std::endl;
		return 1;
	}
	else
	{
		return item->second(command, argc, argv);
	}
}

void Commands::printIncorrectArgumentsWarning(const std::string& command)
{
    std::cout << "Incorrect number of arguments for command '" << command << "'. To see available commands run 'wavlab help'" << std::endl;
}