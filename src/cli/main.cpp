#include <exception>
#include <iostream>
#include "Commands.h"

namespace
{

int run(int argc, char* argv[])
{
    if (argc < 2)
    {
        std::cerr << "No command specified. To see available commands run 'wavlab help'" << std::endl;
        return EXIT_FAILURE;
    }

    CommandsContainer commands;

    std::string command = static_cast<std::string>(argv[1]);

    if (command == "help" || command == "--help" || command == "-h")
    {
        if (argc == 2)
        {
            //Print all
            std::cout << "wavlab commands" << std::endl << std::endl;

            std::cout << "help" << std::endl;
            std::cout << "\tdescription: shows the command list" << std::endl << std::endl;

            commands.showAllHelp();
        }
        else 
        {
            //print specified command help
            std::string argument = static_cast<std::string>(argv[2]);
            commands.help(argument);
        }
        return EXIT_SUCCESS;
    }
    else 
    {
        return commands.command(command, argc, argv);
    }
}

}

int main(int argc, char* argv[])
{
    try
    {
        return run(argc, argv);
    }
    catch (const std::exception& error)
    {
        std::cerr << "wavlab: " << error.what() << std::endl;
        return EXIT_FAILURE;
    }
}
