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
        throw std::invalid_argument("Номер группы пропущен");
    }
    else if (param == "--week" || param == "-w") {
      if (i + 1 < argc) {
        if (!strcmp(argv[i + 1], "current")) {
          w_cur = true;
          i++;
          continue;
        }
        if (!strcmp(argv[i + 1], "next")) {
          w_next = true;
          i++;
          continue;
        }

        week = stoi(argv[++i]);
        if ((!week || (week > 18)) && !list)
          throw invalid_argument("Такой недели не существует");
      }
      else
        throw invalid_argument("Номер недели пропущен");
    }
    else if (param == "--proxy") {
      if (i + 1 < argc)
        proxy = argv[++i];
      else
        throw invalid_argument("Адрес прокси пропущен");
    }
    else if (param == "--output" || param == "-o") {
      if (i + 1 < argc)
        output_path = argv[++i];
      else
        throw invalid_argument("Путь вывода пропущен");
    }
    else if (param == "--sleep") {
      if (i + 1 < argc) {
        sleep = stoi(argv[++i]);
      }
      else
        throw invalid_argument("Время простоя пропущено");
    }
    else if (param == "--list" || param == "-l") {
      list = true;
      return;
    }
    else if (param == "--ics")
      ics = true;
    else if (param == "--sem")
      semester = true;
    else if (param == "--tilsem")
      until_semester = true;
    else if (param == "--session")
      session = true;
    else
      throw invalid_argument(("Неизвестный параметр " + param).c_str());
  }

  if (argc > 3 && (!group || group > group_names.size()) && !list)
    throw std::invalid_argument("Номер группы не существует");
}

void Params::checkArgc(int& argc) {
  if (argc < 3)
    printHelp();
}

void Params::printHelp() {
  cout << "TimeTable.exe {Институт} {Курс} --group <Число> --week <Число>\n" <<
    "TimeTable.exe {Институт} {Курс} --list\n" <<
    "TimeTable.exe --clear\n\n" <<
    "  Институт     - Номер института от 1 до 12\n" <<
    "  Курс         - Номер курса от 1 до 6\n" <<
    "  --group, -g  - Номер группы из списка\n" <<
    "  --week,  -w  - Номер недели от 1 до 18 или current для текущей недили, next — для следующей\n" <<
    "  --list,  -l  - Показать только список групп\n" <<
    "  --ics        - Вывод в ics файл\n" <<
    "  --proxy      - Использовать прокси\n" <<
    "                 <протокол://адрес:порт>\n" <<
    "  --sem        - Загрузить весь семестр\n" <<
    "  --tilsem     - Загрузить семестр от текущей недели до конца\n" <<
    "  --sleep      - Время (в секундах) простоя после загрузки недели для семестра\n" <<
    "  --session    - Расписание сессии\n" <<
    "  --output, -o - Путь для вывода\n";
  exit(1);
}