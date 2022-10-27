#include <iostream>
#include "Commands.h"

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        std::cout << "No command specified. To see available commands run 'wavlab help'" << std::endl;
        return 1;
    }

    CommandsContainer commands;

    std::string command = static_cast<std::string>(argv[1]);

    if (command == "help")
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
        return 0;
    }
    else 
    {
        return commands.command(command, argc, argv);
    }

    std::cout << "Unrecognised command specified. To see available commands run 'wavlab help'" << std::endl;
    return 1;
}
