#pragma once
#include <iostream>
#include <string>
#include <vector>

using namespace std;

struct Params {
  unsigned short dep{ 0 }, course{ 0 },
    group{ 0 }, week{ 0 };
  bool list{ false }, clear{ false };
  vector<string> group_names;
  string filename;

  Params() {}
  Params(int argc, char* argv[]);
  static void printHelp();
  friend void parse_group(Params& p, const bool isPrint);
};

