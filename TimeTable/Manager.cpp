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

const string Manager::getPtimeString(const ptime& time, const char* format) {
  locale loc(cout.getloc(),
    new time_facet(format));

  stringstream ss;
  ss.imbue(loc);
  ss << time;
  return ss.str();
}

void Manager::readTT() {

}

void Manager::writeTT() {

}

void Manager::setTimeTable() {
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
      for (const auto& group : p.group_names)
        cout << group << endl;
    else if (p.group && p.week)
      parser.parse(&tt, p, week_url());
    else if (p.group && !p.week)
      parser.parse(&tt, p, today_url());
    else {
      cout << "Введите номер группы: ";
      while (!(cin >> p.group) || !p.group || p.group > p.group_names.size()) {
        cout << "Введите номер группы: ";
        cin.clear();
        cin.ignore(std::numeric_limits<streamsize>::max(), '\n');
      }
      parser.parse(&tt, p, today_url());
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
    cout << day.pdate << endl;
    for (const auto& item : day.items) {
      cout << '[' << item.item_type << "] " << item.name << endl <<
        getPtimeString(item.time, "%H:%M") << " - " <<
        getPtimeString(item.time + minutes(90), "%H:%M") << " / ";

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
  uniform_int_distribution<unsigned long long> distr;
  random_device rd;
  knuth_b knuth(rd());
  knuth.seed((unsigned long long)time(0));

  stringstream filename;
  filename << tt.group << 
    (p.semester ? "_Semester" : 
      (p.until_semester ? "_Until_Semester" : "_Week_"));
  if (!p.semester && !p.until_semester)
    if (tt.week)
      filename << tt.week;
    else
      filename << "Today";
  filename << ".ics";

  cout << "Вывод в файл " << filename.str() << endl;

  ofstream ofs(filename.str());
  ofs << "BEGIN:VCALENDAR\nVERSION:2.0\n" <<
    "PRODID:ghost17 | Alexey Sorokin\n" <<
    "CALSCALE:GREGORIAN\n\n";

  for (const auto& day : tt.days) {
    for (const auto& item : day.items) {
      distr.param(uniform_int_distribution<unsigned long long>::param_type(0xFFFFFFFF, 0x8000000000000000));
      ofs << "BEGIN:VEVENT\nUID:" << distr(knuth) << "\nDTSTART:" <<
        getPtimeString(item.time, "%Y%m%dT%H%M%S") << "\nDTSTAMP:" <<
        getPtimeString(item.time, "%Y%m%dT%H%M%SZ") << "\nDTEND:" <<
        getPtimeString(item.time + minutes(90), "%Y%m%dT%H%M%S") << endl <<
        "SUMMARY:" << item.name << endl << "LOCATION:" << item.item_type <<
        " / ";

      for (const auto& place : item.places) {
        ofs << place;
        if (place != item.places.back())
          ofs << " / ";
      }

      if (item.educators.size()) {
        ofs << " / ";
        for (const auto& educator : item.educators) {
          ofs << educator;
          if (educator != item.educators.back())
            ofs << " / ";
        }
      }

      ofs << "\nEND:VEVENT\n\n";
    }
  }

  ofs.close();
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

void Manager::run() {
  if (p.week != -1 && !p.list && !p.clear && (p.semester || p.until_semester)) {
    unsigned short week = 18;
    if (p.until_semester) {
      week = parser.parse_week(p, today_url());
      if (day_clock::local_day().day_of_week().as_number() == 0)
        week++;
    }
    else if (p.semester)
      week = 1;

    for (; week <= 18; week++) {
      cout << "Получаю расписание " << week << " недели\n";
      p.week = week;
      setTimeTable();
      if (week != 18) {
        cout << "Ожидаю " << p.sleep << " секунд\n";
        this_thread::sleep_for(chrono::seconds(p.sleep));
      }
    }
  }
  else if (p.week == (unsigned short)(-1)) {
    cout << parser.parse_week(p, today_url()) << endl;
    return;
  }
  else
    setTimeTable();

  if (p.list || p.clear)
    return;

  if (p.ics)
    writeIcsTimeTable();
  else
    printTimeTable();
}