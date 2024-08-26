#include <Params.hpp>

using namespace std;

void Params::fetchParams(const unsigned argc, char* argv[]) {
  string param;

  for (std::size_t i = 3; i < argc; i++) {
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
    else if (param == "--workdir" || param == "-d") {
      if (i + 1 < argc)
        work_path = argv[++i];
      else
        throw invalid_argument("Путь рабочей директории пропущен");
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
}

const bool Params::validateGroup(const unsigned argc) const {
  return argc > 3 && (!group || group > group_names.size()) && !list;
}

void Params::setDepCourse(const char* arg1, const char* arg2) {
  dep = stoi(arg1), course = stoi(arg2);
  if (dep > 0 && dep < 15 && dep != 13 && course > 0 && course < 7)
    filename = to_string(dep) + '-' + to_string(course) + ".txt";
  else
    printHelp();
}

void Params::checkArgc(const unsigned argc) const {
  if (argc < 3)
    printHelp();
}

void Params::printHelp() const {
  cout << "TimeTable.exe {Институт} {Курс} --group <Число> --week <Число>\n" <<
    "TimeTable.exe {Институт} {Курс} --list\n" <<
    "TimeTable.exe --clear\n\n" <<
    "  Институт      - Номер института от 1 до 12\n" <<
    "  Курс          - Номер курса от 1 до 6\n" <<
    "  --group,   -g - Номер группы из списка\n" <<
    "  --week,    -w - Номер недели от 1 до 18 или current для текущей недили, next — для следующей\n" <<
    "  --list,    -l - Показать только список групп\n" <<
    "  --ics         - Вывод в ics файл\n" <<
    "  --proxy       - Использовать прокси\n" <<
    "                  <протокол://адрес:порт>\n" <<
    "  --sem         - Загрузить весь семестр\n" <<
    "  --tilsem      - Загрузить семестр от текущей недели до конца\n" <<
    "  --sleep       - Время (в секундах) простоя после загрузки недели для семестра\n" <<
    "  --session     - Расписание сессии\n" <<
    "  --workdir, -d - Путь рабочей директории (кэш)\n" <<
    "  --output,  -o - Путь для вывода\n";
  exit(1);
}
