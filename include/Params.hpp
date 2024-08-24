#pragma once
#include <iostream>
#include <cstring>
#include <string>
#include <vector>

struct Params {
  unsigned short dep{ 0 }, course{ 0 },
    group{ 0 }, week{ 0 }, sleep{ 15 };
  bool list{ false }, ics{ false }, semester{ false },
    until_semester{ false }, session{ false },
    w_cur{ false }, w_next{ false };
  std::vector<std::string> group_names;
  std::string filename, proxy, output_path = ".", work_path = ".";

  int error_code = 0;

  Params() {}
  
  void fetchParams(const unsigned argc, char* argv[]);
  const bool validateGroup(const unsigned argc) const;
  void setDepCourse(const char* arg1, const char* arg2);
  void checkArgc(const unsigned argc) const;
  void printHelp() const;
};