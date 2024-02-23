#include <Manager.hpp>

#undef max

const string Manager::today_url() const {
  return base_url + "index.php?group=" + p.group_names[p.group - 1];
}

const string Manager::week_url() const {
  return today_url() + "&week=" + to_string(p.week);
}

const string Manager::group_url() const {
  return base_url + "groups.php?department=Институт+№"
    + to_string(p.dep) + "&course=" + to_string(p.course);
}

const string Manager::session_url() const {
  return base_url + "session/index.php?group=" + p.group_names[p.group - 1];
}

const string Manager::getPtimeString(const ptime& time, const char* format) const {
  locale loc(cout.getloc(),
    new time_facet(format));

  stringstream ss;
  ss.imbue(loc);
  ss << time;
  return ss.str();
}

/***
 * @todo Caching to local database
void Manager::readTT() {

}

void Manager::writeTT() {

}
*/

unsigned short Manager::calcWeek() const {
  short week;
  auto const today = day_clock::local_day();

  if (today >= date(today.year(), 9, 1))
    week = today.week_number() - 34;
  else
    week = today.week_number() - 6;

  if (week < 1)
    week = 1;
  else if (week > 18)
    week = 18;

  return week;
}

void Manager::setTimeTable() {
  if (p.list)
    for (const auto& group : p.group_names)
      cout << group << endl;
  else if (p.group && p.session)
    parser.parse(tt, p, session_url());
  else if (p.group && (p.w_cur || p.w_next)) {
    p.week = calcWeek();
    if (p.week != 18 && p.w_next)
      p.week++;
    parser.parse(tt, p, week_url());
  }
  else if (p.group && p.week)
    parser.parse(tt, p, week_url());
  else if (p.group && !p.week)
    parser.parse(tt, p, today_url());
  else {
    cout << "Введите номер группы: ";
    while (!(cin >> p.group) || !p.group || p.group > p.group_names.size()) {
      cout << "Введите номер группы: ";
      cin.clear();
      cin.ignore(std::numeric_limits<streamsize>::max(), '\n');
    }
    parser.parse(tt, p, today_url());
  }
}

void Manager::printTimeTable() const {
  cout << p.group_names[p.group - 1] << "\n\n";

  if (p.week)
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

void Manager::writeIcsTimeTable() const {
  uniform_int_distribution<unsigned long long> distr;
  random_device rd;
  mt19937 mt(rd());
  mt.seed(time(0L));

  stringstream filename;

  if (!p.output_path.empty() &&
      !std::filesystem::exists(p.output_path))
    std::filesystem::create_directory(p.output_path);

  filename << p.output_path << '/' << tt.group << 
    (p.semester ? "_Semester" : 
      (p.until_semester ? "_Until_Semester" : 
        (p.session ? "_Session" : "_Week_")));
  
  if (!p.semester && !p.until_semester && !p.session) {
    if (tt.week)
      filename << tt.week;
    else
      filename << "Today";
  }
  
  filename << ".ics";

  cout << "Вывод в файл " << filename.str() << endl;

  ofstream ofs(filename.str());
  ofs << "BEGIN:VCALENDAR\nVERSION:2.0\n" <<
    "PRODID:ghost17 | Alexey Sorokin\n" <<
    "CALSCALE:GREGORIAN\n\n";

  for (const auto& day : tt.days) {
    for (const auto& item : day.items) {
      distr.param(uniform_int_distribution<unsigned long long>::param_type(0xFFFFFFFF, 0x8000000000000000));
      ofs << "BEGIN:VEVENT\nUID:" << distr(mt) << "\nDTSTART:" <<
        getPtimeString(item.time, "%Y%m%dT%H%M%S") << "\nDTSTAMP:" <<
        getPtimeString(item.time, "%Y%m%dT%H%M%SZ") << "\nDTEND:" <<
        getPtimeString(item.time + minutes(90), "%Y%m%dT%H%M%S") <<
        "\nSUMMARY:" << item.name << "\nLOCATION:";

      for (const auto& place : item.places) {
        ofs << place;
        if (place != item.places.back())
          ofs << " / ";
      }

      ofs << " / " << item.item_type;

      if (item.educators.size())
        for (const auto& educator : item.educators)
          ofs << " / " << educator;

      ofs << "\nEND:VEVENT\n\n";
    }
  }

  ofs << "END:VCALENDAR";
  ofs.close();
}

Manager::Manager(const unsigned argc, char* argv[]) {
  p.checkArgc(argc);
  p.setDepCourse(argv[1], argv[2]);
  p.fetchParams(argc, argv);

  if (argc > 3)
    parser.parse_group(p, group_url(), false);
  else
    parser.parse_group(p, group_url(), true);

  if (p.validateGroup(argc))
    throw invalid_argument("Номер группы не существует");

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
  if (!p.week && !p.list && (p.semester || p.until_semester)) {
    unsigned short week = 18;
    if (p.until_semester) {
      week = calcWeek();
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
        cout << "Ожидаю " << p.sleep << " сек.\n";
        this_thread::sleep_for(chrono::seconds(p.sleep));
      }
    }
  }
  else
    setTimeTable();

  if (p.list)
    return;

  if (tt.days.size()) {
    if (p.ics)
      writeIcsTimeTable();
    else
      printTimeTable();
  }
}
