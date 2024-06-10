#include <boost/date_time/gregorian/gregorian.hpp>
#include <filesystem>
#include <iostream>
#include <fstream>
#include <sstream>
#include <thread>
#include <limits>
#include <random>

#include <Params.hpp>
#include <Parser.hpp>

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

  const string today_url() const;
  const string  week_url() const;
  const string group_url() const;
  const string session_url() const;
  const string getPtimeString(const ptime& time, const char* format) const;

 // void readTT();
 // void writeTT();
  const uint16_t calcWeek() const;
  void setTimeTable();
  void printTimeTable() const;
  void writeIcsTimeTable() const;

public:
  Manager(const unsigned argc, char* argv[]);

  void run();
};