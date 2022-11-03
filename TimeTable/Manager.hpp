#include <boost/date_time/gregorian/gregorian.hpp>
#include <filesystem>
#include <iostream>
#include <fstream>
#include <sstream>
#include <thread>
#include <limits>
#include <random>

#include "Params.hpp"
#include "Parser.hpp"

using namespace std;
using namespace std::filesystem;

class Manager {
private:
  Params p;
  Parser parser;
  TimeTable tt;
  const string base_url = "https://mai.ru/education/studies/schedule/";

  const char* const short_weekday_names[8] = {
    "Вс", "Пн", "Вт","Ср", "Чт", "Пт", "Сб", "Сб"
  };
  const char* const long_month_names[12] = {
    "января", "февраля", "марта","апреля", "мая",
    "июня", "июля", "августа", "сентября", "октября",
    "ноября", "декабря"
  };
  date_facet* russian_facet = new date_facet();

  const string today_url();
  const string  week_url();
  const string group_url();
  const string session_url();
  const string getPtimeString(const ptime& time, const char* format);

  void readTT();
  void writeTT();
  unsigned short calcWeek();
  void setTimeTable();
  void printTimeTable();
  void writeIcsTimeTable();

public:
  Manager(int& argc, char* argv[]);

  void run();
};