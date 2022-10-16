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
    else if (param == "--proxy") {
      if (i + 1 < argc) {
        proxy = argv[++i];
      }
      else
        throw "Адрес прокси пропущен";
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

  if (argc > 3 && (!group || group > group_names.size()) && !list && !clear)
    throw "Номер группы не существует";
}


void Params::checkArgc(int& argc) {
  if (argc < 3)
    printHelp();
}

void Params::printHelp() {
  cout << "TimeTable.exe {Институт} {Курс} --group <Число> --week <Число>\n" <<
    "TimeTable.exe {Институт} {Курс} --list\n" <<
    "TimeTable.exe --clear\n\n" <<
    "  Институт - Номер института от 1 до 12\n" <<
    "  Курс     - Номер курса от 1 до 6\n" <<
    "  --group, - Номер группы из списка\n  -g\n" <<
    "  --week,  - Номер недели от 1 до 18\n  -w\n" <<
    "  --list,  - Показать только список групп\n  -l\n" <<
    "  --clear, - Очистить весь кэш\n  -c\n";
  exit(1);
}