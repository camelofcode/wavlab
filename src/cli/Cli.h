#pragma once

#include <CLI/CLI.hpp>

#include <functional>
#include <string>

using CliAction = std::function<void()>;

void configureGeneratorCommands(CLI::App &app, CliAction &selectedAction);
void configureToolCommands(CLI::App &app, CliAction &selectedAction);
void configureEqCommands(CLI::App &app, CliAction &selectedAction);
void configureEffectCommands(CLI::App &app, CliAction &selectedAction);

class CommandLine
{
  public:
    CommandLine();

    void parse(int argc, char *argv[]);
    void execute();
    [[nodiscard]] int exit(const CLI::ParseError &error) const;
    [[nodiscard]] std::string referenceMarkdown() const;

  private:
    CLI::App app_;
    CliAction selectedAction_;
};
