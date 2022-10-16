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

const string Manager::getTimeString(const time_duration& td) {
  stringstream ss;

  if (td.hours() > 9)
    ss << td.hours();
  else
    ss << "0" << td.hours();

  if (td.minutes() > 9)
    ss << ':' << td.minutes();
  else
    ss << ':' << "0" << td.minutes();

  return ss.str();
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
    exit(1);
  }
  catch (const exception& e) {
    cerr << e.what() << endl;
    exit(1);
  }
  catch (const char* e) {
    cerr << e << endl;
    exit(1);
  }

  cout.imbue(locale(locale::classic(), russian_facet));
  date_facet::input_collection_type short_weekdays, long_month;
  copy(&short_weekday_names[0], &short_weekday_names[7],
    back_inserter(short_weekdays));
  copy(&long_month_names[0], &long_month_names[11],
    back_inserter(long_month));
  russian_facet->short_weekday_names(short_weekdays);
  russian_facet->long_month_names(long_month);
  russian_facet->format("%a, %d %B");
}

void Manager::getTimeTable() {
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
  cout << tt.group << "\n\n";

  if (tt.week)
    cout << "Учебная неделя №" << tt.week << "\n\n";

  for (const auto& day : tt.days) {
    cout << day.date << endl;
    for (const auto& item : day.items) {
      cout << item.name << " [" << item.item_type << "]\n" <<
        getTimeString(item.time.time_of_day()) << " - " <<
        getTimeString((item.time + minutes(90)).time_of_day()) << " / ";

      if (item.educators.size())
        for (const auto& educator : item.educators)
          cout << educator << " / ";

      for (const auto& place : item.places) {
        cout << place;
        if (place != item.places.back())
          cout << " / ";
      }
      cout << endl;
    }
    cout << endl;
  }
}

void Manager::writeIcsTimeTable() {

}