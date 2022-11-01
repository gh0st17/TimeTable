#pragma once
#include <iostream>
#include <string>
#include <vector>

struct Params {
  unsigned short dep{ 0 }, course{ 0 },
    group{ 0 }, week{ 0 }, sleep{ 15 };
  bool list{ false }, clear{ false },
    ics{ false }, semester{ false },
    until_semester{ false },
    session{ false };
  std::vector<std::string> group_names;
  std::string filename, proxy;

  Params() {}
  Params(char* arg1, char* arg2);
  Params(Params& p, int& argc, char* argv[]);

  void checkArgc(int& argc);
  void printHelp();
};