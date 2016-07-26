#include <jflags/jflags.h>

DEFINE_string(message, "", "The message to print");
void print_message(); // in jflags_declare_flags.cc

int main(int argc, char **argv)
{
  jflags::SetUsageMessage("Test compilation and use of jflags_declare.h");
  jflags::ParseCommandLineFlags(&argc, &argv, true);
  print_message();
  return 0;
}
