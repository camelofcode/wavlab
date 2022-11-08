#pragma once
#include <functional>
#include <map>
#include <string>
#include <iostream>
#include "StringHelpers.h"
#include "TryParse.h"

class CommandsContainer
{
public:
	CommandsContainer();

	void registerCommand(const std::string& command, const std::function<void()>& help, const std::function<int(const std::string& command, int argc, char* argv[])>& action);
	void showAllHelp();
	void help(const std::string& command);
	int command(const std::string& command, int argc, char* argv[]);

private:
	std::map <std::string, std::function<void()>> helpMap;
	std::map <std::string, std::function<int(std::string command, int argc, char* argv[])>> commandMap;
};

namespace Commands
{
    void printIncorrectArgumentsWarning(const std::string& command);

	//Command help and command line parsing functions are defined here, but implemented in their respective files

	//Tools

	void statHelp();
	int statCommand(const std::string& command, int argc, char* argv[]);

	void normaliseHelp();
	int normaliseCommand(const std::string& command, int argc, char* argv[]);

	void reverseHelp();
	int reverseCommand(const std::string& command, int argc, char* argv[]);

	void flipPhaseHelp();
	int flipPhaseCommand(const std::string& command, int argc, char* argv[]);

	void swapChannelsHelp();
	int swapChannelsCommand(const std::string& command, int argc, char* argv[]);

	void splitStereoHelp();
	int splitStereoCommand(const std::string& command, int argc, char* argv[]);

	void combineLeftRightHelp();
	int combineLeftRightCommand(const std::string& command, int argc, char* argv[]);

	void splitStereoMidSideHelp();
	int splitStereoMidSideCommand(const std::string& command, int argc, char* argv[]);

	void combineMidSideHelp();
	int combineMidSideCommand(const std::string& command, int argc, char* argv[]);

	void getPeakDbHelp();
	int getPeakDbCommand(const std::string& command, int argc, char* argv[]);

	void getRMSDbHelp();
	int getRMSDbCommand(const std::string& command, int argc, char* argv[]);

	void gainHelp();
	int gainCommand(const std::string& command, int argc, char* argv[]);

	void gainDbHelp();
	int gainDbCommand(const std::string& command, int argc, char* argv[]);

	void mixHelp();
	int mixCommand(const std::string& command, int argc, char* argv[]);

	void splitHelp();
	int splitCommand(const std::string& command, int argc, char* argv[]);

	void joinHelp();
	int joinCommand(const std::string& command, int argc, char* argv[]);

	/* EQ */

	void eqPeakHelp();
	void eqLowShelfHelp();
	void eqHighShelfHelp();
	int eqWithGainCommand(const std::string& command, int argc, char* argv[]);

	void eqAllPassHelp();
	void eqBandPassHelp();
	void eqLowPassHelp();
	void eqHighPassHelp();
	int eqPassCommand(const std::string& command, int argc, char* argv[]);

	/* FX */
	void saturateHelp();
	int saturateCommand(const std::string& command, int argc, char* argv[]);

	void digitalDelayMonoHelp();
	int digitalDelayMonoCommand(const std::string& command, int argc, char* argv[]);

	/* Generators */

	void sineWaveHelp();
	int sineWaveCommand(const std::string& command, int argc, char* argv[]);

	void squareWaveHelp();
	int squareWaveCommand(const std::string& command, int argc, char* argv[]);

	void squareWaveAdditiveHelp();
	int squareWaveAdditiveCommand(const std::string& command, int argc, char* argv[]);

	void triangleWaveHelp();
	int triangleWaveCommand(const std::string& command, int argc, char* argv[]);

	void triangleWaveAdditiveHelp();
	int triangleWaveAdditiveCommand(const std::string& command, int argc, char* argv[]);

	void sawtoothWaveHelp();
	int sawtoothWaveCommand(const std::string& command, int argc, char* argv[]);

	void sawtoothWaveAdditiveHelp();
	int sawtoothWaveAdditiveCommand(const std::string& command, int argc, char* argv[]);

	void whiteNoiseHelp();
	int whiteNoiseCommand(const std::string& command, int argc, char* argv[]);

	void pinkNoiseHelp();
	int pinkNoiseCommand(const std::string& command, int argc, char* argv[]);
}