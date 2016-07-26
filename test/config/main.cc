#include <iostream>
#include <jflags/jflags.h>

DEFINE_string(message, "Hello World!", "The message to print");

static bool ValidateMessage(const char* flagname, const std::string &message)
{
  return !message.empty();
}
DEFINE_validator(message, ValidateMessage);

int main(int argc, char **argv)
{
  jflags::SetUsageMessage("Test CMake configuration of jflags library (jflags-config.cmake)");
  jflags::SetVersionString("0.1");
  jflags::ParseCommandLineFlags(&argc, &argv, true);
  std::cout << FLAGS_message << std::endl;
  jflags::ShutDownCommandLineFlags();
  return 0;
}
