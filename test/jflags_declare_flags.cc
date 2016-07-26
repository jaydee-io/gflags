#define JFLAGS_DLL_DECLARE_FLAG

#include <iostream>
#include <jflags/jflags_declare.h>

DECLARE_string(message); // in jflags_delcare_test.cc

void print_message();
void print_message()
{
  std::cout << FLAGS_message << std::endl;
}
