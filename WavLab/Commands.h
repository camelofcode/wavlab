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

	void registerCommand(std::string command, std::function<void()> help, std::function<int(std::string command, int argc, char* argv[])> action);
	void showAllHelp();
	void help(std::string command);
	int command(std::string command, int argc, char* argv[]);

private:
	std::map <std::string, std::function<void()>> helpMap;
	std::map <std::string, std::function<int(std::string command, int argc, char* argv[])>> commandMap;
};

namespace Commands
{
	//Command help and command line parsing functions are defined here, but implemented in their respective files

	//Tools

	void statHelp();
	int statCommand(std::string command, int argc, char* argv[]);

	void normaliseHelp();
	int normaliseCommand(std::string command, int argc, char* argv[]);

	void reverseHelp();
	int reverseCommand(std::string command, int argc, char* argv[]);

	void flipPhaseHelp();
	int flipPhaseCommand(std::string command, int argc, char* argv[]);

	void swapChannelsHelp();
	int swapChannelsCommand(std::string command, int argc, char* argv[]);

	void splitStereoHelp();
	int splitStereoCommand(std::string command, int argc, char* argv[]);

	void getPeakDbHelp();
	int getPeakDbCommand(std::string command, int argc, char* argv[]);

	void getRMSDbHelp();
	int getRMSDbCommand(std::string command, int argc, char* argv[]);

	void gainHelp();
	int gainCommand(std::string command, int argc, char* argv[]);

	void gainDbHelp();
	int gainDbCommand(std::string command, int argc, char* argv[]);

	/* EQ */

	void eqPeakHelp();
	void eqLowShelfHelp();
	void eqHighShelfHelp();
	int eqWithGainCommand(std::string command, int argc, char* argv[]);

	void eqAllPassHelp();
	void eqBandPassHelp();
	void eqLowPassHelp();
	void eqHighPassHelp();
	int eqPassCommand(std::string command, int argc, char* argv[]);

	/* FX */
	void saturateHelp();
	int saturateCommand(std::string command, int argc, char* argv[]);

	/* Generators */

	void sineWaveHelp();
	int sineWaveCommand(std::string command, int argc, char* argv[]);
}