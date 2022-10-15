#include "Manager.hpp"

#undef max

const string Manager::today_url() {
  return base_url + "index.php?group=" + p.group_names[p.group - 1];
}

const string Manager::week_url() {
  return today_url() + "&week=" + to_string(p.week);
}

const string Manager::group_url() {
  return base_url + "groups.php?department=Институт+№"
    + to_string(p.dep) + "&course=" + to_string(p.course);
}

Manager::Manager(int& argc, char* argv[]) {
  try {
    p.checkArgc(argc);
    p = Params(argv[1], argv[2]);
    if (argc > 3)
      parser.parse_group(p, group_url(), false);
    else
      parser.parse_group(p, group_url(), true);

    p = Params(p, argc, argv);
  }
  catch (bad_alloc const&) {
    cerr << "Ошибка выделения памяти\n";
  }
  catch (const exception& e) {
    cerr << e.what() << endl;
  }
  catch (const char* e) {
    cerr << e << endl;
  }
}

void Manager::run() {
  try {
    if (p.clear) {
      unsigned cnt{ 0 };
      for (const auto& file : directory_iterator("./")) {
        if (file.path().extension() == ".xml") {
          cout << file.path().filename().u8string() << endl;
          remove(file.path());
          cnt++;
        }
      }
      cout << "Удалено файлов: " << cnt << endl;
    }
    else if (p.list)
      return;
    else if (p.group && p.week)
      tt = parser.parse(p, week_url());
    else if (p.group && !p.week)
      tt = parser.parse(p, today_url());
    else {
      cout << "Введите номер группы: ";
      while (!(cin >> p.group) || !p.group || p.group > p.group_names.size()) {
        cout << "Введите номер группы: ";
        cin.clear();
        cin.ignore(std::numeric_limits<streamsize>::max(), '\n');
      }
      tt = parser.parse(p, today_url());
    }
  }
  catch (bad_alloc const&) {
    cerr << "Ошибка выделения памяти\n";
  }
  catch (const exception& e) {
    cerr << e.what() << endl;
  }
  catch (const char* e) {
    cerr << e << endl;
  }
}

void Manager::printTimeTable() {

}

void Manager::writeIcsTimeTable() {

}