#include "Params.hpp"

using namespace std;

Params::Params(char* arg1, char* arg2) {
  dep = stoi(arg1), course = stoi(arg2);
  if (dep > 12 || dep == 0 ||
    course > 6 || course == 0)
    printHelp();
  else
    filename = to_string(dep) + '-' + to_string(course) + ".xml";
}

Params::Params(Params& p, int& argc, char* argv[]) {
  *this = p;
  string param;

  for (size_t i = 3; i < argc; i++) {
    param = string(argv[i]);
    if (param == "--group" || param == "-g") {
      if (i + 1 < argc)
        group = stoi(argv[++i]);
      else
        throw "Номер группы пропущен";
    }
    else if (param == "--week" || param == "-w") {
      if (i + 1 < argc) {
        week = stoi(argv[++i]);
        if ((!week || week > 18) && !list && !clear)
          throw "Такой недели не существует";
      }
      else
        throw "Номер недели пропущен";
    }
    else if (param == "--list" || param == "-l")
      list = true;
    else if (param == "--clear" || param == "-c") {
      clear = true;
      return;
    }
    else
      throw ("Неизвестный параметр " + param).c_str();
  }

  if ((!group || group > group_names.size()) && !list && !clear)
    throw "Номер группы не существует или не задан";
}


void Params::checkArgc(int& argc) {
  if (argc < 3)
    printHelp();
}

void Params::printHelp() {
  cout << "TimeTable.exe {Институт} {Курс} --group <Число> --week <Число>\n";
  cout << "TimeTable.exe {Институт} {Курс} --list\n";
  cout << "TimeTable.exe --clear\n\n";
  cout << "  Институт - Номер института от 1 до 12\n";
  cout << "  Курс     - Номер курса от 1 до 6\n";
  cout << "  --group, - Номер группы из списка\n  -g\n";
  cout << "  --week,  - Номер недели от 1 до 18\n  -w\n";
  cout << "  --list,  - Показать только список групп\n  -l\n";
  cout << "  --clear, - Очистить весь кэш\n  -c\n";
  exit(1);
}