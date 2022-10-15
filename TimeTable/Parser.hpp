#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/algorithm/string.hpp>
#include <pugixml.hpp>
#include <filesystem>
#include <iomanip>
#include <map>

#include "Params.hpp"

using namespace boost::gregorian;
using namespace boost::posix_time;

struct Item {
  std::vector<std::string> educator;
  std::vector<std::string> place;
  std::string item_type;
  std::string name;
  ptime time;
};

struct Day {
  date date = day_clock::local_day();
  std::vector<Item> items;
};

struct TimeTable {
  unsigned short week{ 0 };
  std::vector<Day> days;
  std::string group;
};

class Parser {
private:
  struct group_predicate {
    bool operator()(pugi::xml_node node) const;
  };

  struct week_predicate {
    bool operator()(pugi::xml_node node) const;
  };

  const std::map<std::string, std::string> month = { {"января", "01"}, {"февраля", "02"},
    {"марта", "03"}, {"апреля", "04"}, {"мая", "05"}, {"июня", "06"}, {"июля", "07"},
    {"августа", "08"}, {"сентября", "09"}, {"октября", "10"}, {"ноября", "11"}, {"декабря", "12"} };

  void prepareHTML(std::string* html);

public:
  Parser() {};

  TimeTable parse(const Params& p, const std::string& url);
  void parse_group(Params& p, const std::string& url, const bool isPrint);
};

