#include <filesystem>
#include <iostream>
#include <limits>

#include "Params.hpp"
#include "Parser.hpp"

using namespace std;
using namespace std::filesystem;

class Manager {
private:
  Params p;
  Parser parser;
  const string base_url = "http://mai.ru/education/studies/schedule/";

  const string today_url();
  const string  week_url();
  const string group_url();

public:
  Manager(int& argc, char* argv[]);
};