#include "Cli.h"

#include <cstdlib>
#include <exception>
#include <iostream>

int main(int argc, char *argv[])
{
    CommandLine commandLine;
    try
    {
        commandLine.parse(argc, argv);
        commandLine.execute();
        return EXIT_SUCCESS;
    }
    catch (const CLI::ParseError &error)
    {
        return commandLine.exit(error);
    }
    catch (const std::exception &error)
    {
        std::cerr << "wavlab: " << error.what() << '\n';
        return EXIT_FAILURE;
    }
}
