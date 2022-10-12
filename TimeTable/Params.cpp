#include "Params.hpp"

Params::Params(int argc, char* argv[]) {
  string param;
  dep = stoi(argv[1]), course = stoi(argv[2]);
  if (dep > 12 || dep == 0 ||
    course > 6 || course == 0)
    printHelp();

  filename = to_string(dep) + '-' + to_string(course) + ".xml";

  if (argc > 3) {
    for (size_t i = 3; i < argc; i++) {
      param = string(argv[i]);
      if (param == "--group" || param == "-g")
        group = stoi(argv[++i]);
      else if (param == "--week" || param == "-w")
        week = stoi(argv[++i]);
      else if (param == "--list" || param == "-l")
        list = true;
      else if (param == "--clear" || param == "-c") {
        clear = true;
        return;
      }
      else
        throw ("Unknown parameter " + param).c_str();
    }

    parse_group(*this);

    if ((!group || group > group_names.size()) && !list && !clear)
      throw "Такого номера группы не существует";
    if ((!week || week > 18) && !list && !clear)
      throw "Такой недели не существует";
  } else
    parse_group(*this);
}

void Params::printHelp() {
  cout << "TimeTable.exe {Институт} {Курс} --group [Номер группы из списка] \
--week [номер недели 1-18]\n";
  cout << "TimeTable.exe {Институт} {Курс} --list\n";
  cout << "TimeTable.exe --clear\n\n";
  cout << "  --list  - Показать только список групп\n";
  cout << "  --clear - Очистить весь кэш\n";
  exit(1);
}